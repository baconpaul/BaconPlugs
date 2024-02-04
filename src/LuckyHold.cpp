#include "BaconPlugs.hpp"
#include <initializer_list>

#include "BaconModule.hpp"
#include "BaconModuleWidget.h"

#include <random>

namespace bp = baconpaul::rackplugs;

struct LuckyHold : virtual bp::BaconModule
{
    enum ParamIds
    {
        POLY_COUNT,
        CHANCE,
        RNG_SCALE,
        RNG_OFFSET,
        LATCH,
        NUM_PARAMS
    };

    enum InputIds
    {
        CLOCK_IN,
        CHANCE_CV,
        NUM_INPUTS
    };

    enum OutputIds
    {
        A_CLOCK,
        A_RNG,
        B_CLOCK,
        B_RNG,
        U_CLOCK,
        U_RNG,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        POLY_LIGHT,
        GATE_0,
        NUM_LIGHTS = GATE_0 + MAX_POLY,
    };

    std::default_random_engine gen;
    std::uniform_real_distribution<float> ud;

    LuckyHold() : bp::BaconModule(), ud(0, 1)
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        auto s = configParam(POLY_COUNT, 1, MAX_POLY, 1, "Polyphony");
        s->snapEnabled = true;

        configParam(CHANCE, -1, 1, 0, "A (-1) or B (1) chance?", "%", 0, 100);
        configParam(RNG_SCALE, 0, 10, 10, "Random CV Scale", "V");
        configParam(RNG_OFFSET, -5, 5, 0, "Random CV Center", "V");
        configSwitch(LATCH, 0, 1, 1, "Latch vs Passthrough", {"Passthrough", "Latch"});

        configInput(CLOCK_IN, "Clock");
        configInput(CHANCE_CV, "Chance");
        configOutput(A_CLOCK, "A Clock");
        configOutput(A_RNG, "A Sample and Hold Random CV");
        configOutput(B_CLOCK, "B Clock");
        configOutput(B_RNG, "B Sample and Hold Random CV");
        configOutput(U_RNG, "Clocked UnChanced Sample and Hold Random CV");
        configBypass(CLOCK_IN, A_CLOCK);
        configBypass(CLOCK_IN, B_CLOCK);

        for (int i = 0; i < MAX_POLY; ++i)
        {
            aGate[i] = false;
            bGate[i] = false;
            rA[i] = 0.f;
            rB[i] = 0.f;
            rU[i] = 0.f;
        }
    }

    int nChan{0};
    rack::dsp::SchmittTrigger inTrig;
    bool aGate[MAX_POLY], bGate[MAX_POLY];
    float rA[MAX_POLY], rB[MAX_POLY], rU[MAX_POLY];
    void process(const ProcessArgs &args) override
    {
        int pc = params[POLY_COUNT].getValue();
        if (pc != nChan)
        {
            nChan = pc;
            lights[POLY_LIGHT].value = nChan;
            for (int i = 0; i < MAX_POLY; ++i)
            {
                lights[GATE_0 + i].value = 0;
            }
        }

        auto lv = (bool)std::round(params[LATCH].getValue());

        if (inTrig.process(inputs[CLOCK_IN].getVoltageSum()))
        {
            for (int c = 0; c < nChan; ++c)
            {
                auto ch = ud(gen) * 2 - 1;
                auto rv{0.f};
                rv = ud(gen);

                rU[c] = rv;
                float chThresh = params[CHANCE].getValue();
                chThresh += inputs[CHANCE_CV].getVoltage() * 0.2;
                chThresh = std::clamp(chThresh, -1.f, 1.f);

                if (ch > chThresh)
                {
                    if (!bGate[c] || !lv)
                        rB[c] = rv;
                    aGate[c] = false;
                    bGate[c] = true;
                }
                else
                {
                    if (!aGate[c] || !lv)
                        rA[c] = rv;
                    aGate[c] = true;
                    bGate[c] = false;
                }

                lights[GATE_0 + c].value = aGate[c] ? 1 : -1;
            }
        }

        outputs[A_CLOCK].setChannels(nChan);
        outputs[B_CLOCK].setChannels(nChan);
        outputs[A_RNG].setChannels(nChan);
        outputs[B_RNG].setChannels(nChan);
        outputs[U_RNG].setChannels(nChan);
        outputs[U_CLOCK].setChannels(nChan);

        auto sc = params[RNG_SCALE].getValue();
        auto of = params[RNG_OFFSET].getValue();
        for (int c = 0; c < nChan; ++c)
        {
            outputs[A_CLOCK].setVoltage(aGate[c] * (lv ? 10 : inputs[CLOCK_IN].getVoltageSum()), c);
            outputs[B_CLOCK].setVoltage(bGate[c] * (lv ? 10 : inputs[CLOCK_IN].getVoltageSum()), c);
            outputs[A_RNG].setVoltage(rA[c] * sc + of, c);
            outputs[B_RNG].setVoltage(rB[c] * sc + of, c);
            outputs[U_RNG].setVoltage(rU[c] * sc + of, c);
            outputs[U_CLOCK].setVoltage(inputs[CLOCK_IN].getVoltage(), c);
        }
    }
};

struct ABLights : rack::TransparentWidget, baconpaul::rackplugs::StyleParticipant
{
    BufferedDrawFunctionWidget *bdw{nullptr}, *bdwLight{nullptr};
    LuckyHold *module{nullptr};
    float cSize{0}, xc0, yc0, dx, dy;
    void setup()
    {
        bdw = new BufferedDrawFunctionWidget(rack::Vec(0, 0), box.size,
                                             [this](auto v) { drawBG(v); });
        addChild(bdw);

        bdwLight = new BufferedDrawFunctionWidgetOnLayer(rack::Vec(0, 0), box.size,
                                                         [this](auto v) { drawLight(v); });
        addChild(bdwLight);
        for (int i = 0; i < MAX_POLY; ++i)
            vals[i] = 0.f;

        cSize = std::min(box.size.x / 17.f, box.size.y / 4.f) * 0.45;
        dx = box.size.x / 16.f;
        xc0 = dx * 0.5;

        dy = box.size.y / 3.f;
        yc0 = dy * 0.5;
    }

    float vals[MAX_POLY];
    void step() override
    {
        if (module)
        {
            for (int i = 0; i < MAX_POLY; ++i)
            {
                if (vals[i] != module->lights[LuckyHold::GATE_0 + i].value)
                {
                    vals[i] = module->lights[LuckyHold::GATE_0 + i].value;
                    bdw->dirty = true;
                    bdwLight->dirty = true;
                }
            }
        }
        rack::TransparentWidget::step();
    }

    void drawBG(NVGcontext *vg)
    {
        auto style = baconpaul::rackplugs::BaconStyle::get();

        int nChan = MAX_POLY;
        if (module)
            nChan = module->nChan;
        for (int i = 0; i < MAX_POLY; ++i)
        {
            nvgBeginPath(vg);
            nvgStrokeColor(vg,
                           style->getColor(baconpaul::rackplugs::BaconStyle::SECTION_RULE_LINE));
            nvgMoveTo(vg, xc0 + i * dx, yc0 + cSize);
            nvgLineTo(vg, xc0 + i * dx, yc0 + 2 * dy - cSize);
            nvgStrokeWidth(vg, 0.5);
            nvgStroke(vg);
            for (int j = 0; j < 3; ++j)
            {
                if (j == 1)
                    continue;
                nvgBeginPath(vg);
                nvgEllipse(vg, xc0 + dx * i, yc0 + dy * j, cSize, cSize);
                nvgFillColor(vg, style->getColor(baconpaul::rackplugs::BaconStyle::LIGHT_BG));
                nvgStrokeColor(
                    vg, style->getColor(baconpaul::rackplugs::BaconStyle::SECTION_RULE_LINE));
                if (i < nChan)
                    nvgFill(vg);
                nvgStrokeWidth(vg, 0.5);
                nvgStroke(vg);
            }
        }
    }
    void drawLight(NVGcontext *vg)
    {
        auto style = baconpaul::rackplugs::BaconStyle::get();
        for (int i = 0; i < MAX_POLY; ++i)
        {
            if (vals[i] == 0)
                continue;

            int j = 1;
            if (vals[i] > 0)
                j = 0;
            if (vals[i] < 0)
                j = 2;
            nvgBeginPath(vg);
            nvgEllipse(vg, xc0 + dx * i, yc0 + dy * j, cSize, cSize);
            nvgFillColor(vg, SCHEME_BLUE);
            nvgStrokeColor(vg,
                           style->getColor(baconpaul::rackplugs::BaconStyle::SECTION_RULE_LINE));
            nvgFill(vg);
            nvgStrokeWidth(vg, 0.5);
            nvgStroke(vg);
        }
    }

    void onStyleChanged() override
    {
        bdw->dirty = true;
        bdwLight->dirty = true;
    }
};
struct LuckyHoldWidget : bp::BaconModuleWidget
{
    typedef LuckyHold M;
    LuckyHoldWidget(LuckyHold *model);
};

LuckyHoldWidget::LuckyHoldWidget(LuckyHold *m) : bp::BaconModuleWidget()
{
    setModule(m);
    box.size = Vec(SCREW_WIDTH * 9, RACK_HEIGHT);

    BaconBackground *bg = new BaconBackground(box.size, "LuckyHold");
    addChild(bg->wrappedInFramebuffer());

    // Poly Knob and light
    {
        int yp = 30, h = 40;
        bg->addRoundedBorder(rack::Vec(5, yp), rack::Vec(box.size.x - 10, h));
        bg->addLabel(rack::Vec(8, yp + h * 0.5), "Poly", 12, NVG_ALIGN_MIDDLE | NVG_ALIGN_LEFT);
        addParam(createParamCentered<RoundBlackKnob>(Vec(60, yp + h * 0.5), module, M::POLY_COUNT));
        auto msg = MultiDigitSevenSegmentLight<BlueLight, 2, 2>::create(rack::Vec(90, yp + h * 0.5),
                                                                        module, M::POLY_LIGHT);
        msg->box.pos.y -= msg->box.size.y * 0.5;
        addChild(msg);
    }

    // Chance Knob CV In and Switch
    {
        int yp = 75, h = 40;
        bg->addRoundedBorder(rack::Vec(5, yp), rack::Vec(box.size.x - 10, h));
        bg->addLabel(rack::Vec(8, yp + h * 0.5), "Chance", 12, NVG_ALIGN_MIDDLE | NVG_ALIGN_LEFT);
        addParam(createParamCentered<RoundBlackKnob>(Vec(60, yp + h * 0.5), module, M::CHANCE));

        addInput(
            createInputCentered<PJ301MPort>(rack::Vec(90, yp + h * 0.5), module, M::CHANCE_CV));

        addParam(createParamCentered<CKSS>(rack::Vec(115, yp + h * 0.5 + 3), module, M::LATCH));
        bg->addLabel(rack::Vec(115, yp + h * 0.5 - 9), "latch", 8,
                     NVG_ALIGN_BOTTOM | NVG_ALIGN_CENTER);
    }

    // Scale and Offset Knobs
    {
        int yp = 120, h = 40;
        bg->addRoundedBorder(rack::Vec(5, yp), rack::Vec(box.size.x - 10, h));
        bg->addLabel(rack::Vec(8, yp + h * 0.5), "Rand CV", 12, NVG_ALIGN_MIDDLE | NVG_ALIGN_LEFT);

        addParam(createParamCentered<RoundSmallBlackKnob>(Vec(65, yp + h * 0.5 - 3), module,
                                                          M::RNG_SCALE));
        bg->addLabel(rack::Vec(65, yp + h * 0.5 + 10), "scale", 8,
                     NVG_ALIGN_TOP | NVG_ALIGN_CENTER);
        addParam(createParamCentered<RoundSmallBlackKnob>(Vec(105, yp + h * 0.5 - 3), module,
                                                          M::RNG_OFFSET));
        bg->addLabel(rack::Vec(105, yp + h * 0.5 + 10), "center", 8,
                     NVG_ALIGN_TOP | NVG_ALIGN_CENTER);
    }

    auto lt = new ABLights;
    lt->box.pos = rack::Vec(10, 168);
    lt->box.size = rack::Vec(box.size.x - 20, 30);
    lt->module = m;
    lt->setup();
    addChild(lt);

    std::vector<std::string> labels{"A", "B", "Every"};
    for (int i = 0; i < 3; ++i)
    {
        auto h = 144 - i * 38;
        bg->addRoundedBorder(Vec(5, RACK_HEIGHT - (h + 30)), Vec(box.size.x - 10, 35),
                             baconpaul::rackplugs::BaconStyle::HIGHLIGHT_BG);
        bg->addLabel(Vec(20, RACK_HEIGHT - (h + 12)), labels[i].c_str(), 11,
                     NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM,
                     baconpaul::rackplugs::BaconStyle::DEFAULT_HIGHLIGHT_LABEL);
        bg->addLabel(Vec(20, RACK_HEIGHT - h), "gate", 11, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM,
                     baconpaul::rackplugs::BaconStyle::DEFAULT_HIGHLIGHT_LABEL);
        addOutput(
            createOutput<PJ301MPort>(Vec(35, RACK_HEIGHT - (h + 24)), module, M::A_CLOCK + i * 2));

        bg->addLabel(Vec(80, RACK_HEIGHT - (h + 12)), labels[i].c_str(), 11,
                     NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM,
                     baconpaul::rackplugs::BaconStyle::DEFAULT_HIGHLIGHT_LABEL);
        bg->addLabel(Vec(80, RACK_HEIGHT - h), "CV", 11, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM,
                     baconpaul::rackplugs::BaconStyle::DEFAULT_HIGHLIGHT_LABEL);
        addOutput(
            createOutput<PJ301MPort>(Vec(95, RACK_HEIGHT - (h + 24)), module, M::A_RNG + i * 2));
    }

    {
        auto h = 30;
        bg->addRoundedBorder(Vec(5, RACK_HEIGHT - (h + 30)), Vec(70, 35),
                             baconpaul::rackplugs::BaconStyle::INPUT_BG);
        bg->addLabel(Vec(20, RACK_HEIGHT - (h + 12)), "Clock", 11,
                     NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM,
                     baconpaul::rackplugs::BaconStyle::DEFAULT_LABEL);
        bg->addLabel(Vec(20, RACK_HEIGHT - h), "in", 11, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM,
                     baconpaul::rackplugs::BaconStyle::DEFAULT_LABEL);
        addInput(createInput<PJ301MPort>(Vec(43, RACK_HEIGHT - (h + 24)), module, M::CLOCK_IN));
    }
}

Model *modelLuckyHold = createModel<LuckyHold, LuckyHoldWidget>("LuckyHold");

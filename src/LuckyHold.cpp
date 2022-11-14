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
        RNG_IS_UNI,
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
        NUM_OUTPUTS
    };

    enum LightIds
    {
        GATE_0,
        NUM_LIGHTS = GATE_0 + MAX_POLY,
    };

    std::default_random_engine gen;
    std::uniform_real_distribution<float> ud;
    std::normal_distribution<float> nd;

    LuckyHold() : bp::BaconModule(), ud(0,1), nd(0, 0.333)
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        auto s = configParam(POLY_COUNT, 1, MAX_POLY, 1, "Polyphony");
        s->snapEnabled = true;

        configParam(CHANCE, -1, 1, 0, "A (-1) or B (1) chance?", "%", 0, 100);
        configParam(RNG_SCALE, 0, 10, 10, "RNG Scale?", "V");
        configSwitch(RNG_IS_UNI, 0, 1, 0, "RNG Polarity", { "Unipolar", "Bipolar" }  );
        configSwitch(LATCH, 0, 1, 1, "Latch vs Passthrough", { "Passthrough", "Latch" } );

        configInput(CLOCK_IN, "Clock");
        configInput(CHANCE_CV, "Chance");
        configOutput(A_CLOCK, "A Clock");
        configOutput(A_RNG, "A Sample and Hold RNG");
        configOutput(B_CLOCK, "B Clock");
        configOutput(B_RNG, "B Sample and Hold RNG");
        configBypass(CLOCK_IN, A_CLOCK);
        configBypass(CLOCK_IN, B_CLOCK);

        for (int i=0; i<MAX_POLY; ++i)
        {
            aGate[i] = false;
            bGate[i] = false;
            rA[i] = 0.f;
            rB[i] = 0.f;
        }
    }


    int nChan{0};
    rack::dsp::SchmittTrigger inTrig;
    bool aGate[MAX_POLY], bGate[MAX_POLY];
    float rA[MAX_POLY], rB[MAX_POLY];
    void process(const ProcessArgs &args) override
    {
        int pc = params[POLY_COUNT].getValue();
        if (pc != nChan)
        {
            nChan = pc;
        }

        auto lv = (bool)std::round(params[LATCH].getValue());

        if (inTrig.process(inputs[CLOCK_IN].getVoltageSum()))
        {
            auto uni = (bool)std::round(params[RNG_IS_UNI].getValue());
            for (int c = 0; c < nChan; ++c)
            {
                auto ch = ud(gen) * 2 - 1;
                auto rv{0.f};
                rv = ud(gen);
                if (uni)
                    rv = rv - 0.5;
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
            }
        }

        outputs[A_CLOCK].setChannels(nChan);
        outputs[B_CLOCK].setChannels(nChan);
        outputs[A_RNG].setChannels(nChan);
        outputs[B_RNG].setChannels(nChan);

        auto sc = params[RNG_SCALE].getValue();
        for (int c = 0; c < nChan; ++c)
        {
            outputs[A_CLOCK].setVoltage(aGate[c] * (lv ? 10 : inputs[CLOCK_IN].getVoltageSum()), c);
            outputs[B_CLOCK].setVoltage(bGate[c] * (lv ? 10 : inputs[CLOCK_IN].getVoltageSum()), c);
            outputs[A_RNG].setVoltage(rA[c] * sc, c);
            outputs[B_RNG].setVoltage(rB[c] * sc, c);
        }
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
    box.size = Vec(SCREW_WIDTH * 12, RACK_HEIGHT);

    BaconBackground *bg = new BaconBackground(box.size, "LuckyHold");
    addChild(bg->wrappedInFramebuffer());

    int dmdig = 6;
    {
        float kPos = 45;
        bg->addLabel(rack::Vec(10, kPos), "Poly", 12, NVG_ALIGN_MIDDLE | NVG_ALIGN_LEFT);
        addParam(createParamCentered<RoundSmallBlackKnob>(Vec(60, kPos), module, M::POLY_COUNT));
        auto srlt = DotMatrixLightTextWidget::create(rack::Vec(80, kPos),
                                                     module, dmdig,
            nullptr,
                                                     [](auto m)
                                                     {
                            return m->paramQuantities[M::POLY_COUNT]->getDisplayValueString();
                                                     });
        srlt->box.pos.y = kPos - srlt->box.size.y * 0.5;
        addChild(srlt);
    }

    {
        float kPos = 70;
        bg->addLabel(rack::Vec(10, kPos), "Chance", 12, NVG_ALIGN_MIDDLE | NVG_ALIGN_LEFT);
        addParam(createParamCentered<RoundSmallBlackKnob>(Vec(60, kPos), module, M::CHANCE));
        auto srlt = DotMatrixLightTextWidget::create(rack::Vec(80, kPos),
                                                     module, dmdig,
                                                     nullptr,
                                                     [](auto m)
                                                     {
                                                         return m->paramQuantities[M::CHANCE]->getDisplayValueString();
                                                     });
        srlt->box.pos.y = kPos - srlt->box.size.y * 0.5;
        addChild(srlt);

        addInput(createInputCentered<PJ301MPort>(rack::Vec(60, kPos + 25),
                                                 module,
                                                 M::CHANCE_CV));
    }

    {
        float kPos = 120;
        bg->addLabel(rack::Vec(10, kPos), "Scale", 12, NVG_ALIGN_MIDDLE | NVG_ALIGN_LEFT);
        addParam(createParamCentered<RoundSmallBlackKnob>(Vec(60, kPos), module, M::RNG_SCALE));
        auto srlt = DotMatrixLightTextWidget::create(rack::Vec(80, kPos),
                                                     module, dmdig,
                                                     nullptr,
                                                     [](auto m)
                                                     {
                                                         return m->paramQuantities[M::RNG_SCALE]->getDisplayValueString();
                                                     });
        srlt->box.pos.y = kPos - srlt->box.size.y * 0.5;
        addChild(srlt);
    }

    rack::Vec outP;
    outP.x = 20;
    outP.y = 170;
    addParam(createParamCentered<CKSS>(outP, module, M::LATCH));
    auto lp = outP;
    lp.y -= 12;
    bg->addLabel(lp, "latch", 11, NVG_ALIGN_BOTTOM | NVG_ALIGN_CENTER);;

    outP.x += 30;
    addParam(createParamCentered<CKSS>(outP, module, M::RNG_IS_UNI));
    lp = outP;
    lp.y -= 12;
    bg->addLabel(lp, "polar", 11, NVG_ALIGN_BOTTOM | NVG_ALIGN_CENTER);;


    {
        auto h = 120;
        bg->addRoundedBorder(Vec(5, RACK_HEIGHT - (h + 30)), Vec(box.size.x - 10, 38),
                             baconpaul::rackplugs::BaconStyle::HIGHLIGHT_BG);
        bg->addLabel(Vec(20, RACK_HEIGHT - (h+12)), "A", 11, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM,
                     baconpaul::rackplugs::BaconStyle::DEFAULT_HIGHLIGHT_LABEL);
        bg->addLabel(Vec(20, RACK_HEIGHT - h), "gate", 11, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM,
                     baconpaul::rackplugs::BaconStyle::DEFAULT_HIGHLIGHT_LABEL);
        addOutput(
            createOutput<PJ301MPort>(Vec(35, RACK_HEIGHT - (h+24)), module, M::A_CLOCK));

        bg->addLabel(Vec(75, RACK_HEIGHT - (h+12)), "A", 11, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM,
                     baconpaul::rackplugs::BaconStyle::DEFAULT_HIGHLIGHT_LABEL);
        bg->addLabel(Vec(75, RACK_HEIGHT - h), "rng", 11, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM,
                     baconpaul::rackplugs::BaconStyle::DEFAULT_HIGHLIGHT_LABEL);
        addOutput(
            createOutput<PJ301MPort>(Vec(90, RACK_HEIGHT - (h+24)), module, M::A_RNG));

    }
    {
        auto h = 80;
        bg->addRoundedBorder(Vec(5, RACK_HEIGHT - (h + 30)), Vec(box.size.x - 10, 38),
                             baconpaul::rackplugs::BaconStyle::HIGHLIGHT_BG);
        bg->addLabel(Vec(20, RACK_HEIGHT - (h+12)), "B", 11, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM,
                     baconpaul::rackplugs::BaconStyle::DEFAULT_HIGHLIGHT_LABEL);
        bg->addLabel(Vec(20, RACK_HEIGHT - h), "gate", 11, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM,
                     baconpaul::rackplugs::BaconStyle::DEFAULT_HIGHLIGHT_LABEL);
        addOutput(
            createOutput<PJ301MPort>(Vec(35, RACK_HEIGHT - (h+24)), module, M::B_CLOCK));

        bg->addLabel(Vec(75, RACK_HEIGHT - (h+12)), "B", 11, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM,
                     baconpaul::rackplugs::BaconStyle::DEFAULT_HIGHLIGHT_LABEL);
        bg->addLabel(Vec(75, RACK_HEIGHT - h), "rng", 11, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM,
                     baconpaul::rackplugs::BaconStyle::DEFAULT_HIGHLIGHT_LABEL);
        addOutput(
            createOutput<PJ301MPort>(Vec(90, RACK_HEIGHT - (h+24)), module, M::B_RNG));

    }

    {
        auto h = 40;
        bg->addRoundedBorder(Vec(5, RACK_HEIGHT - (h + 30)), Vec(70, 38),
                             baconpaul::rackplugs::BaconStyle::INPUT_BG);
        bg->addLabel(Vec(20, RACK_HEIGHT - (h+12)), "Clock", 11, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM,
                     baconpaul::rackplugs::BaconStyle::DEFAULT_LABEL);
        bg->addLabel(Vec(20, RACK_HEIGHT - h), "in", 11, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM,
                     baconpaul::rackplugs::BaconStyle::DEFAULT_LABEL);
        addInput(
            createInput<PJ301MPort>(Vec(35, RACK_HEIGHT - (h+24)), module, M::CLOCK_IN));


    }
}

Model *modelLuckyHold = createModel<LuckyHold, LuckyHoldWidget>("LuckyHold");

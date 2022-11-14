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
        RNG_IS_NORMAL,
        LATCN_STYLE,
        NUM_PARAMS
    };

    enum InputIds
    {
        CLOCK_IN,
        NUM_INPUTS
    };

    enum OutputIds
    {
        CLOCK_OUT,
        RNG,
        A_CLOCK,
        A_RNG,
        B_CLOCK,
        B_RNG,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        POLY_COUNT_LIGHT,
        CHANCE_LIGHT,
        SCALE_LIGHT,
        GATE_0,
        NUM_LIGHTS = GATE_0 + MAX_POLY,
    };

    std::default_random_engine gen;
    std::uniform_real_distribution<float> ud;

    LuckyHold() : bp::BaconModule(), ud(0,1)
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        auto s = configParam(POLY_COUNT, 1, MAX_POLY, 1, "Polyphony");
        s->snapEnabled = true;

        configParam(CHANCE, -1, 1, 0, "A (-1) or B (1) chance?", "%", 0, 100);
        configParam(RNG_SCALE, 0, 10, 10, "RNG Scale?", "V");

        configBypass(CLOCK_IN, CLOCK_OUT);
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
            lights[POLY_COUNT_LIGHT].value = nChan;
        }

        if (inTrig.process(inputs[CLOCK_IN].getVoltageSum()))
        {
            for (int c = 0; c < nChan; ++c)
            {
                auto rv = ud(gen), ch = ud(gen) * 2 - 1;

                if (ch > params[CHANCE].getValue())
                {
                    if (!bGate[c])
                        rB[c] = rv;
                    aGate[c] = false;
                    bGate[c] = true;
                }
                else
                {
                    if (!aGate[c])
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

        for (int c = 0; c < nChan; ++c)
        {
            outputs[A_CLOCK].setVoltage(aGate[c] * 10, c);
            outputs[B_CLOCK].setVoltage(bGate[c] * 10, c);
            outputs[A_RNG].setVoltage(rA[c] * 10, c);
            outputs[B_RNG].setVoltage(rB[c] * 10, c);
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
    }

    {
        float kPos = 95;
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

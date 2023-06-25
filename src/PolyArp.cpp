#include "BaconPlugs.hpp"
#include <initializer_list>

#include "BaconModule.hpp"
#include "BaconModuleWidget.h"

#include <random>

namespace bp = baconpaul::rackplugs;



struct PolyArp : virtual bp::BaconModule
{
    static constexpr  int numArps{4};
    enum ParamIds
    {
        ARPCOUNT,
        OCT_START,
        OCT_END,
        NUM_PARAMS
    };

    enum InputIds
    {
        POLY_VOCT_IN,
        CLOCK_IN,
        NUM_INPUTS
    };

    enum OutputIds
    {
        MONO_VOCT_OUT,
        POLY_VOCT_OUT = MONO_VOCT_OUT + numArps,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        ARPCOUNT_LIGHT,
        OCTSTART_LIGHT,
        OCTEND_LIGHT,
        NUM_LIGHTS,
    };

    PolyArp() : bp::BaconModule()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(ARPCOUNT, 1, numArps, 1, "Number of Arpeggios")->snapEnabled = true;
        configParam(OCT_START, -4, 0, 0, "Start Octave Range")->snapEnabled = true;
        configParam(OCT_END, 1, 4, 1, "End Octave Range")->snapEnabled = true;
    }

    static constexpr int checkOnEvery{8};
    int every{checkOnEvery};
    int outPoly{1};

    dsp::SchmittTrigger onClock;
    void process(const ProcessArgs &args) override
    {
        if (every == checkOnEvery)
        {
            every = 0;
            outPoly = std::round(params[ARPCOUNT].getValue());

            lights[ARPCOUNT_LIGHT].value = outPoly;
            lights[OCTSTART_LIGHT].value = params[OCT_START].getValue();
            lights[OCTEND_LIGHT].value = params[OCT_END].getValue();
        }
        else
        {
            every ++;
        }
        outputs[POLY_VOCT_OUT].setChannels(outPoly);
        for (int i=0; i<numArps; ++i)
            outputs[MONO_VOCT_OUT + i].setChannels(1);

        if (onClock.process(inputs[CLOCK_IN].getVoltage()))
        {
            for (int i=0; i<numArps; ++i)
            {
                dupChannel[i] = rand() % inputs[POLY_VOCT_IN].getChannels();
                auto os = std::clamp((int)std::round(params[OCT_START].getValue()), -4, 0);
                auto oe = std::clamp((int)std::round(params[OCT_END].getValue()), 1, 4);
                octOff[i] = rand() % (oe - os) + os;
            }
        }
        for (int i=0;i<numArps;++i)
        {
            auto voltI = inputs[POLY_VOCT_IN].getVoltage(dupChannel[i]) + octOff[i];
            outputs[MONO_VOCT_OUT + i].setVoltage(voltI);
            outputs[POLY_VOCT_OUT].setVoltage(voltI, i);
        }
    }

    int dupChannel[4]{0,0,0,0};
    int octOff[4]{0,0,0,0};

    bool patternStringDirty = true;
    std::string patternString = "pattern";
    static bool getPatternNameDirty(Module *that)
    {
        return dynamic_cast<PolyArp *>(that)->patternStringDirty;
    }
    static std::string getPatternName(Module *that)
    {
        dynamic_cast<PolyArp *>(that)->patternStringDirty = false;
        return dynamic_cast<PolyArp *>(that)->patternString;
    }
};

struct PolyArpWidget : bp::BaconModuleWidget
{
    typedef PolyArp M;
    PolyArpWidget(PolyArp *model);
};

PolyArpWidget::PolyArpWidget(PolyArp *m) : bp::BaconModuleWidget()
{
    setModule(m);
    box.size = Vec(SCREW_WIDTH * 9, RACK_HEIGHT);

    BaconBackground *bg = new BaconBackground(box.size, "PolyArp");
    addChild(bg->wrappedInFramebuffer());

    {
        auto h = RACK_HEIGHT - 60;
        bg->addRoundedBorder(Vec(5, RACK_HEIGHT - (h + 30)), Vec(box.size.x - 10, 35),
                             baconpaul::rackplugs::BaconStyle::INPUT_BG);
        bg->addLabel(Vec(20, RACK_HEIGHT - (h+12)), "Poly", 11, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM,
                     baconpaul::rackplugs::BaconStyle::DEFAULT_LABEL);
        bg->addLabel(Vec(20, RACK_HEIGHT - h), "v/oct", 11, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM,
                     baconpaul::rackplugs::BaconStyle::DEFAULT_LABEL);
        addInput(
            createInput<PJ301MPort>(Vec(35, RACK_HEIGHT - (h+24)), module, M::POLY_VOCT_IN ));

        bg->addLabel(Vec(80, RACK_HEIGHT - (h+12)), "Clock", 11, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM,
                     baconpaul::rackplugs::BaconStyle::DEFAULT_LABEL);
        bg->addLabel(Vec(80, RACK_HEIGHT - h), "In", 11, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM,
                     baconpaul::rackplugs::BaconStyle::DEFAULT_LABEL);
        addInput(
            createInput<PJ301MPort>(Vec(95, RACK_HEIGHT - (h+24)), module, M::CLOCK_IN ));

    }

    {
        auto h = RACK_HEIGHT - 98;
        auto yp = RACK_HEIGHT -( h + 30);
        bg->addRoundedBorder(Vec(5, RACK_HEIGHT - (h + 30)), Vec(box.size.x - 10, 35 + 60),
                             baconpaul::rackplugs::BaconStyle::BG);
        bg->addLabel(Vec(20, RACK_HEIGHT - (h+12)), "Num", 11, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM,
                     baconpaul::rackplugs::BaconStyle::DEFAULT_LABEL);
        bg->addLabel(Vec(20, RACK_HEIGHT - h), "Arps", 11, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM,
                     baconpaul::rackplugs::BaconStyle::DEFAULT_LABEL);

        {
            addParam(
                createParamCentered<RoundSmallBlackKnob>(Vec(60, yp + 35 * 0.5), module, M::ARPCOUNT));
            auto msg = MultiDigitSevenSegmentLight<BlueLight, 2, 2>::create(
                rack::Vec(90, yp + 35 * 0.5), module, M::ARPCOUNT_LIGHT);
            msg->box.pos.y -= msg->box.size.y * 0.5;
            addChild(msg);
        }

        yp += 30;
        {
            bg->addLabel(Vec(20, RACK_HEIGHT - (h+12) + 30), "Oct", 11, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM,
                         baconpaul::rackplugs::BaconStyle::DEFAULT_LABEL);
            bg->addLabel(Vec(20, RACK_HEIGHT - h + 30), "Start", 11, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM,
                         baconpaul::rackplugs::BaconStyle::DEFAULT_LABEL);

            addParam(
                createParamCentered<RoundSmallBlackKnob>(Vec(60, yp + 35 * 0.5), module, M::OCT_START));
            auto msg = MultiDigitSevenSegmentLight<BlueLight, 2, 2>::create(
                rack::Vec(90, yp + 35 * 0.5), module, M::OCTSTART_LIGHT);
            msg->box.pos.y -= msg->box.size.y * 0.5;
            addChild(msg);
        }

        yp += 30;
        {
            bg->addLabel(Vec(20, RACK_HEIGHT - (h+12) + 60), "Oct", 11, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM,
                         baconpaul::rackplugs::BaconStyle::DEFAULT_LABEL);
            bg->addLabel(Vec(20, RACK_HEIGHT - h + 60), "End", 11, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM,
                         baconpaul::rackplugs::BaconStyle::DEFAULT_LABEL);

            addParam(
                createParamCentered<RoundSmallBlackKnob>(Vec(60, yp + 35 * 0.5), module, M::OCT_END));
            auto msg = MultiDigitSevenSegmentLight<BlueLight, 2, 2>::create(
                rack::Vec(90, yp + 35 * 0.5), module, M::OCTEND_LIGHT);
            msg->box.pos.y -= msg->box.size.y * 0.5;
            addChild(msg);
        }
    }

    {
        auto yp = 167;
        bg->addRoundedBorder(Vec(5, yp), Vec(box.size.x - 10, 70),
                             baconpaul::rackplugs::BaconStyle::BG);

        addChild(DotMatrixLightTextWidget::create(
            Vec(10, 170),
            module, 10, PolyArp::getPatternNameDirty, PolyArp::getPatternName));

    }

    for (int i=0; i<4; ++i )
    {
        auto h = 30;
        auto yp = RACK_HEIGHT - (h+30) - ( i < 2 ? 40 : 0) - 40;
        auto xp = (i % 2 == 0 ? 5 : 70);
        bg->addRoundedBorder(Vec(xp, yp), Vec(61, h + 5),
                             baconpaul::rackplugs::BaconStyle::HIGHLIGHT_BG);
        bg->addLabel(Vec(xp + 4, yp + 5), "Arp", 11, NVG_ALIGN_LEFT | NVG_ALIGN_TOP,
                     baconpaul::rackplugs::BaconStyle::DEFAULT_HIGHLIGHT_LABEL);
        bg->addLabel(Vec(xp + 4, yp + 20), ("# " + std::to_string(i+1)).c_str(), 11, NVG_ALIGN_LEFT | NVG_ALIGN_TOP,
                     baconpaul::rackplugs::BaconStyle::DEFAULT_HIGHLIGHT_LABEL);
        addOutput(
            createOutput<PJ301MPort>(Vec(xp + 34, yp + 6), module, M::MONO_VOCT_OUT + i));
    }

    {
        auto h = 30;
        auto yp = RACK_HEIGHT - (h+30);
        auto xp = 5;
        bg->addRoundedBorder(Vec(xp, yp), Vec(126, h + 5),
                             baconpaul::rackplugs::BaconStyle::HIGHLIGHT_BG);
        bg->addLabel(Vec(xp + 4, yp + 5), "Polyphonic", 11, NVG_ALIGN_LEFT | NVG_ALIGN_TOP,
                     baconpaul::rackplugs::BaconStyle::DEFAULT_HIGHLIGHT_LABEL);
        bg->addLabel(Vec(xp + 4, yp + 20), "Arpeggios", 11, NVG_ALIGN_LEFT | NVG_ALIGN_TOP,
                     baconpaul::rackplugs::BaconStyle::DEFAULT_HIGHLIGHT_LABEL);
        addOutput(
            createOutput<PJ301MPort>(Vec(xp + 34 + 65, yp + 6), module, M::POLY_VOCT_OUT));
    }
}

Model *modelPolyArp = createModel<PolyArp, PolyArpWidget>("PolyArp");

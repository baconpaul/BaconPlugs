#include "BaconPlugs.hpp"
#include "ChipSym.hpp"
#include "BaconModule.hpp"
#include "BaconModuleWidget.h"


namespace bp = baconpaul::rackplugs;

struct ChipNoise : virtual bp::BaconModule
{
    enum ParamIds
    {
        NOISE_LENGTH,
        LONG_MODE,
        SHORT_LEN,
        PERIOD_93,
        NUM_PARAMS
    };

    enum InputIds
    {
        NOISE_LENGTH_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        NOISE_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        NOISE_FROM_INPUT,
        NOISE_FROM_KNOB,

        NOISE_LENGTH_LIGHT,

        PERIOD_93_LIGHT,

        USING_93,

        NUM_LIGHTS
    };

    std::unique_ptr<ChipSym::NESNoise> noise = nullptr;
    int prior_shortlen;
    bool prior_longmode;

    ChipNoise()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configSwitch(LONG_MODE, 0, 1, 1, "Use Long Sequence", {"Off", "On"});
        configParam(NOISE_LENGTH, 0, 15, 7, "Length of sequence");
        configSwitch(SHORT_LEN, 0, 1, 1, "Short Sequence Length", {"31", "93"});
        configParam(PERIOD_93, 0, 351, 17, "Which Length-93 Sequence");

        configInput(NOISE_LENGTH_INPUT, "Wavelength (akin to tone roughly)");
        configOutput(NOISE_OUTPUT, "The Noise");
        prior_shortlen = 1;
        prior_longmode = false;
    }

    void process(const ProcessArgs &args) override
    {
        if (noise == nullptr)
        {
            noise.reset(new ChipSym::NESNoise(-5, 5, args.sampleRate));
        }

        lights[NOISE_FROM_KNOB].value = !inputs[NOISE_LENGTH_INPUT].isConnected();
        lights[NOISE_FROM_INPUT].value = inputs[NOISE_LENGTH_INPUT].isConnected();

        unsigned int nl = (unsigned int)clamp(params[NOISE_LENGTH].getValue(), 0.0f, 15.0f);
        if (inputs[NOISE_LENGTH_INPUT].isConnected())
            nl = (unsigned int)clamp(inputs[NOISE_LENGTH_INPUT].getVoltage() * 1.5, 0.0f, 15.0f);

        lights[NOISE_LENGTH_LIGHT].value = nl;
        noise->setPeriod(nl);

        int p93 = (int)params[PERIOD_93].getValue();
        lights[PERIOD_93_LIGHT].value = p93;
        if (params[LONG_MODE].getValue() == 0 && params[SHORT_LEN].getValue() == 1)
        {
            noise->set93Key(p93);
            lights[USING_93].value = 1;
        }
        else
        {
            lights[USING_93].value = 0;
        }

        bool tmpNoiseFlag = (params[LONG_MODE].getValue() == 0);
        if (tmpNoiseFlag != prior_longmode)
        {
            prior_longmode = tmpNoiseFlag;
            noise->setModeFlag(prior_longmode);
        }

        if (params[SHORT_LEN].getValue() != prior_shortlen)
        {
            prior_shortlen = params[SHORT_LEN].getValue();
            if (prior_shortlen == 1)
            {
                noise->setShortLength(ChipSym::NESNoise::SHORT_93);
            }
            else
            {
                noise->setShortLength(ChipSym::NESNoise::SHORT_31);
            }
        }

        outputs[NOISE_OUTPUT].setVoltage(noise->step());
    }
};

struct ChipNoiseWidget : bp::BaconModuleWidget
{
    ChipNoiseWidget(ChipNoise *module);
};

ChipNoiseWidget::ChipNoiseWidget(ChipNoise *module)
{
    setModule(module);
    box.size = Vec(SCREW_WIDTH * 6, RACK_HEIGHT);

    BaconBackground *bg = new BaconBackground(box.size, "ChipNoise");
    addChild(bg->wrappedInFramebuffer());

    // Control the noise length
    bg->addRoundedBorder(Vec(8, 45), Vec(SCREW_WIDTH * 6 - 16, 75));
    bg->addLabel(Vec(bg->cx() + 7, 55), "wave", 11, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
    bg->addLabel(Vec(bg->cx() + 5, 66), "length", 11, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
    Vec inP = Vec(16, 53);
    addInput(createInput<PJ301MPort>(inP, module, ChipNoise::NOISE_LENGTH_INPUT));
    addChild(createLight<SmallLight<BlueLight>>(inP.minus(Vec(4, 4)), module,
                                                ChipNoise::NOISE_FROM_INPUT));

    int ybot = 120;
    addParam(
        createParam<RoundSmallBlackKnob>(Vec(16, ybot - 3 - 28), module, ChipNoise::NOISE_LENGTH));
    addChild(createLight<SmallLight<BlueLight>>(Vec(16 - 4, ybot - 3 - 28 - 4), module,
                                                ChipNoise::NOISE_FROM_KNOB));
    addChild(MultiDigitSevenSegmentLight<BlueLight, 2, 2>::create(Vec(47, ybot - 5 - 24), module,
                                                                  ChipNoise::NOISE_LENGTH_LIGHT));

    bg->addRoundedBorder(Vec(8, 135), Vec(SCREW_WIDTH * 6 - 16, 160));
    bg->addLabel(Vec(bg->cx(), 155), "Sequence", 13, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM);
    addParam(createParam<NKK_UpDown>(Vec(bg->cx() - 32, 175), module, ChipNoise::LONG_MODE));
    addParam(createParam<NKK_UpDown>(Vec(bg->cx() + 2, 175), module, ChipNoise::SHORT_LEN));
    bg->addLabel(Vec(bg->cx() + 16 - 32, 160), "long", 11, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
    bg->addLabel(Vec(bg->cx() + 16 - 32, 223), "short", 11, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);

    bg->addLabel(Vec(bg->cx() + 16 + 2, 160), "93", 11, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
    bg->addLabel(Vec(bg->cx() + 16 + 2, 223), "31", 11, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);

    bg->addLabel(Vec(bg->cx(), 258), "Which 93 seq", 11, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM);

    addChild(MultiDigitSevenSegmentLight<BlueLight, 2, 3>::create(Vec(50 - 14, 262), module,
                                                                  ChipNoise::PERIOD_93_LIGHT));

    addParam(createParam<RoundSmallBlackKnob>(Vec(11, 262), module, ChipNoise::PERIOD_93));
    addChild(createLight<SmallLight<BlueLight>>(Vec(12, 249), module, ChipNoise::USING_93));

    // Output port
    Vec outP = Vec(bg->cx(24), RACK_HEIGHT - 15 - 43);
    bg->addPlugLabel(outP, BaconBackground::SIG_OUT, "out");
    addOutput(createOutput<PJ301MPort>(outP, module, ChipNoise::NOISE_OUTPUT));
}

Model *modelChipNoise = createModel<ChipNoise, ChipNoiseWidget>("ChipNoise");

#include "BaconPlugs.hpp"
#include "BaconModule.hpp"
#include "BaconModuleWidget.h"

namespace bp = baconpaul::rackplugs;

struct HarMoNee : bp::BaconModule
{
    enum ParamIds
    {
        UP_OR_DOWN,
        HALF_STEP,
        WHOLE_STEP,
        MINOR_THIRD,
        MAJOR_THIRD,
        FIFTH,
        OCTAVE,
        GLISS_RATE,
        NUM_PARAMS
    };
    enum InputIds
    {
        SOURCE_INPUT,
        UP_OR_DOWN_CV,
        HALF_STEP_CV,
        WHOLE_STEP_CV,
        MINOR_THIRD_CV,
        MAJOR_THIRD_CV,
        FIFTH_CV,
        OCTAVE_CV,
        NUM_INPUTS
    };
    enum OutputIds
    {
        ECHO_OUTPUT,
        INCREASED_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        UP_LIGHT,
        DOWN_LIGHT,
        HALF_STEP_LIGHT,
        WHOLE_STEP_LIGHT,
        MINOR_THIRD_LIGHT,
        MAJOR_THIRD_LIGHT,
        FIFTH_LIGHT,
        OCTAVE_LIGHT,

        DIGIT_LIGHT,

        NUM_LIGHTS
    };

    std::vector<float> offsets;

    float priorOffset;
    float targetOffset;
    int offsetCount;

    HarMoNee()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(GLISS_RATE, 0.1, 1, 0.55, "Glissando rate when things change");
        configParam(UP_OR_DOWN, 0, 1, 1, "Increase or Decrease by interval");
        const char *labels[] = {"1/2", "W", "m3", "III", "V", "O"};
        for (int i = HarMoNee::HALF_STEP; i <= HarMoNee::OCTAVE; ++i)
        {
            int v;
            if (i == HarMoNee::MAJOR_THIRD)
            {
                v = 1;
            }
            else
            {
                v = 0;
            }
            configParam(i, 0, 1, v, "Offset by " + std::string(labels[i - HALF_STEP]));
        }

        for (int i = 0; i < OCTAVE; ++i)
            offsets.push_back(0);

        configInput(UP_OR_DOWN_CV, "Gate to force toggle to UP");
        configInput(HALF_STEP_CV, "Gate to add an extra half step");
        configInput(WHOLE_STEP_CV, "Gate to add an extra whole step");
        configInput(MINOR_THIRD_CV, "Gate to add an extra minor third");
        configInput(MAJOR_THIRD_CV, "Gate to add an extra major third");
        configInput(FIFTH_CV, "Gate to add an extra fifth");
        configInput(OCTAVE_CV, "Gate to add an extra octave");

        configInput(SOURCE_INPUT, "V/Oct input to be harmonized");
        configOutput(ECHO_OUTPUT, "Echo of Input");
        configOutput(INCREASED_OUTPUT, "Harmonized Output");

        offsets[HALF_STEP] = 1;
        offsets[WHOLE_STEP] = 2;
        offsets[MINOR_THIRD] = 3;
        offsets[MAJOR_THIRD] = 4;
        offsets[FIFTH] = 7;
        offsets[OCTAVE] = 12;
        priorOffset = 0;
        targetOffset = 0;
        offsetCount = 0;

        configBypass(SOURCE_INPUT, ECHO_OUTPUT);
        configBypass(SOURCE_INPUT, INCREASED_OUTPUT);
    }

    void process(const ProcessArgs &args) override;
};

void HarMoNee::process(const ProcessArgs &args)
{
    /* TODO

       Display the shift
       Tests
    */
    int nChan = inputs[SOURCE_INPUT].getChannels();
    outputs[ECHO_OUTPUT].setChannels(nChan);
    outputs[INCREASED_OUTPUT].setChannels(nChan);

    float offsetI = 0;
    float uod = (params[UP_OR_DOWN].getValue() > 0) ? 1.0 : -1.0;
    if (uod > 0)
    {
        lights[UP_LIGHT].value = 1;
        lights[DOWN_LIGHT].value = 0;
    }
    else
    {
        lights[DOWN_LIGHT].value = 1;
        lights[UP_LIGHT].value = 0;
    }

    int ld = HALF_STEP_LIGHT - HALF_STEP;
    for (int i = HALF_STEP; i <= OCTAVE; ++i)
    {
        if (params[i].getValue() > 0)
        {
            lights[i + ld].value = 1.0;
            offsetI += offsets[i];
        }
        else
        {
            lights[i + ld].value = 0.0;
        }
    }

    /*
    ** And the CV inputs
    */
    ld = HALF_STEP_LIGHT - HALF_STEP_CV;
    for (int i = UP_OR_DOWN_CV; i <= OCTAVE_CV; ++i)
    {
        if (inputs[i].getVoltage() > 2.0f)
        {
            if (i == UP_OR_DOWN_CV)
            {
                lights[UP_LIGHT].value = 1;
                lights[DOWN_LIGHT].value = 0;
                uod = 1;
            }
            else
            {
                lights[i + ld].value = 1.0;
                offsetI += offsets[i - UP_OR_DOWN_CV + UP_OR_DOWN];
            }
        }
    }

    lights[DIGIT_LIGHT].value = offsetI;

    offsetI = uod * offsetI / 12.0;

    int shift_time = args.sampleRate / 10 * params[GLISS_RATE].getValue();
    /* Glissando state management which basically makes it so that
       you don't click when you change a value

       - priorOffset is the place we are starting the glide from
       - targetOffset is where we are headed
       - offsetI is where the switches are set
       - offsetCount is how far we are in.

       when we aren't in a glissando offsetCount will be 0 and
       all three will be the same. offsetCount being
       non-zero is the same as in-gliss.
     */
    bool inGliss = offsetCount != 0;
    if (!inGliss)
    {
        // We are not sliding. Should we be?
        if (offsetI != priorOffset)
        {
            targetOffset = offsetI;
            offsetCount = 1;
            inGliss = true;
        }
    }

    if (inGliss)
    {
        // If the target == the offset we haven't changed anything so
        // just march along linear time
        if (offsetI != targetOffset)
        {
            float lastKnown =
                ((shift_time - offsetCount) * priorOffset + offsetCount * targetOffset) /
                shift_time;
            targetOffset = offsetI;
            priorOffset = lastKnown;
            offsetCount = 0;
        }

        offsetI = ((shift_time - offsetCount) * priorOffset + offsetCount * offsetI) / shift_time;

        offsetCount++;
    }

    // Finally if we are done, reset it all to zero
    if (offsetCount == shift_time)
    {
        offsetCount = 0;
        priorOffset = offsetI;
        targetOffset = offsetI;
    }

    for (int i = 0; i < nChan; ++i)
    {
        float in = inputs[SOURCE_INPUT].getVoltage(i);
        float increased = in + offsetI;

        outputs[ECHO_OUTPUT].setVoltage(in, i);
        outputs[INCREASED_OUTPUT].setVoltage(increased, i);
    }
}

struct HarMoNeeWidget : bp::BaconModuleWidget
{
    HarMoNeeWidget(HarMoNee *model);
};

HarMoNeeWidget::HarMoNeeWidget(HarMoNee *model)
{
    setModule(model);
    box.size = Vec(SCREW_WIDTH * 8, RACK_HEIGHT);

    BaconBackground *bg = new BaconBackground(box.size, "HarMoNee");

    addChild(bg->wrappedInFramebuffer());

    Vec iPos = Vec(7, RACK_HEIGHT - 15 - 43);

    bg->addPlugLabel(iPos, BaconBackground::SIG_IN, "in");
    addInput(createInput<PJ301MPort>(iPos, module, HarMoNee::SOURCE_INPUT));

    iPos = Vec(box.size.x / 2 - 12, RACK_HEIGHT - 15 - 43);
    bg->addPlugLabel(iPos, BaconBackground::SIG_IN, "gliss");
    addParam(createParam<RoundSmallBlackKnob>(iPos, module, HarMoNee::GLISS_RATE));

    iPos = Vec(box.size.x - 24 - 7, RACK_HEIGHT - 15 - 43);

    bg->addPlugLabel(iPos, BaconBackground::SIG_OUT, "harm");
    addOutput(createOutput<PJ301MPort>(iPos, module, HarMoNee::INCREASED_OUTPUT));

    // NKK is 32 x 44
    int x0 = 53;
    addParam(createParam<NKK_UpDown>(Vec(x0, 26), module, HarMoNee::UP_OR_DOWN));
    addInput(
        createInput<PJ301MPort>(Vec(x0 + SizeTable<NKK>::X + 5, 26 + diffY2c<NKK, PJ301MPort>()),
                                module, HarMoNee::UP_OR_DOWN_CV));
    bg->addLabel(Vec(x0 - 6, 26 + 22 - 4 - 5 - 5), "up", 12, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM);
    addChild(createLight<MediumLight<GreenLight>>(Vec(x0 - 10, 26 + 22 - 4 - 5), module,
                                                  HarMoNee::UP_LIGHT));

    bg->addLabel(Vec(x0 - 6, 26 + 22 - 4 + 5 + 8 + 7), "dn", 12, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
    addChild(createLight<MediumLight<RedLight>>(Vec(x0 - 10, 26 + 22 - 4 + 5), module,
                                                HarMoNee::DOWN_LIGHT));

    addChild(MultiDigitSevenSegmentLight<BlueLight, 2, 2>::create(Vec(7, 38), module,
                                                                  HarMoNee::DIGIT_LIGHT));

    int x = 50;
    int y = 26 + 45 + 14;
    int ld = HarMoNee::HALF_STEP_LIGHT - HarMoNee::HALF_STEP;

    const char *labels[] = {"1/2", "W", "m3", "III", "V", "O"};
    for (int i = HarMoNee::HALF_STEP; i <= HarMoNee::OCTAVE; ++i)
    {
        addParam(createParam<CKSS>(Vec(x, y), module, i));
        bg->addLabel(Vec(x - 14, y + SizeTable<PJ301MPort>::Y / 2), labels[i - HarMoNee::HALF_STEP],
                     14, NVG_ALIGN_RIGHT | NVG_ALIGN_MIDDLE);
        addChild(createLight<SmallLight<BlueLight>>(
            Vec(x - 10, y + SizeTable<PJ301MPort>::Y / 2 - 4), module, i + ld));
        addInput(createInput<PJ301MPort>(
            Vec(x + SizeTable<CKSS>::X + 5, y + diffY2c<CKSS, PJ301MPort>()), module,
            HarMoNee::HALF_STEP_CV + i - HarMoNee::HALF_STEP));

        y += 35;
    }
}

Model *modelHarMoNee = createModel<HarMoNee, HarMoNeeWidget>("HarMoNee");

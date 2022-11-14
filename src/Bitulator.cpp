#include "BaconPlugs.hpp"
#include "BaconModule.hpp"
#include "BaconModuleWidget.h"

namespace bp = baconpaul::rackplugs;
/*
** ToDo:
**   Add lights for on/off
**   Add a 7 segment display for step count
*/

struct Bitulator : bp::BaconModule
{
    enum ParamIds
    {
        WET_DRY_MIX,
        STEP_COUNT,
        AMP_LEVEL,
        BITULATE,
        CLIPULATE,
        NUM_PARAMS
    };

    enum InputIds
    {
        SIGNAL_INPUT,
        BIT_CV,
        AMP_CV,
        MIX_CV,
        NUM_INPUTS
    };

    enum OutputIds
    {
        CRUNCHED_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        BITULATING_LIGHT,
        CRUNCHING_LIGHT,
        NUM_LIGHTS
    };

    Bitulator()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(WET_DRY_MIX, 0.0, 1.0, 1.0, "Wet Dry Mix");
        configParam(STEP_COUNT, 2.0, 16.0, 6, "Number of steps");
        configParam(AMP_LEVEL, 1.0, 11.0, 1, "Amplificaiton level");
        configParam(BITULATE, 0.0, 1.0, 1, "Bittiness");
        configParam(CLIPULATE, 0.0, 1.0, 1, "Clipulation");

        configInput(SIGNAL_INPUT, "Signal");
        configInput(BIT_CV, "Bittiness Control Voltage");
        configInput(AMP_CV, "Amplification Control Voltage");
        configInput(MIX_CV, "Mix Control Voltage");
        configOutput(CRUNCHED_OUTPUT, "Output. Good luck!");

        lights[BITULATING_LIGHT].value = 1;
        lights[CRUNCHING_LIGHT].value = 1;

        configBypass(SIGNAL_INPUT, CRUNCHED_OUTPUT);
    }

    void process(const ProcessArgs &args) override
    {
        int nChan = inputs[SIGNAL_INPUT].getChannels();
        outputs[CRUNCHED_OUTPUT].setChannels(nChan);
        for (int i = 0; i < nChan; ++i)
        {
            float vin = inputs[SIGNAL_INPUT].getVoltage(i);
            float wd = params[WET_DRY_MIX].getValue() + inputs[MIX_CV].getPolyVoltage(i) / 10.0;
            wd = clamp(wd, 0.f, 1.f);
            // Signals are +/-5V signals of course. So

            float res = 0;
            if (params[BITULATE].getValue() > 0)
            {
                float qi = params[STEP_COUNT].getValue() / 2 +
                           inputs[BIT_CV].getPolyVoltage(i) / 10.0 * 14.0;
                float crunch = (int)((vin / 5.0) * qi) / qi * 5.0;

                res = crunch;
                lights[BITULATING_LIGHT].value = 1;
            }
            else
            {
                res = vin;
                lights[BITULATING_LIGHT].value = 0;
            }

            if (params[CLIPULATE].getValue() > 0)
            {
                float al = params[AMP_LEVEL].getValue() + inputs[AMP_CV].getPolyVoltage(i);
                res = clamp(res * al, -5.0f, 5.0f);
                lights[CRUNCHING_LIGHT].value = 1;
            }
            else
            {
                lights[CRUNCHING_LIGHT].value = 0;
            }

            outputs[CRUNCHED_OUTPUT].setVoltage(wd * res + (1.0 - wd) * vin, i);
        }
    }
};

struct BitulatorWidget : bp::BaconModuleWidget
{
    BitulatorWidget(Bitulator *model);
};

BitulatorWidget::BitulatorWidget(Bitulator *model)
{
    setModule(model);
    box.size = Vec(SCREW_WIDTH * 6, RACK_HEIGHT);

    BaconBackground *bg = new BaconBackground(box.size, "Bitulator");
    addChild(bg->wrappedInFramebuffer());
    bg->addDrawFunction([](NVGcontext *vg) {
        int xpos = 5 + 12 + SizeTable<RoundLargeBlackKnob>::X / 2;
        int ypos = 35 + 25 + SizeTable<RoundLargeBlackKnob>::Y / 2;
        for (int i = 0; i < 3; ++i)
        {
            nvgBeginPath(vg);
            nvgMoveTo(vg, xpos, ypos + i * 88);
            nvgLineTo(vg, xpos + 25, ypos + i * 88);
            nvgStrokeColor(vg, componentlibrary::SCHEME_BLACK);
            nvgStrokeWidth(vg, 1);
            nvgStroke(vg);
        }
    });

    Vec cr(5, 35), rs(box.size.x - 10, 80);
    bg->addRoundedBorder(cr, rs);
    bg->addLabel(Vec(box.size.x / 2, cr.y + 3), "Mix", 14, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);

    Vec knobPos = Vec(cr.x + 12, cr.y + 25);
    Vec knobCtr = knobPos.plus(Vec(18, 18));
    addParam(createParam<RoundLargeBlackKnob>(knobPos, module, Bitulator::WET_DRY_MIX));
    bg->addLabel(knobCtr.plus(Vec(8, 21)), "dist", 10, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
    bg->addLabel(knobCtr.plus(Vec(-8, 21)), "clean", 10, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);
    knobPos.x = cr.x + rs.x - 3 - 24;
    ;
    knobPos.y += diffY2c<RoundLargeBlackKnob, PJ301MPort>();
    addInput(createInput<PJ301MPort>(knobPos, module, Bitulator::MIX_CV));

    cr.y += 88;
    bg->addRoundedBorder(cr, rs);
    bg->addLabel(Vec(cr.x + 3, cr.y + 3), "Quantize", 14, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
    addChild(createLight<SmallLight<BlueLight>>(cr.plus(Vec(rs.x - 27, 6)), module,
                                                Bitulator::BITULATING_LIGHT));

    addParam(createParam<CKSS>(cr.plus(Vec(rs.x - 17, 5)), module, Bitulator::BITULATE));

    knobPos = Vec(cr.x + 12, cr.y + 25);
    knobCtr = knobPos.plus(Vec(18, 18));
    addParam(createParam<RoundLargeBlackKnob>(knobPos, module, Bitulator::STEP_COUNT));
    bg->addLabel(knobCtr.plus(Vec(8, 21)), "smth", 10, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
    bg->addLabel(knobCtr.plus(Vec(-8, 21)), "crnch", 10, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);
    knobPos.x = cr.x + rs.x - 3 - 24;
    ;
    knobPos.y += diffY2c<RoundLargeBlackKnob, PJ301MPort>();
    addInput(createInput<PJ301MPort>(knobPos, module, Bitulator::BIT_CV));

    cr.y += 88;
    bg->addRoundedBorder(cr, rs);
    bg->addLabel(Vec(cr.x + 3, cr.y + 3), "Amp/Clip", 14, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
    addChild(createLight<SmallLight<BlueLight>>(cr.plus(Vec(rs.x - 27, 6)), module,
                                                Bitulator::CRUNCHING_LIGHT));

    addParam(createParam<CKSS>(cr.plus(Vec(rs.x - 17, 5)), module, Bitulator::CLIPULATE));

    knobPos = Vec(cr.x + 12, cr.y + 25);
    knobCtr = knobPos.plus(Vec(18, 18));
    addParam(createParam<RoundLargeBlackKnob>(knobPos, module, Bitulator::AMP_LEVEL));
    bg->addLabel(knobCtr.plus(Vec(8, 21)), "11", 10, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
    bg->addLabel(knobCtr.plus(Vec(-8, 21)), "one", 10, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);
    knobPos.x = cr.x + rs.x - 3 - 24;
    ;
    knobPos.y += diffY2c<RoundLargeBlackKnob, PJ301MPort>();
    addInput(createInput<PJ301MPort>(knobPos, module, Bitulator::AMP_CV));

    Vec inP = Vec(10, RACK_HEIGHT - 15 - 43);
    Vec outP = Vec(box.size.x - 24 - 10, RACK_HEIGHT - 15 - 43);

    bg->addPlugLabel(inP, BaconBackground::SIG_IN, "in");
    addInput(createInput<PJ301MPort>(inP, module, Bitulator::SIGNAL_INPUT));

    bg->addPlugLabel(outP, BaconBackground::SIG_OUT, "out");
    addOutput(createOutput<PJ301MPort>(outP, module, Bitulator::CRUNCHED_OUTPUT));
}

Model *modelBitulator = createModel<Bitulator, BitulatorWidget>("Bitulator");

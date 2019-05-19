#include "BaconPlugs.hpp"

/*
** ToDo:
**   Add lights for on/off
**   Add a 7 segment display for step count
*/

struct Bitulator : Module {
    enum ParamIds {
        WET_DRY_MIX,
        STEP_COUNT,
        AMP_LEVEL,
        BITULATE,
        CLIPULATE,
        NUM_PARAMS
    };

    enum InputIds { SIGNAL_INPUT, NUM_INPUTS };

    enum OutputIds { CRUNCHED_OUTPUT, NUM_OUTPUTS };

    enum LightIds { BITULATING_LIGHT, CRUNCHING_LIGHT, NUM_LIGHTS };

    Bitulator() : Module() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(WET_DRY_MIX, 0.0, 1.0, 1.0, "Wet Dry Mix");
        configParam(STEP_COUNT, 2.0, 16.0, 6, "Number of steps");
        configParam(AMP_LEVEL, 1.0, 11.0, 1, "Amplificaiton level");
        configParam(BITULATE, 0.0, 1.0, 1, "Bittiness");
        configParam(CLIPULATE, 0.0, 1.0, 1, "Clipulation");

        lights[BITULATING_LIGHT].value = 1;
        lights[CRUNCHING_LIGHT].value = 1;
    }

    void process(const ProcessArgs &args) override {
        float vin = inputs[SIGNAL_INPUT].getVoltage();
        float wd = params[WET_DRY_MIX].getValue();

        // Signals are +/-5V signals of course. So

        float res = 0;
        if (params[BITULATE].getValue() > 0) {
            float qi = params[STEP_COUNT].getValue() / 2;
            float crunch = (int)((vin / 5.0) * qi) / qi * 5.0;

            res = crunch;
            lights[BITULATING_LIGHT].value = 1;
        } else {
            res = vin;
            lights[BITULATING_LIGHT].value = 0;
        }

        if (params[CLIPULATE].getValue() > 0) {
            float al = params[AMP_LEVEL].getValue();
            res = clamp(res * al, -5.0f, 5.0f);
            lights[CRUNCHING_LIGHT].value = 1;
        } else {
            lights[CRUNCHING_LIGHT].value = 0;
        }

        outputs[CRUNCHED_OUTPUT].setVoltage(wd * res + (1.0 - wd) * vin);
    }
};

struct BitulatorWidget : ModuleWidget {
    BitulatorWidget(Bitulator *model);
};

BitulatorWidget::BitulatorWidget(Bitulator *model) : ModuleWidget() {
    setModule(model);
    box.size = Vec(SCREW_WIDTH * 6, RACK_HEIGHT);

    BaconBackground *bg = new BaconBackground(box.size, "Bitulator");
    addChild(bg->wrappedInFramebuffer());

    int wdpos = 40;
    bg->addLabel(Vec(bg->cx(), wdpos), "Mix", 14,
                 NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
    bg->addLabel(Vec(bg->cx() + 10, wdpos + 72), "Wet", 13,
                 NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
    bg->addLabel(Vec(bg->cx() - 10, wdpos + 72), "Dry", 13,
                 NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);

    addParam(createParam<RoundHugeBlackKnob>(Vec(bg->cx(56), wdpos + 10),
                                             module, Bitulator::WET_DRY_MIX));

    Vec cr(5, 140), rs(box.size.x - 10, 70);
    bg->addRoundedBorder(cr, rs);
    bg->addLabel(Vec(bg->cx(), cr.y + 3), "Quantize", 14,
                 NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
    addChild(createLight<SmallLight<BlueLight>>(cr.plus(Vec(5, 5)), module,
                                                Bitulator::BITULATING_LIGHT));
    addParam(
        createParam<CKSS>(cr.plus(Vec(5, 25)), module, Bitulator::BITULATE));

    Vec knobPos = Vec(cr.x + rs.x - 36 - 12, cr.y + 18);
    Vec knobCtr = knobPos.plus(Vec(18, 18));
    addParam(createParam<RoundLargeBlackKnob>(knobPos, module,
                                              Bitulator::STEP_COUNT));
    bg->addLabel(knobCtr.plus(Vec(8, 21)), "smth", 10,
                 NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
    bg->addLabel(knobCtr.plus(Vec(-8, 21)), "crnch", 10,
                 NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);

    cr = Vec(5, 215);
    bg->addRoundedBorder(cr, rs);
    bg->addLabel(Vec(bg->cx(5), cr.y + 3), "Amp'n'Clip", 14,
                 NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
    addChild(createLight<SmallLight<BlueLight>>(cr.plus(Vec(5, 5)), module,
                                                Bitulator::CRUNCHING_LIGHT));
    addParam(
        createParam<CKSS>(cr.plus(Vec(5, 25)), module, Bitulator::CLIPULATE));
    knobPos = Vec(cr.x + rs.x - 36 - 12, cr.y + 18);
    knobCtr = knobPos.plus(Vec(18, 18));
    addParam(createParam<RoundLargeBlackKnob>(knobPos, module,
                                              Bitulator::AMP_LEVEL));
    bg->addLabel(knobCtr.plus(Vec(12, 21)), "11", 10,
                 NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
    bg->addLabel(knobCtr.plus(Vec(-8, 21)), "one", 10,
                 NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);

    Vec inP = Vec(10, RACK_HEIGHT - 15 - 43);
    Vec outP = Vec(box.size.x - 24 - 10, RACK_HEIGHT - 15 - 43);

    bg->addPlugLabel(inP, BaconBackground::SIG_IN, "in");
    addInput(createInput<PJ301MPort>(inP, module, Bitulator::SIGNAL_INPUT));

    bg->addPlugLabel(outP, BaconBackground::SIG_OUT, "out");
    addOutput(
        createOutput<PJ301MPort>(outP, module, Bitulator::CRUNCHED_OUTPUT));
}

Model *modelBitulator = createModel<Bitulator, BitulatorWidget>("Bitulator");

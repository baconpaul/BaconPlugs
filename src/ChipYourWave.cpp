
#include "BaconPlugs.hpp"
#include "ChipSym.hpp"

struct ChipYourWave : virtual Module {
    enum ParamIds {
        FREQ_KNOB,

        WAVEFORM_START,
        NUM_PARAMS = WAVEFORM_START + 32
    };

    enum InputIds { FREQ_CV, NUM_INPUTS };

    enum OutputIds { WAVE_OUTPUT, NUM_OUTPUTS };

    enum LightIds { NUM_LIGHTS };

    std::unique_ptr<ChipSym::NESArbitraryWaveform> narb = nullptr;

    ChipYourWave() : Module() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(FREQ_KNOB, -54.0, 54.0, 0.0, "Frequency");
        for (int i = 0; i < 16; ++i) {
            configParam(WAVEFORM_START + (15-i), 0, 1.0, i / 15.0f);
            configParam(WAVEFORM_START + 16+i, 0, 1.0, i / 15.0f);
        }
    }

    float digWFInSeconds(float pitchKnob, float pitchCV) {
        // This is the frequency tuning used in Fundamental/VCO so lets be
        // consistent
        float pitch = pitchKnob + pitchCV;
        float freq = 261.626f * powf(2.0f, pitch / 12.0f);
        // OK so now we have the frequency. We need the wavelength though.
        // Simple
        float wl = 1.0f / freq;

        return wl;
    }

    void process(const ProcessArgs &args) override {
        if (narb == nullptr) {
            narb.reset(
                new ChipSym::NESArbitraryWaveform(-5.0, 5.0, args.sampleRate));
            narb->setDigWavelength(2 << 8);
        }
        float dwf = digWFInSeconds(params[FREQ_KNOB].getValue(),
                                   12.0f * inputs[FREQ_CV].getVoltage());

        narb->setWavelengthInSeconds(dwf);

        for (int i = 0; i < 32; ++i)
            narb->setWaveformPoint(i, params[WAVEFORM_START + i].getValue());

        if (outputs[WAVE_OUTPUT].isConnected())
            outputs[WAVE_OUTPUT].setVoltage(narb->step());
    }
};

struct ChipYourWaveWidget : ModuleWidget {
    ChipYourWaveWidget(ChipYourWave *module);
};

ChipYourWaveWidget::ChipYourWaveWidget(ChipYourWave *module) : ModuleWidget() {
    setModule(module);
    box.size = Vec(SCREW_WIDTH * 23, RACK_HEIGHT);

    BaconBackground *bg = new BaconBackground(box.size, "ChipYourWave");
    addChild(bg->wrappedInFramebuffer());

    Vec outP = Vec(box.size.x - 40, 45 + 30);
    bg->addPlugLabel(outP, BaconBackground::SIG_OUT, "out");
    addOutput(
        createOutput<PJ301MPort>(outP, module, ChipYourWave::WAVE_OUTPUT));

    bg->addLabel(Vec(50, 45), "Freq", 14, NVG_ALIGN_LEFT | NVG_ALIGN_BOTTOM);
    addParam(createParam<RoundHugeBlackKnob>(Vec(10, 50), module,
                                             ChipYourWave::FREQ_KNOB));
    Vec fcv = Vec(56 + 20, 45 + 30);
    bg->addPlugLabel(fcv, BaconBackground::SIG_IN, "v/o");
    addInput(createInput<PJ301MPort>(fcv, module, ChipYourWave::FREQ_CV));

    bg->addLabel(Vec(bg->cx(), 135), "Draw your Digital Waveform Here", 14,
                 NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM);
    
    for (int i = 0; i < 32; ++i) {
        addParam(createParam<
                 NStepDraggableLEDWidget<16, RedGreenFromMiddleColorModel>>(
                     Vec(10 + 10 * i, 140), module, ChipYourWave::WAVEFORM_START + i));
    }
}

Model *modelChipYourWave =
    createModel<ChipYourWave, ChipYourWaveWidget>("ChipYourWave");


#include "BaconPlugs.hpp"
#include "ChipSym.hpp"

struct ChipYourWave : virtual Module {
    enum ParamIds {
        FREQ_KNOB,

        WAVEFORM_START,
        NUM_PARAMS = WAVEFORM_START + 32
    };

    enum InputIds { FREQ_CV, WAVE0_CV, NUM_INPUTS = WAVE0_CV + 32 };

    enum OutputIds { WAVE_OUTPUT, NUM_OUTPUTS };

    enum LightIds { NUM_LIGHTS };

    std::vector<std::unique_ptr<ChipSym::NESArbitraryWaveform>> narb;

    ChipYourWave() : Module() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(FREQ_KNOB, -54.0, 54.0, 0.0, "Frequency");
        for (int i = 0; i < 16; ++i) {
            configParam(WAVEFORM_START + (15-i), 0, 15.0, i );
            configParam(WAVEFORM_START + 16+i, 0, 15.0, i );
        }
        narb.resize(16);
        for( int i=0; i<16; ++i )
            narb[i].reset(nullptr);
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
        int nChan = inputs[FREQ_CV].getChannels();
        outputs[WAVE_OUTPUT].setChannels(nChan);
        for( int i=0; i<nChan; ++i )
        {
            if (narb[i] == nullptr) {
                narb[i].reset(
                    new ChipSym::NESArbitraryWaveform(-5.0, 5.0, args.sampleRate));
                narb[i]->setDigWavelength(2 << 8);
            }
            float dwf = digWFInSeconds(params[FREQ_KNOB].getValue(),
                                       12.0f * inputs[FREQ_CV].getVoltage(i));
            
            narb[i]->setWavelengthInSeconds(dwf);
            
            for (int j = 0; j < 32; ++j)
                narb[i]->setWaveformPoint(j, params[WAVEFORM_START + j].getValue());
            
            if (outputs[WAVE_OUTPUT].isConnected())
                outputs[WAVE_OUTPUT].setVoltage(narb[i]->step(), i);
        }
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

    bg->addLabel(Vec(bg->cx(), 135), "Click to draw your Digital Waveform", 14,
                 NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM);


    float x0 = (box.size.x - 10 * 32)/2.0;

    for (int i = 0; i < 32; ++i) {

        /*
        int swxc = x0 + 10 * i + 5;
        float swy = 122;

        if( i % 3 == 0 )
            swxc += 0;
        else if (i % 3 == 1)
        {
            swy = 98;
        }
        else
        {
            swxc += 0;
            swy = 73;
        }
        swxc -= 12;
        addInput(createInput<PJ301MPort>(Vec(swxc, swy), module, ChipYourWave::WAVE0_CV + i ) );
        */
        
        addParam(createParam<
                 NStepDraggableLEDWidget<16, RedGreenFromMiddleColorModel>>(
                     Vec(x0 + 10 * i, 155), module, ChipYourWave::WAVEFORM_START + i));
    }

    bg->addDrawFunction([x0](NVGcontext *vg) {
            nvgBeginPath(vg);
            nvgRect(vg, x0, 150, 320, 200);
            nvgFillColor(vg, componentlibrary::SCHEME_BLACK);
            nvgFill(vg);
        }
        );
}

Model *modelChipYourWave =
    createModel<ChipYourWave, ChipYourWaveWidget>("ChipYourWave");

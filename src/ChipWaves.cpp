
#include "BaconPlugs.hpp"
#include "ChipSym.hpp"

struct ChipWaves : virtual Module
{
    enum ParamIds
    {
        FREQ_KNOB,
        PULSE_CYCLE,
        NUM_PARAMS
    };

    enum InputIds
    {
        FREQ_CV,
        NUM_INPUTS
    };

    enum OutputIds
    {
        PULSE_OUTPUT,
        TRI_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        PULSE_CYCLE_LIGHT,
        NUM_LIGHTS
    };

    // For now just do this the obvious way for polyphony
    std::vector<std::unique_ptr<ChipSym::NESPulse>> npulse;
    std::vector<std::unique_ptr<ChipSym::NESTriangle>> ntri;

    ChipWaves() : Module()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(PULSE_CYCLE, 0, 3, 0, "NES Pulse Wave Duty Cycle");
        configParam(FREQ_KNOB, -54.0, 54.0, 0.0, "Frequency");

        npulse.resize(16);
        ntri.resize(16);
    }

    float digWFInSeconds(float pitchKnob, float pitchCV)
    {
        // This is the frequency tuning used in Fundamental/VCO so lets be
        // consistent
        float pitch = pitchKnob + pitchCV;
        float freq = 261.626f * powf(2.0f, pitch / 12.0f);
        // OK so now we have the frequency. We need the wavelength though.
        // Simple
        float wl = 1.0f / freq;

        return wl;
    }

    void process(const ProcessArgs &args) override
    {
        if (npulse[0] == nullptr || ntri[0] == nullptr)
        {
            for (int i = 0; i < 16; ++i)
            {
                npulse[i].reset(new ChipSym::NESPulse(-5.0, 5.0, args.sampleRate));
                ntri[i].reset(new ChipSym::NESTriangle(-5.0, 5.0, args.sampleRate));

                npulse[i]->setDigWavelength(2 << 9);
                ntri[i]->setDigWavelength(2 << 8);
            }
        }

        int chanct = std::max(1, inputs[FREQ_CV].getChannels());
        outputs[TRI_OUTPUT].setChannels(chanct);
        outputs[PULSE_OUTPUT].setChannels(chanct);

        for (int c = 0; c < chanct; ++c)
        {
            float dwf =
                digWFInSeconds(params[FREQ_KNOB].getValue(), 12.0f * inputs[FREQ_CV].getVoltage(c));

            ntri[c]->setWavelengthInSeconds(dwf);
            npulse[c]->setWavelengthInSeconds(dwf);

            int dc = clamp((int)(params[PULSE_CYCLE].getValue()), 0, 3);
            npulse[c]->setDutyCycle(dc);
            lights[PULSE_CYCLE_LIGHT].value = dc;

            if (outputs[TRI_OUTPUT].isConnected())
                outputs[TRI_OUTPUT].setVoltage(ntri[c]->step(), c);
            if (outputs[PULSE_OUTPUT].isConnected())
                outputs[PULSE_OUTPUT].setVoltage(npulse[c]->step(), c);
        }
    }
};

struct ChipWavesWidget : ModuleWidget
{
    ChipWavesWidget(ChipWaves *module);
};

ChipWavesWidget::ChipWavesWidget(ChipWaves *module) : ModuleWidget()
{
    setModule(module);
    box.size = Vec(SCREW_WIDTH * 6, RACK_HEIGHT);

    BaconBackground *bg = new BaconBackground(box.size, "ChipWaves");
    addChild(bg->wrappedInFramebuffer());

    Vec outP = Vec(bg->cx(24) + 22, RACK_HEIGHT - 15 - 43);
    bg->addPlugLabel(outP, BaconBackground::SIG_OUT, "pulse");
    addOutput(createOutput<PJ301MPort>(outP, module, ChipWaves::PULSE_OUTPUT));

    Vec outT = Vec(bg->cx(24) - 22, RACK_HEIGHT - 15 - 43);
    bg->addPlugLabel(outT, BaconBackground::SIG_OUT, "tri");
    addOutput(createOutput<PJ301MPort>(outT, module, ChipWaves::TRI_OUTPUT));

    Vec fcv = Vec(bg->cx(24), 140);
    bg->addPlugLabel(fcv, BaconBackground::SIG_IN, "v/o");
    addInput(createInput<PJ301MPort>(fcv, module, ChipWaves::FREQ_CV));

    float x0 = bg->cx(63);
    bg->addRoundedBorder(Vec(x0, 200), Vec(63, 49));
    bg->addLabel(Vec(box.size.x / 2, 204), "Duty Cycle", 12, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
    int ybot = 200 + 24 + 5 + 20;
    addParam(createParam<RoundSmallBlackKnob>(Vec(x0 + 6, ybot - 3 - 28), module,
                                              ChipWaves::PULSE_CYCLE));
    addChild(createLight<SevenSegmentLight<BlueLight, 2>>(Vec(x0 + 42, ybot - 5 - 24), module,
                                                          ChipWaves::PULSE_CYCLE_LIGHT));

    bg->addLabel(Vec(bg->cx(), 45), "Freq", 14, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM);
    addParam(createParam<RoundHugeBlackKnob>(Vec(bg->cx(56), 50), module, ChipWaves::FREQ_KNOB));
}

Model *modelChipWaves = createModel<ChipWaves, ChipWavesWidget>("ChipWaves");

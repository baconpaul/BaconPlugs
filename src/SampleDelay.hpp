
#include <algorithm>
#include <vector>

template <typename TBase> struct SampleDelay : virtual TBase
{
    enum ParamIds
    {
        DELAY_KNOB,
        NUM_PARAMS
    };

    enum InputIds
    {
        SIGNAL_IN,
        NUM_INPUTS
    };

    enum OutputIds
    {
        SIGNAL_OUT,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        DELAY_VALUE_LIGHT,
        NUM_LIGHTS
    };

    using TBase::inputs;
    using TBase::lights;
    using TBase::outputs;
    using TBase::params;

    std::vector<float> ring[16];
    size_t ringSize;
    size_t pos[16];

    SampleDelay() : TBase()
    {
        TBase::config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        TBase::configParam(DELAY_KNOB, 1, 99, 1, "Samples to delay");
        TBase::configInput(SIGNAL_IN, "Input Signal");
        TBase::configOutput(SIGNAL_OUT, "Output (Input delayed by N samples)");
        ringSize = 100;
        for (int i = 0; i < 16; ++i)
        {
            ring[i].resize(ringSize);
            pos[i] = 0;
            std::fill(ring[i].begin(), ring[i].end(), 0);
        }

        TBase::configBypass(SIGNAL_IN, SIGNAL_OUT);
    }

    void process(const typename TBase::ProcessArgs &args) override
    {
        int del = params[DELAY_KNOB].getValue() - 1;
        int nChan = inputs[SIGNAL_IN].getChannels();
        outputs[SIGNAL_OUT].setChannels(nChan);
        lights[DELAY_VALUE_LIGHT].value = del + 1;

        for (int i = 0; i < nChan; ++i)
        {
            int dpos = ((int)pos[i] - del);
            if (dpos < 0)
                dpos += ringSize;

            ring[i][pos[i]] = inputs[SIGNAL_IN].getVoltage(i);
            outputs[SIGNAL_OUT].setVoltage(ring[i][dpos], i);

            pos[i]++;
            if (pos[i] >= ringSize)
                pos[i] = 0;
        }
    }
};

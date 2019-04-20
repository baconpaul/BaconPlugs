
#include <algorithm>
#include <vector>

template <typename TBase> struct SampleDelay : virtual TBase {
    enum ParamIds { DELAY_KNOB, NUM_PARAMS };

    enum InputIds { SIGNAL_IN, NUM_INPUTS };

    enum OutputIds { SIGNAL_OUT, NUM_OUTPUTS };

    enum LightIds { DELAY_VALUE_LIGHT, NUM_LIGHTS };

    using TBase::inputs;
    using TBase::lights;
    using TBase::outputs;
    using TBase::params;

    std::vector<float> ring;
    size_t ringSize;
    size_t pos;

    SampleDelay() : TBase() {
        TBase::config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        TBase::configParam(DELAY_KNOB, 1, 99, 1, "Samples to delay");
        ringSize = 100;
        ring.resize(ringSize);
        std::fill(ring.begin(), ring.end(), 0);
        pos = 0;
    }

    void process(const typename TBase::ProcessArgs &args) override {
        int del = params[DELAY_KNOB].getValue() - 1;
        int dpos = ((int)pos - del);
        if (dpos < 0)
            dpos += ringSize;

        ring[pos] = inputs[SIGNAL_IN].getVoltage();
        outputs[SIGNAL_OUT].setVoltage(ring[dpos]);
        lights[DELAY_VALUE_LIGHT].value = del + 1;

        pos++;
        if (pos >= ringSize)
            pos = 0;
    }
};

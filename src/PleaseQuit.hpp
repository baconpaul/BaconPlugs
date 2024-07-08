
#include <algorithm>
#include <vector>
#include "rack.hpp"

template <typename TBase> struct PleaseQuit : virtual TBase
{
    enum ParamIds
    {
        NUM_PARAMS
    };

    enum InputIds
    {
        PLEASE_QUIT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        NUM_OUTPUTS
    };

    enum LightIds
    {
        NUM_LIGHTS
    };

    using TBase::inputs;
    using TBase::lights;
    using TBase::outputs;
    using TBase::params;

    std::vector<float> ring[16];
    size_t ringSize;
    size_t pos[16];

    PleaseQuit() : TBase()
    {
        TBase::config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        TBase::configInput(PLEASE_QUIT, "Trigger Me to Quit");
    }

    rack::dsp::SchmittTrigger inTrig;
    uint64_t samplesGoneBy{0};
    void process(const typename TBase::ProcessArgs &args) override
    {
        if (samplesGoneBy < 12000)
        {
            samplesGoneBy ++;
        }
        else
        {
            if (inTrig.process(inputs[PLEASE_QUIT].getVoltageSum()))
            {
                APP->window->close();
            }
        }
    }
};

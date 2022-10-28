#include <algorithm>

template <typename TBase> struct Glissinator : public TBase
{
    enum ParamIds
    {
        GLISS_TIME,

        NUM_PARAMS
    };

    enum InputIds
    {
        SOURCE_INPUT,
        GLISS_CV_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        SLID_OUTPUT,
        GLISSING_GATE,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        SLIDING_LIGHT,
        NUM_LIGHTS
    };

    float priorIn[16];
    float targetIn[16];
    int offsetCount[16];

    // Hey thanks https://stackoverflow.com/a/4643091
    using TBase::inputs;
    using TBase::lights;
    using TBase::outputs;
    using TBase::params;

    Glissinator() : TBase()
    {
        TBase::config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        TBase::configParam(GLISS_TIME, 0, 1, 0.1, "Time to gliss, in seconds");
        TBase::configInput(SOURCE_INPUT, "Input to Slew/Gliss");
        TBase::configInput(GLISS_CV_INPUT, "Modify Input Time: +/-5v for full range");
        TBase::configOutput(SLID_OUTPUT, "Slewed/Glissed Output");
        TBase::configOutput(GLISSING_GATE, "Gate at end of Gliss");
        for (int i = 0; i < 16; ++i)
            offsetCount[i] = -1;
    }

    void process(const typename TBase::ProcessArgs &args) override
    {
        float glist_sec = params[GLISS_TIME].getValue();


        int nChan = inputs[SOURCE_INPUT].getChannels();
        outputs[SLID_OUTPUT].setChannels(nChan);
        outputs[GLISSING_GATE].setChannels(nChan);
        lights[SLIDING_LIGHT].value = 0;

        for (int i = 0; i < nChan; ++i)
        {
            auto cglist = std::clamp(glist_sec + inputs[GLISS_CV_INPUT].getVoltage(i) * 0.2f, 0.f, 1.f);
            int shift_time = args.sampleRate * cglist;
            if (shift_time < 10)
                shift_time = 10;

            float thisIn = inputs[SOURCE_INPUT].getVoltage(i);

            // This means I am being intialized
            if (offsetCount[i] < 0)
            {
                priorIn[i] = thisIn;
                offsetCount[i] = 0;
            }

            bool inGliss = offsetCount[i] != 0;
            float thisOut = thisIn;

            // When I begin the cycle, the shift_time may be a different shift_time
            // than the prior cycle. This is not a problem unless the shift time is
            // now shorter than the offset_time. If that's the case we have
            // basically finished the gliss. This check used to be at the end of the
            // loop but that lead to one bad value even with the >=
            if (offsetCount[i] >= shift_time)
            {
                offsetCount[i] = 0;
                priorIn[i] = thisIn;
                targetIn[i] = thisIn;
                inGliss = false;
            }

            // I am not glissing
            if (!inGliss)
            {
                // But I have a new target, so start glissing by setting offset
                // count to 1.
                if (thisIn != priorIn[i])
                {
                    targetIn[i] = thisIn;
                    offsetCount[i] = 1;
                    inGliss = true;
                }
            }

            // I am glissing (note this is NOT in an else since inGliss can be reset
            // above)
            if (inGliss)
            {
                // OK this means my note has changed underneath me so I have to
                // simulate my starting point.
                if (thisIn != targetIn[i])
                {
                    // This "-1" is here because we want to know the LAST known step
                    // - so at the prior offset count. Without this a turnaround
                    // will tick above the turnaround point for one sample.
                    float lastKnown = ((shift_time - (offsetCount[i] - 1)) * priorIn[i] +
                                       (offsetCount[i] - 1) * targetIn[i]) /
                                      shift_time;
                    targetIn[i] = thisIn;
                    priorIn[i] = lastKnown;
                    offsetCount[i] = 0;
                }

                // Then the output is just the weighted sum of the prior input and
                // this input.
                thisOut = ((shift_time - offsetCount[i]) * priorIn[i] + offsetCount[i] * thisIn) /
                          shift_time;

                // and step along one.
                offsetCount[i]++;
            }

            lights[SLIDING_LIGHT].value += inGliss ? 1.0 / nChan : 0;
            outputs[SLID_OUTPUT].setVoltage(thisOut, i);
            outputs[GLISSING_GATE].setVoltage(inGliss ? 10 : 0, i);
        }
    }
};

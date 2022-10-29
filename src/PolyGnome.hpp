#include <math.h>
#include "rack.hpp"
#define NUM_CLOCKS 4

template <typename TBase> struct PolyGnome : virtual TBase
{
    enum ParamIds
    {
        CLOCK_PARAM,
        CLOCK_NUMERATOR_1,
        CLOCK_DENOMINATOR_1 = CLOCK_NUMERATOR_1 + NUM_CLOCKS,
        NUM_PARAMS = CLOCK_DENOMINATOR_1 + NUM_CLOCKS,
    };

    enum InputIds
    {
        CLOCK_INPUT,
        NUM_INPUTS,
    };

    enum OutputIds
    {
        CLOCK_GATE_0,

        CLOCK_CV_LEVEL_0 = CLOCK_GATE_0 + NUM_CLOCKS +
                           1, // the "1" is for the 1/4 note clock which isn't parameterized
        NUM_OUTPUTS = CLOCK_CV_LEVEL_0 + NUM_CLOCKS + 1
    };

    enum LightIds
    {
        LIGHT_NUMERATOR_1,
        LIGHT_DENOMINATOR_1 = LIGHT_NUMERATOR_1 + NUM_CLOCKS,
        BPM_LIGHT = LIGHT_DENOMINATOR_1 + NUM_CLOCKS,
        NUM_LIGHTS
    };

    using TBase::inputs;
    using TBase::lights;
    using TBase::outputs;
    using TBase::params;

    float phase;
    long phase_longpart;

    PolyGnome() : TBase()
    {
        TBase::config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        phase = 0.0f;
        phase_longpart = 274;

        TBase::configParam(CLOCK_PARAM, -2.0, 2.0, 0.0, "Clock tempo", " bpm", 2.f, 60.f);
        for (int i = 0; i < NUM_CLOCKS; ++i)
        {
            TBase::configParam(CLOCK_NUMERATOR_1 + i, 1, 30, 1);
            TBase::configParam(CLOCK_DENOMINATOR_1 + i, 1, 16, 1);
        }
    }

    inline int numi(int i) { return (int)params[CLOCK_NUMERATOR_1 + i].getValue(); }
    inline int deni(int i) { return (int)params[CLOCK_DENOMINATOR_1 + i].getValue(); }
    void process(const typename TBase::ProcessArgs &args) override
    {
        float clockCV = params[CLOCK_PARAM].getValue();
        float clockTime = 2 * powf(2.0f, clockCV);
        outputs[CLOCK_CV_LEVEL_0].setVoltage(clockCV);

        float dPhase = clockTime * args.sampleTime;
        float samplesPerBeat = 1.0 / dPhase;
        float secondsPerBeat = samplesPerBeat / args.sampleRate;
        float beatsPerMinute = 60.0 / secondsPerBeat;
        lights[BPM_LIGHT].value = beatsPerMinute;

        for (int i = 0; i < NUM_CLOCKS; ++i)
        {
            float cbpm = beatsPerMinute * deni(i) / numi(i);
            // bpm = 60 * 2^cv so cv = log2(bpm/60)
            outputs[CLOCK_CV_LEVEL_0 + i + 1].setVoltage(log2f(cbpm / 60.0));
        }

        phase += clockTime * args.sampleTime;

        while (phase > 1)
        {
            phase = phase - 1;
            phase_longpart++;
        }

        /* Alright we have to stop that longpart from getting too big otherwise
           it will swamp the fractional parts but we have to reset it when all
           the clocks are firing at once otherwise one of the clocks will
           stutter. So figure out the product of my fractions. Probably we
           should use the common prime factors so we can get an earlier step but
           lets leave it for now.
        */
        int sd = 1;
        int sn = 1;
        for (int i = 0; i < NUM_CLOCKS; ++i)
        {
            if (outputs[CLOCK_GATE_0 + i + 1].isConnected())
            {
                sd *= deni(i);
                sn *= numi(i);
            }
        }
        int commonp = sd * sn; // so we know at least that the clocks will
                               // intersect at this tick.

        while (phase_longpart > commonp)
        {
            phase_longpart -= commonp;
        }

        for (int i = 0; i < NUM_CLOCKS + 1; ++i)
        {
            bool gateIn = false;
            float frac;
            if (i == 0)
                frac = 1;
            else
                frac = deni(i - 1) / (1.0f * numi(i - 1));

            // Note that we have two parts which comprise the phase number now,
            // the float and the long. The addition can overflow, though which
            // is why I mod the phase_longpart with a larger number
            float lphase = phase * frac;
            double liphase = phase_longpart * frac;

            double ipart;
            // I still worry a bit this + may overflow if you let this run long
            // enough and blow out the precision in the decimal
            float fractPhase = modf(lphase + liphase, &ipart);
            gateIn = (fractPhase < 0.5f);

            outputs[CLOCK_GATE_0 + i].setVoltage(gateIn ? 10.0f : 0.0f);
        }

        for (int i = 0; i < NUM_CLOCKS; ++i)
        {
            lights[LIGHT_NUMERATOR_1 + i].value = (int)params[CLOCK_NUMERATOR_1 + i].getValue();
            lights[LIGHT_DENOMINATOR_1 + i].value = (int)params[CLOCK_DENOMINATOR_1 + i].getValue();
        }
    }
};

#include <math.h>
#include "rack.hpp"
#define NUM_CLOCKS 5

template <typename TBase> struct PolyGnome : virtual TBase
{
    enum ParamIds
    {
        CLOCK_PARAM,
        CLOCK_DENOMINATOR_0,
        CLOCK_NUMERATOR_0 = CLOCK_DENOMINATOR_0 + NUM_CLOCKS,
        CLOCK_PULSE_WIDTH_0 = CLOCK_NUMERATOR_0 + NUM_CLOCKS,
        RUN_PARAM = CLOCK_PULSE_WIDTH_0 + NUM_CLOCKS,
        RESET_PARAM,
        NUM_PARAMS
    };

    enum InputIds
    {
        CLOCK_INPUT,
        RUN_INPUT,
        RESET_INPUT,
        NUM_INPUTS,
    };

    enum OutputIds
    {
        CLOCK_GATE_0,

        CLOCK_CV_LEVEL_0 = CLOCK_GATE_0 + NUM_CLOCKS +
                           1, // the "1" is for the 1/4 note clock which isn't parameterized
        RUN_OUTPUT = CLOCK_CV_LEVEL_0 + NUM_CLOCKS + 1,
        RESET_OUTPUT,
        BPM_CV_OUTPUT,

        NUM_OUTPUTS
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
            TBase::configParam(CLOCK_DENOMINATOR_0 + i, 1, 30, 1, "Denominator " + std::to_string(i+1));
            TBase::configParam(CLOCK_NUMERATOR_0 + i, 1, 16, 1, "Numerator " + std::to_string(i+1) );
            TBase::configParam(CLOCK_PULSE_WIDTH_0 + i, 0, 1, 0.5, "Pulse Width " + std::to_string(i+1));
        }
        TBase::configParam(RUN_PARAM, 0, 1, 1, "Run");
    }

    inline int deni(int i) { return (int)params[CLOCK_DENOMINATOR_0 + i].getValue(); }
    inline int numi(int i) { return (int)params[CLOCK_NUMERATOR_0 + i].getValue(); }
    bool wasRunning{false};
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

        bool useGates = params[RUN_PARAM].getValue() > 0.5;
        bool doReset = params[RESET_PARAM].getValue() > 0.5;
        if (useGates && ! wasRunning)
        {
            wasRunning = true;
            doReset = true;
        }

        for (int i = 0; i < NUM_CLOCKS; ++i)
        {
            float cbpm = beatsPerMinute * numi(i) / deni(i);
            // bpm = 60 * 2^cv so cv = log2(bpm/60)
            outputs[CLOCK_CV_LEVEL_0 + i + 1].setVoltage(log2f(cbpm / 120.0));
        }

        if (useGates)
            phase += clockTime * args.sampleTime;

        while (phase > 1)
        {
            phase = phase - 1;
            phase_longpart++;
        }

        if (doReset)
        {
            phase = 0;
            phase_longpart = 274;
            useGates = false;
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
                sd *= numi(i);
                sn *= deni(i);
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
            auto pw = 0.5;
            if (i == 0)
                frac = 1;
            else
            {
                pw = std::clamp(params[CLOCK_PULSE_WIDTH_0 + i - 1].getValue(), 0.001f, 0.999f);
                frac = numi(i - 1) / (1.0f * deni(i - 1));
            }
            // Note that we have two parts which comprise the phase number now,
            // the float and the long. The addition can overflow, though which
            // is why I mod the phase_longpart with a larger number
            float lphase = phase * frac;
            double liphase = phase_longpart * frac;

            double ipart;
            // I still worry a bit this + may overflow if you let this run long
            // enough and blow out the precision in the decimal
            float fractPhase = modf(lphase + liphase, &ipart);
            gateIn = (fractPhase < pw);

            outputs[CLOCK_GATE_0 + i].setVoltage(gateIn && useGates ? 10.0f : 0.0f);
        }

        for (int i = 0; i < NUM_CLOCKS; ++i)
        {
            lights[LIGHT_NUMERATOR_1 + i].value = (int)params[CLOCK_DENOMINATOR_0 + i].getValue();
            lights[LIGHT_DENOMINATOR_1 + i].value = (int)params[CLOCK_NUMERATOR_0 + i].getValue();
        }
    }
};

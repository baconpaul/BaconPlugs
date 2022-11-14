#include <math.h>
#include "rack.hpp"
#include <iostream>
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
        SELF_RESET_EVERY,
        NUM_PARAMS
    };

    enum InputIds
    {
        RUN_INPUT,
        RESET_INPUT,
        BPM_INPUT,
        NUM_INPUTS,
    };

    enum OutputIds
    {
        CLOCK_GATE_0,

        CLOCK_CV_LEVEL_0 = CLOCK_GATE_0 + NUM_CLOCKS +
                           1, // the "1" is for the 1/4 note clock which isn't parameterized
        RUN_OUTPUT = CLOCK_CV_LEVEL_0 + NUM_CLOCKS + 1,
        RESET_OUTPUT,

        NUM_OUTPUTS
    };

    enum LightIds
    {
        LIGHT_NUMERATOR_1,
        LIGHT_DENOMINATOR_1 = LIGHT_NUMERATOR_1 + NUM_CLOCKS,
        BPM_LIGHT = LIGHT_DENOMINATOR_1 + NUM_CLOCKS,
        RUNNING_LIGHT,
        SELF_RESET_LIGHT,
        NUM_LIGHTS
    };

    using TBase::inputs;
    using TBase::lights;
    using TBase::outputs;
    using TBase::params;

    float phase;
    long phase_longpart, since_reset{0};

    PolyGnome() : TBase()
    {
        TBase::config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        phase = 0.0f;
        phase_longpart = 274;

        TBase::configParam(CLOCK_PARAM, -2.0, 2.0, 0.0, "Clock tempo", " bpm", 2.f, 120.f);
        for (int i = 0; i < NUM_CLOCKS; ++i)
        {
            auto d = TBase::configParam(CLOCK_DENOMINATOR_0 + i, 1, 32, 1, "Denominator " + std::to_string(i+1));
            d->snapEnabled = true;
            auto n = TBase::configParam(CLOCK_NUMERATOR_0 + i, 1, 32, 1, "Numerator " + std::to_string(i+1) );
            n->snapEnabled = true;
            TBase::configParam(CLOCK_PULSE_WIDTH_0 + i, 0, 1, 0.5, "Pulse Width " + std::to_string(i+1));
        }
        for (int i=0; i<NUM_CLOCKS + 1; ++i)
            priorGates[i] = false;
        TBase::configSwitch(RUN_PARAM, 0, 1, 1, "Run", { "Stop", "Run" });
        TBase::configSwitch(RESET_PARAM, 0, 1, 1, "Reset", { "Off", "Reset" });
        auto sre = TBase::configParam(SELF_RESET_EVERY, 0, 64, 0, "Self Reset Every" );

        TBase::configInput(RUN_INPUT, "Run");
        TBase::configInput(RESET_INPUT, "Reset");
        TBase::configInput(BPM_INPUT, "BPM CV");

        TBase::configOutput(RUN_OUTPUT, "Run");
        TBase::configOutput(RESET_OUTPUT, "Reset");
        TBase::configOutput(CLOCK_GATE_0, "Base Clock Gate");
        TBase::configOutput(CLOCK_CV_LEVEL_0, "Base BPM CV");

        TBase::configBypass(RUN_INPUT, RUN_OUTPUT);
        TBase::configBypass(RESET_INPUT, RESET_OUTPUT);
        TBase::configBypass(BPM_INPUT, CLOCK_CV_LEVEL_0);
        for (int i=0; i<NUM_CLOCKS; ++i)
        {
            TBase::configOutput(CLOCK_GATE_0 + i + 1, "Clock " + std::to_string(i+1) + " Gate");
            TBase::configOutput(CLOCK_CV_LEVEL_0 + i + 1, "BPM " + std::to_string(i+1) + " CV");
        }
        sre->snapEnabled = true;
    }

    inline int deni(int i) { return (int)params[CLOCK_DENOMINATOR_0 + i].getValue(); }
    inline int numi(int i) { return (int)params[CLOCK_NUMERATOR_0 + i].getValue(); }
    bool wasRunning{false}, wasResetting{false};

    rack::dsp::SchmittTrigger runTrigger, resetTrigger;
    int32_t resetTriggerOut{0}, runTriggerOut{0};
    float priorClockCV{-1000}, clockTime{0};
    bool priorGates[NUM_CLOCKS + 1];
    void process(const typename TBase::ProcessArgs &args) override
    {
        float clockCV = params[CLOCK_PARAM].getValue();
        if (inputs[BPM_INPUT].isConnected())
            clockCV = inputs[BPM_INPUT].getVoltage();

        if (clockCV != priorClockCV)
        {
            clockTime = 2 * powf(2.0f, clockCV);
            priorClockCV = clockCV;
        }
        outputs[CLOCK_CV_LEVEL_0].setVoltage(clockCV);

        float dPhase = clockTime * args.sampleTime;
        float samplesPerBeat = 1.0 / dPhase;
        float secondsPerBeat = samplesPerBeat / args.sampleRate;
        float beatsPerMinute = 60.0 / secondsPerBeat;

        int selfResetEvery = (int)std::round(params[SELF_RESET_EVERY].getValue());
        lights[BPM_LIGHT].value = beatsPerMinute;
        lights[SELF_RESET_LIGHT].value = selfResetEvery;

        bool useGates = params[RUN_PARAM].getValue() > 0.5;
        if (inputs[RUN_INPUT].isConnected())
        {
            auto tog = runTrigger.process(inputs[RUN_INPUT].getVoltage());
            if (tog)
            {
                useGates = !useGates;
                params[RUN_PARAM].setValue(useGates ? 1 : 0);
            }
        }

        bool doReset = params[RESET_PARAM].getValue() > 0.5;

        if (inputs[RESET_INPUT].isConnected())
        {
            auto tog = resetTrigger.process(inputs[RESET_INPUT].getVoltage());
            if (tog)
            {
                doReset = true;
            }
        }

        if (useGates && ! wasRunning)
        {
            wasRunning = true;
            doReset = true;
            runTriggerOut = 96;
        }
        else if (!useGates && wasRunning)
        {
            runTriggerOut = 96;
            wasRunning = false;
        }

        if (selfResetEvery > 0 && selfResetEvery == since_reset)
        {
            doReset = true;
        }

        if (doReset && !wasResetting)
        {
            wasResetting = true;
            resetTriggerOut = 96;
        }
        else if (doReset)
        {
            doReset = false;
        }
        else
        {
            wasResetting = false;
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
            since_reset = 0;
            useGates = false;
        }

        lights[RUNNING_LIGHT].value = useGates ? 10 : 0;

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

            if (gateIn && !priorGates[i] && i == 0)
            {
                since_reset ++;
            }

            priorGates[i] = gateIn;

            outputs[CLOCK_GATE_0 + i].setVoltage(gateIn && useGates ? 10.0f : 0.0f);
        }

        for (int i = 0; i < NUM_CLOCKS; ++i)
        {
            lights[LIGHT_NUMERATOR_1 + i].value = (int)params[CLOCK_DENOMINATOR_0 + i].getValue();
            lights[LIGHT_DENOMINATOR_1 + i].value = (int)params[CLOCK_NUMERATOR_0 + i].getValue();
        }

        outputs[RUN_OUTPUT].setVoltage(runTriggerOut > 0 ? 10 : 0);
        if (runTriggerOut > 0)
        {
            runTriggerOut--;
        }
        outputs[RESET_OUTPUT].setVoltage(resetTriggerOut > 0 ? 10 : 0);
        if (resetTriggerOut > 0)
        {
            resetTriggerOut--;
        }
    }
};

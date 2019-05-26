#pragma once

#include "rack.hpp"

struct PolyGenerator : public rack::Module {
    enum ParamIds {
        BPM_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        NUM_INPUTS
    };
    enum OutputIds {
        TONE_CV,
        VEL_CV,
        GATE_CV,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    float bpm = 120.0;
    float dPhase = 0;
    float phase = 0;
    
    PolyGenerator() : rack::Module() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        for( int i=0; i<16; ++i )
            notes[i].on = false;
    }

    struct pnote
    {
        float tone;
        float vel;
        float len;
        bool on;
    };

    pnote notes[16];
        
    
    void process( const ProcessArgs &args ) override {
        if( dPhase == 0 )
        {
            float samplesPerSecond = args.sampleRate;
            float samplesPerMinute = samplesPerSecond * 60;
            float samplesPerBeat = samplesPerMinute / bpm;
            dPhase = 1.0 / samplesPerBeat;

            outputs[TONE_CV].setChannels(16);
            outputs[VEL_CV].setChannels(16);
            outputs[GATE_CV].setChannels(16);
        }
        phase += dPhase;
        if( phase > 1.0 )
        {
            phase -= 1.0;
            for( int i=0; i<16; ++i )
            {
                if( ! notes[i].on && rand() * 1.0 / RAND_MAX > 0.6 )
                {
                    notes[i].on = true;
                    notes[i].vel = rand() % 70 + 40;
                    notes[i].tone = rand() % 60 / 12.0 + 2.0;
                    notes[i].len = rand() % 100 / 30.0;
                }
            }
        }
        for( int i=0; i<16; ++i )
        {
            if( notes[i].on )
            {
                notes[i].len -= dPhase;
                outputs[TONE_CV].setVoltage(notes[i].tone,i);
                outputs[VEL_CV].setVoltage(notes[i].vel,i);
                outputs[GATE_CV].setVoltage(10,i);

                if( notes[i].len < 0 )
                    notes[i].on = false;
            }
            else
            {
                outputs[TONE_CV].setVoltage(4,i);
                outputs[VEL_CV].setVoltage(0,i);
                outputs[GATE_CV].setVoltage(0,i);
            }
        }
    }
};


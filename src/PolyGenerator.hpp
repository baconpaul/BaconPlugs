#pragma once

#include "rack.hpp"

struct PolyGenerator : public rack::Module {
    enum ParamIds {
        BPM_PARAM,
        VOICES_PARAM,
        PATTERN_PARAM,
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

        configParam(BPM_PARAM, -2, 3, 1, "BPM" );
        configParam(VOICES_PARAM, 1, 16, 16, "Voice Count" );
        configParam(PATTERN_PARAM, 0, 4, 0, "Pattern" );
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

    std::vector<int> major = { 0, 2, 4, 5, 7, 9, 11, 12 };
    std::vector<int> arp = { 0, 4, 7, 2, 7, 12, 5 };
    std::vector<int> directions;
    std::vector<float> snotes;
    
    float fifthPos = 0;
    
    void process( const ProcessArgs &args ) override {
        if( dPhase == 0 )
        {
            outputs[TONE_CV].setChannels(16);
            outputs[VEL_CV].setChannels(16);
            outputs[GATE_CV].setChannels(16);

            directions.resize(16);
            snotes.resize(16);
            for( int i=0; i<16; ++i ) {
                directions[i] = rand() % 5 - 2;
                snotes[i] = -1.0 + rand() % 24 / 12.0;
            }
        }

        bpm = 60.0 * pow( 2.0, params[BPM_PARAM].getValue() );
        float samplesPerSecond = args.sampleRate;
        float samplesPerMinute = samplesPerSecond * 60;
        float samplesPerBeat = samplesPerMinute / bpm;
        dPhase = 1.0 / samplesPerBeat;

        phase += dPhase;
        int pattern = (int)params[PATTERN_PARAM].getValue();
        int voices = (int)(params[VOICES_PARAM].getValue());
        switch( pattern )
        {
        case 1:
        {
            int pos = std::max(0, std::min((int)(phase * 16), 15));
            if( pos < voices && ! notes[pos].on )
            {
                int n = pos % arp.size();
                int o = pos / arp.size();
                notes[pos].on = true;
                notes[pos].vel = 8;
                notes[pos].len = 0.3;
                notes[pos].tone = arp[n] / 12.0 + o;
            }
            break;
        }
        case 2:
        {
            if( phase > 1.0 )
            {
                std::vector<int> triad = { 0, 4, 7, 10 };
                notes[0].on = true;
                notes[0].vel = 8;
                notes[0].len = 0.8;
                notes[0].tone = fifthPos - 2;
                for( int i=1; i<voices; ++i )
                {
                    int p = i-1;
                    int n = p % triad.size();
                    int o = p / triad.size();
                    notes[i].on = true;
                    notes[i].vel = 8;
                    notes[i].len = 0.8;
                    notes[i].tone = fifthPos + triad[n]/12.0 + o;
                }
                fifthPos += 7/12.0;
                if( fifthPos > 1) fifthPos -= 1;
            }
            break;
        }

        case 3:
        {
            if( phase > 1.0 )
            {
                for( int i=0; i<voices; ++i )
                {
                    if( ! notes[i].on )
                    {
                        snotes[i] += directions[i] * 1.0 / 12.0;
                        notes[i].on = true;
                        notes[i].vel = 8;
                        notes[i].len = 0.8 + rand() % 2;
                        notes[i].tone = snotes[i];
                        
                        if( rand() % 100 > 60 )
                        {
                            directions[i] = -2 + rand() % 5;
                        }
                        if( snotes[i] > 2.5 || snotes[ i ] < -2.5 )
                            snotes[i] = 0;
                    }
                }
            }
            break;
        }
        
        case 0:
        default:
            if( phase > 1.0 )
            {
                for( int i=0; i<voices; ++i )
                {
                    if( ! notes[i].on && rand() * 1.0 / RAND_MAX > 0.7 )
                    {
                        notes[i].on = true;
                        auto idx = rand() % major.size();
                        auto noff = major[idx];
                        auto oct = rand() % 5 - 2;
                        notes[i].vel = rand() % 600 / 60.0 + 3.0;
                        notes[i].tone = noff / 12.0 + oct;
                        notes[i].len = rand() % 100 / 30.0;
                    }
                }
                break;
            }
        }

        if( phase > 1.0 )
            phase -= 1.0;
        
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
                outputs[TONE_CV].setVoltage(0,i);
                outputs[VEL_CV].setVoltage(0,i);
                outputs[GATE_CV].setVoltage(0,i);
            }
        }
    }
};


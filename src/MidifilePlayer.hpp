#pragma once
#include "BaconPlugs.hpp"
#include "rack.hpp"
#include "MidiFile.h"

struct MidifilePlayer : public rack::Module {
    enum ParamIds {
        BPM_PARAM,
        GO_PARAM,
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

    
    MidifilePlayer() : rack::Module() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam(BPM_PARAM, -2, 3, 1, "BPM" );
        configParam(GO_PARAM, 0, 1, 0, "GO" );
        for( int i=0; i<16; ++i )
            notes[i].on = false;
    }

    struct pnote
    {
        float tone;
        float vel;
        float len;
        int key;
        bool on;
    };

    pnote notes[16];
    smf::MidiFile mf;
    bool init = false;
    float currentTime = 0;
    int currentEvent = 0;
    int nextVoice = 0;
    
    void process( const ProcessArgs &args ) override {
        if( ! init )
        {
            init = true;
            rack::INFO( "Loading midi file" );
            mf.read(rack::asset::plugin(pluginInstance, "res/midi/998-v05.mid").c_str());
            mf.doTimeAnalysis();
            mf.linkNotePairs();
            mf.joinTracks();
            currentEvent = 0;
            currentTime = -1;
        }

        // Loop forever
        if( currentEvent >= mf[0].size())
        {
            currentTime = -1.0;
            currentEvent = 0.0;
            for( int i=0; i<16; ++i )
                notes[i].on = false;
        }
        
        currentTime += args.sampleTime;
        while( currentEvent < mf[0].size() &&  mf[0][currentEvent].seconds < currentTime )
        {
            smf::MidiEvent &evt = mf[0][currentEvent];
            if (evt.isNoteOn())
            {
                int clv = nextVoice;
                while( notes[nextVoice].on && nextVoice != clv - 1 )
                {
                    nextVoice++;
                    if( nextVoice == 16 )
                        nextVoice = 0;
                }
                
                notes[nextVoice].on = true;
                notes[nextVoice].vel = evt.getVelocity() / 127.0 * 10;
                notes[nextVoice].tone = (evt.getKeyNumber() - 60) / 12.0 + 0.0;
                notes[nextVoice].key = evt.getKeyNumber();
                nextVoice ++;
                if( nextVoice == 16 )
                    nextVoice =0;
            }
            if (evt.isNoteOff())
            {
                for( int i=0; i<16; ++i )
                    if( notes[i].key == evt.getKeyNumber() )
                        notes[i].on = false;
            }
            currentEvent ++;
        }
        
        outputs[TONE_CV].setChannels(16);
        outputs[VEL_CV].setChannels(16);
        outputs[GATE_CV].setChannels(16);
        
        for( int i=0; i<16; ++i )
        {
            outputs[TONE_CV].setVoltage(notes[i].tone,i);
            outputs[VEL_CV].setVoltage(notes[i].vel,i);
            outputs[GATE_CV].setVoltage(notes[i].on ? 10 : 0 ,i);
        }
    }
};


#pragma once

#include "rack.hpp"
#include "BaconPlugs.hpp"
#include "MidiFile.h"


struct PPlayer {
    struct pnote
    {
        pnote() : tone(1.0), vel(127), len(0), key(60), on(false) {}
        float tone;
        float vel;
        float len;
        int key;
        bool on;
    };

    int noteCount = 16;
    pnote notes[16];
    int pattern = -1;

    virtual ~PPlayer() {
    }
    virtual void step(int voiceCount, float sampleTime, float phase, float dPhase) { }
    virtual void copyNotes( PPlayer *other ) {
        for( int i=0; i<16; ++i )
            notes[i] = other->notes[i];
    }

    virtual std::string getName() = 0;
    virtual int minVoices(int userChoice) { return std::max(1, userChoice); }
    
    void shortenNotes(float dPhase) {
        for( int i=0; i<noteCount; ++i )
        {
            if( notes[i].on )
                notes[i].len -= dPhase;
        }
    }
    
};

struct RandomTunedPlayer : public PPlayer
{
    std::vector<int> major = { 0, 2, 4, 5, 7, 9, 11, 12 };

    virtual std::string getName() { return "Random Tuned"; }
    
    virtual void step(int voiceCount, float sampleTime, float phase, float dPhase) {
        if( phase >= 1 )
        {
            for( int i=0; i<voiceCount; ++i )
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
        }
        shortenNotes(dPhase);
    }
};

struct ArpPlayer : public PPlayer
{
    std::vector<int> arp = { 0, 4, 7, 2, 7, 12, 5 };
    virtual std::string getName() { return "Quick Arp"; }
    virtual void step(int voiceCount, float sampleTime, float phase, float dPhase) {
        int pos = std::max(0, std::min((int)(phase * 16), 15));
        if( pos < voiceCount && ! notes[pos].on )
        {
            int n = pos % arp.size();
            int o = pos / arp.size();
            notes[pos].on = true;
            notes[pos].vel = 8;
            notes[pos].len = 0.3;
            notes[pos].tone = arp[n] / 12.0 + o;
        }
        shortenNotes(dPhase);
    }
};

struct CircleOfFifthsPlayer : public PPlayer
{
    float fifthPos = 0;
    virtual std::string getName() { return "Circle of 5ths"; }
    virtual void step(int voiceCount, float sampleTime, float phase, float dPhase) {
        if( phase > 1.0 )
        {
            std::vector<int> triad = { 0, 4, 7, 10 };
            notes[0].on = true;
            notes[0].vel = 8;
            notes[0].len = 0.8;
            notes[0].tone = fifthPos - 2;
            for( int i=1; i<voiceCount; ++i )
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
        shortenNotes(dPhase);
    }
};

struct RandomChordPlayer : public PPlayer {
    std::vector<int> directions;
    std::vector<float> snotes;

    virtual std::string getName() { return "Random Chord"; }
    RandomChordPlayer() : PPlayer() {
        directions.resize(16);
        snotes.resize(16);
        for( int i=0; i<16; ++i ) {
            directions[i] = rand() % 5 - 2;
            snotes[i] = -1.0 + rand() % 24 / 12.0;
        }
    }

    virtual void step(int voiceCount, float sampleTime, float phase, float dPhase) {
        if( phase > 1.0 )
        {
            for( int i=0; i<voiceCount; ++i )
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
        shortenNotes(dPhase);
    }
};

struct GoldbergPlayer : public PPlayer {
    smf::MidiFile mf;
    float currentTime = 0;
    int currentEvent = 0;
    int nextVoice = 0;

    virtual std::string getName() { return "Goldberg"; }
    virtual int minVoices(int userCount) { return 16; }
    
    GoldbergPlayer() : PPlayer() {
        rack::INFO( "Loading midi file" );
        mf.read(rack::asset::plugin(pluginInstance, "res/midi/988-v05.mid").c_str());
        mf.doTimeAnalysis();
        mf.linkNotePairs();
        mf.joinTracks();
        currentEvent = 0;
        currentTime = -1;
        rack::INFO( "mf[0].size is %d", mf[0].size());
    }
    
    virtual void step(int voiceCount, float sampleTime, float phase, float dPhase) {
        if( currentEvent >= mf[0].size())
        {
            rack::INFO( "Looping" );
            currentTime = -1.0;
            currentEvent = 0.0;
            for( int i=0; i<16; ++i )
                notes[i].on = false;
        }

        currentTime += sampleTime;
        while( currentEvent < mf[0].size() &&  mf[0][currentEvent].seconds < currentTime )
        {
            rack::INFO( "Got an event at %lf %d", currentTime, currentEvent );
            smf::MidiEvent &evt = mf[0][currentEvent];
            if (evt.isNoteOn())
            {
                int clv = nextVoice;
                int stopVoice = clv-1;
                if( stopVoice < 0)
                    stopVoice = voiceCount - 1;
                while( notes[nextVoice].on && nextVoice != stopVoice )
                {
                    nextVoice++;
                    if( nextVoice == voiceCount )
                        nextVoice = 0;
                }
                
                notes[nextVoice].on = true;
                notes[nextVoice].vel = evt.getVelocity() / 127.0 * 10;
                notes[nextVoice].tone = (evt.getKeyNumber() - 60) / 12.0 + 0.0;
                notes[nextVoice].key = evt.getKeyNumber();
                nextVoice ++;
                if( nextVoice == voiceCount )
                    nextVoice = 0;
            }
            if (evt.isNoteOff())
            {
                for( int i=0; i<16; ++i )
                    if( notes[i].key == evt.getKeyNumber() )
                        notes[i].on = false;
            }
            currentEvent ++;
        }
    }

    virtual void copyNotes( PPlayer *other ) {
        for( int i=0; i<16; ++i )
            notes[i].on = false;
    }

};

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

    std::unique_ptr<PPlayer> player = nullptr;
    
    PolyGenerator() : rack::Module() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam(BPM_PARAM, -2, 3, 1, "BPM" );
        configParam(VOICES_PARAM, 1, 16, 16, "Voice Count" );
        configParam(PATTERN_PARAM, 0, 5, 0, "Pattern" );
        resetPlayer(0);
    }

    void resetPlayer(int pattern) {
        PPlayer *np;
        switch(pattern) {
        case 1:
            np = new ArpPlayer();
            break;
        case 2:
            np = new CircleOfFifthsPlayer();
            break;
        case 3:
            np = new RandomChordPlayer();
            break;
        case 4:
            np = new GoldbergPlayer();
            break;
        case 0:
        default:
            np = new RandomTunedPlayer();
            break;
        };
        if( player != nullptr )
        {
            np->copyNotes(player.get());
        }
        player.reset(np);
        player->pattern = pattern;
        rack::INFO( "Reset player to %d %s", pattern, player->getName().c_str());
    }
    
    void process( const ProcessArgs &args ) override {
        bpm = 60.0 * pow( 2.0, params[BPM_PARAM].getValue() );
        float samplesPerSecond = args.sampleRate;
        float samplesPerMinute = samplesPerSecond * 60;
        float samplesPerBeat = samplesPerMinute / bpm;
        dPhase = 1.0 / samplesPerBeat;

        phase += dPhase;
        int pattern = (int)params[PATTERN_PARAM].getValue();
        int voices = player->minVoices((int)(params[VOICES_PARAM].getValue()));
        outputs[TONE_CV].setChannels(voices);
        outputs[VEL_CV].setChannels(voices);
        outputs[GATE_CV].setChannels(voices);
        if( pattern != player->pattern )
            resetPlayer(pattern);

        player->step(voices, args.sampleTime, phase, dPhase);

        if( phase > 1.0 )
            phase -= 1.0;
        
        for( int i=0; i<voices; ++i )
        {
            outputs[TONE_CV].setVoltage(player->notes[i].tone,i);
            outputs[VEL_CV].setVoltage(player->notes[i].vel,i);
            outputs[GATE_CV].setVoltage(player->notes[i].on ? 10 : 0 ,i);
            
            if( player->notes[i].len < 0 )
                player->notes[i].on = false;
        }
    }
};


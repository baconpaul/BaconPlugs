#pragma once

#include "rack.hpp"
#include "BaconPlugs.hpp"
#include "MidiFile.h"

struct PPlayer
{
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

    virtual ~PPlayer() {}
    virtual void step(int voiceCount, float sampleTime, float phase, float dPhase, float extra) {}
    virtual void copyNotes(PPlayer *other)
    {
        for (int i = 0; i < 16; ++i)
            notes[i] = other->notes[i];
    }

    virtual std::string getName() = 0;
    virtual int minVoices(int userChoice) { return std::max(1, userChoice); }
    virtual bool extraActive() { return false; }

    bool dirty = true;
    virtual std::string extraLabel()
    {
        dirty = false;
        return "";
    }
    virtual bool extraLabelDirty() { return dirty; }

    void shortenNotes(float dPhase)
    {
        for (int i = 0; i < noteCount; ++i)
        {
            if (notes[i].on)
                notes[i].len -= dPhase;
        }
    }
};

struct RandomTunedPlayer : public PPlayer
{
    std::vector<int> major = {0, 2, 4, 5, 7, 9, 11, 12};

    virtual std::string getName() override { return "RND TUNE"; }

    virtual void step(int voiceCount, float sampleTime, float phase, float dPhase,
                      float extra) override
    {
        if (phase >= 1)
        {
            for (int i = 0; i < voiceCount; ++i)
            {
                if (!notes[i].on && rand() * 1.0 / RAND_MAX > 0.7)
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
    std::vector<int> arp = {0, 4, 7, 2, 7, 12, 5};
    virtual std::string getName() override { return "ARPS"; }
    virtual void step(int voiceCount, float sampleTime, float phase, float dPhase,
                      float extra) override
    {
        int pos = std::max(0, std::min((int)(phase * 16), 15));
        if (pos < voiceCount && !notes[pos].on)
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
    virtual std::string getName() override { return "CIRC O 5"; }
    virtual void step(int voiceCount, float sampleTime, float phase, float dPhase,
                      float extra) override
    {
        if (phase > 1.0)
        {
            std::vector<int> triad = {0, 4, 7, 10};
            notes[0].on = true;
            notes[0].vel = 8;
            notes[0].len = 0.8;
            notes[0].tone = fifthPos - 2;
            for (int i = 1; i < voiceCount; ++i)
            {
                int p = i - 1;
                int n = p % triad.size();
                int o = p / triad.size();
                notes[i].on = true;
                notes[i].vel = 8;
                notes[i].len = 0.8;
                notes[i].tone = fifthPos + triad[n] / 12.0 + o;
            }
            fifthPos += 7 / 12.0;
            if (fifthPos > 1)
                fifthPos -= 1;
        }
        shortenNotes(dPhase);
    }
};

struct RandomChordPlayer : public PPlayer
{
    std::vector<int> directions;
    std::vector<float> snotes;

    virtual std::string getName() override { return "RND CHRD"; }
    RandomChordPlayer() : PPlayer()
    {
        directions.resize(16);
        snotes.resize(16);
        for (int i = 0; i < 16; ++i)
        {
            directions[i] = rand() % 5 - 2;
            snotes[i] = -1.0 + rand() % 24 / 12.0;
        }
    }

    virtual void step(int voiceCount, float sampleTime, float phase, float dPhase,
                      float extra) override
    {
        if (phase > 1.0)
        {
            for (int i = 0; i < voiceCount; ++i)
            {
                if (!notes[i].on)
                {
                    snotes[i] += directions[i] * 1.0 / 12.0;
                    notes[i].on = true;
                    notes[i].vel = 8;
                    notes[i].len = 0.8 + rand() % 2;
                    notes[i].tone = snotes[i];

                    if (rand() % 100 > 60)
                    {
                        directions[i] = -2 + rand() % 5;
                    }
                    if (snotes[i] > 2.5 || snotes[i] < -2.5)
                        snotes[i] = 0;
                }
            }
        }
        shortenNotes(dPhase);
    }
};

struct ChaosPlayer : public PPlayer
{
    virtual std::string getName() override { return "CHAOS"; }

    virtual void step(int voiceCount, float sampleTime, float phase, float dPhase,
                      float extra) override
    {
        int modv = (int)(1000.0 + 5000 * extra);
        if (rand() % modv == 14)
        {
            for (int i = 0; i < voiceCount; ++i)
            {
                if (!notes[i].on)
                {
                    notes[i].on = true;
                    notes[i].tone = rand() % 10000 / 10000.0 * 5.0 - 2.0;
                    notes[i].len = rand() % 1000 / 500.0 + 0.1;
                    notes[i].vel = (rand() % 80 + 20) / 10.0;
                    break;
                }
            }
        }
        shortenNotes(dPhase);
    }

    virtual bool extraActive() override { return true; }
    virtual std::string extraLabel() override
    {
        dirty = false;
        return "DENSITY";
    }
};

struct MidiFilePlayer : public PPlayer
{
    smf::MidiFile mf;
    float currentTime = 0;
    int currentEvent = 0;
    int nextVoice = 0;

    virtual int minVoices(int userCount) override { return 16; }

    std::string fileDispV = "";
    int lastFile = -1;
    int nFiles = 1;

    virtual std::string fileName(int file) = 0;
    virtual std::string fileDisp(int file) = 0;

    virtual void loadFile(int var)
    {
        std::string f = fileName(var);
        INFO("loading file %s", f.c_str());
        mf.read(rack::asset::plugin(pluginInstance, f).c_str());
        mf.doTimeAnalysis();
        mf.linkNotePairs();
        mf.joinTracks();
        currentEvent = 0;
        currentTime = -0.25;

        fileDispV = fileDisp(var);
        dirty = true;
        for (int i = 0; i < 16; ++i)
            notes[i].on = false;
    }

    MidiFilePlayer() : PPlayer() {}

    virtual void step(int voiceCount, float sampleTime, float phase, float dPhase,
                      float extra) override
    {
        int wantedFile = rack::clamp((int)(extra * nFiles), 0, nFiles - 1);
        if (wantedFile != lastFile)
        {
            loadFile(wantedFile);
            lastFile = wantedFile;
        }

        if (currentEvent >= mf[0].size())
        {
            currentTime = -1.0;
            currentEvent = 0.0;
            for (int i = 0; i < 16; ++i)
                notes[i].on = false;
        }

        currentTime += sampleTime;
        while (currentEvent < mf[0].size() && mf[0][currentEvent].seconds < currentTime)
        {
            smf::MidiEvent &evt = mf[0][currentEvent];
            if (evt.isNoteOn())
            {
                int clv = nextVoice;
                int stopVoice = clv - 1;
                if (stopVoice < 0)
                    stopVoice = voiceCount - 1;
                while (notes[nextVoice].on && nextVoice != stopVoice)
                {
                    nextVoice++;
                    if (nextVoice == voiceCount)
                        nextVoice = 0;
                }

                notes[nextVoice].on = true;
                notes[nextVoice].vel = evt.getVelocity() / 127.0 * 10;
                notes[nextVoice].tone = (evt.getKeyNumber() - 60) / 12.0 + 0.0;
                notes[nextVoice].key = evt.getKeyNumber();
                nextVoice++;
                if (nextVoice == voiceCount)
                    nextVoice = 0;
            }
            if (evt.isNoteOff())
            {
                for (int i = 0; i < 16; ++i)
                    if (notes[i].key == evt.getKeyNumber())
                        notes[i].on = false;
            }
            currentEvent++;
        }
    }

    virtual bool extraActive() override { return true; }
    virtual void copyNotes(PPlayer *other) override
    {
        for (int i = 0; i < 16; ++i)
            notes[i].on = false;
    }

    virtual std::string extraLabel() override { return fileDispV; }
};

struct GoldbergPlayer : public MidiFilePlayer
{
    virtual std::string getName() override { return "GOLDBERG"; }

    GoldbergPlayer()
    {
        nFiles = 30;
        loadFile(5);
    }

    virtual std::string fileName(int file) override
    {
        char vn[256];
        snprintf(vn, 256, "res/midi/goldberg/988-v%02d.mid", file);
        return vn;
    }

    virtual std::string fileDisp(int file) override
    {
        if (file == 0)
        {
            return "Aria";
        }
        else
        {
            char vn[256];
            snprintf(vn, 256, "Var. %d", file);
            return vn;
        }
    }
};

struct BeethovenPlayer : public MidiFilePlayer
{
    virtual std::string getName() override { return "LV BEETH"; }

    BeethovenPlayer()
    {
        nFiles = 13;
        loadFile(5);
    }

    std::vector<std::string> pfx = {"moon", "pathetique", "waldstein", "hammerk"};

    virtual std::string fileName(int file) override
    {
        int which = file / 3;
        int var = file % 3;
        var++;
        if (file == 12)
        {
            which = 3;
            var = 4;
        } // late beethoven had more movements eh

        char vn[256];
        snprintf(vn, 256, "res/midi/beeth/%s_%d.mid", pfx[which].c_str(), var);
        return vn;
    }

    virtual std::string fileDisp(int file) override
    {
        int which = file / 3;
        int var = file % 3;
        var++;
        if (file == 12)
        {
            which = 3;
            var = 4;
        } // late beethoven had more movements eh
        char x[256];
        snprintf(x, 256, "%s %d", pfx[which].substr(0, 6).c_str(), var);
        return x;
    }
};

struct ChopinPlayer : public MidiFilePlayer
{
    virtual std::string getName() override { return "CHOPIN"; }

    ChopinPlayer()
    {
        nFiles = 24;
        loadFile(5);
    }

    virtual std::string fileName(int file) override
    {
        char vn[256];
        snprintf(vn, 256, "res/midi/chopin/chpn-p%d.mid", file);
        return vn;
    }

    virtual std::string fileDisp(int file) override
    {
        char vn[256];
        snprintf(vn, 256, "PRLD %d", file);
        return vn;
    }
};

struct DebussyPlayer : public MidiFilePlayer
{
    virtual std::string getName() override { return "DEBUSSY"; }

    DebussyPlayer()
    {
        nFiles = 2;
        loadFile(0);
    }

    virtual std::string fileName(int file) override
    {
        if (file == 1)
            return "res/midi/debussy/deb_prel.mid";
        else
            return "res/midi/debussy/deb_clai.mid";
    }

    virtual std::string fileDisp(int file) override
    {
        if (file == 1)
            return "PRELUDE";
        else
            return "CLAIR D";
    }
};

struct PolyGenerator : public rack::Module
{
    enum ParamIds
    {
        BPM_PARAM,
        VOICES_PARAM,
        PATTERN_PARAM,
        EXTRA_PARAM,
        NUM_PARAMS
    };
    enum InputIds
    {
        NUM_INPUTS
    };
    enum OutputIds
    {
        TONE_CV,
        VEL_CV,
        GATE_CV,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        BPM_LIGHT,
        VOICES_LIGHT,
        EXTRA_LIGHT,
        NUM_LIGHTS
    };

    float bpm = 120.0;
    float dPhase = 0;
    float phase = 0;

    std::unique_ptr<PPlayer> player = nullptr;

    PolyGenerator() : rack::Module()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam(BPM_PARAM, -2, 3, 1, "Clock Tempo", " bpm", 2.f, 60.f);
        configParam(VOICES_PARAM, 1, 16, 16, "Voice Count");
        configParam(PATTERN_PARAM, 0, 8, 0, "Pattern");
        configParam(EXTRA_PARAM, 0, 1, 0, "Extra");
        resetPlayer(0);
    }

    void resetPlayer(int pattern)
    {
        PPlayer *np;
        switch (pattern)
        {
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
            np = new ChaosPlayer();
            break;
        case 5:
            np = new GoldbergPlayer();
            break;
        case 6:
            np = new BeethovenPlayer();
            break;
        case 7:
            np = new ChopinPlayer();
            break;
        case 8:
            np = new DebussyPlayer();
            break;
        case 0:
        default:
            np = new RandomTunedPlayer();
            break;
        };
        if (player != nullptr)
        {
            np->copyNotes(player.get());
        }
        player.reset(np);
        player->pattern = pattern;
        patternStringDirty = true;
        patternString = player->getName();
    }

    void process(const ProcessArgs &args) override
    {
        bpm = 60.0 * pow(2.0, params[BPM_PARAM].getValue());
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
        if (pattern != player->pattern)
            resetPlayer(pattern);

        player->step(voices, args.sampleTime, phase, dPhase, params[EXTRA_PARAM].getValue());

        lights[BPM_LIGHT].value = bpm;
        lights[VOICES_LIGHT].value = voices;
        lights[EXTRA_LIGHT].value = player->extraActive() ? 10.0 : 0;

        if (phase > 1.0)
            phase -= 1.0;

        for (int i = 0; i < voices; ++i)
        {
            outputs[TONE_CV].setVoltage(player->notes[i].tone, i);
            outputs[VEL_CV].setVoltage(player->notes[i].vel, i);
            outputs[GATE_CV].setVoltage(player->notes[i].on ? 10 : 0, i);

            if (player->notes[i].len < 0)
                player->notes[i].on = false;
        }
    }

    bool patternStringDirty = true;
    std::string patternString = "pattern";
    static bool getPatternNameDirty(Module *that)
    {
        return dynamic_cast<PolyGenerator *>(that)->patternStringDirty;
    }
    static std::string getPatternName(Module *that)
    {
        dynamic_cast<PolyGenerator *>(that)->patternStringDirty = false;
        return dynamic_cast<PolyGenerator *>(that)->patternString;
    }

    static bool getExtraLabelDirty(Module *that)
    {
        return dynamic_cast<PolyGenerator *>(that)->player->extraLabelDirty();
    }
    static std::string getExtraLabel(Module *that)
    {
        return dynamic_cast<PolyGenerator *>(that)->player->extraLabel();
    }
};

// See associated .cpp file for copyright and other info

#ifndef __mdaPiano__
#define __mdaPiano__

#include <string.h>
#include <cstdint>

typedef int32_t VstInt32;

#define NPARAMS 12 // number of parameters
#define NPROGS 8   // number of programs
#define NOUTS 2    // number of outputs
#define NVOICES 32 // max polyphony
#define SUSTAIN 128
#define SILENCE 0.0001f // voice choking
#define WAVELEN 586348  // wave data bytes

class mdaPianoProgram
{
    friend class mdaPiano;

  public:
    mdaPianoProgram();
    ~mdaPianoProgram() {}

  private:
    float param[NPARAMS];
    char name[24];
};

struct VOICE // voice state
{
    VstInt32 delta; // sample playback
    VstInt32 frac;
    VstInt32 pos;
    VstInt32 end;
    VstInt32 loop;

    float env; // envelope
    float dec;

    float f0; // first-order LPF
    float f1;
    float ff;

    float outl;
    float outr;
    VstInt32 note; // remember what note triggered this
};

struct KGRP // keygroup
{
    VstInt32 root; // MIDI root note
    VstInt32 high; // highest note
    VstInt32 pos;
    VstInt32 end;
    VstInt32 loop;
};

class mdaPiano
{
  public:
    mdaPiano();
    ~mdaPiano();

    virtual void process(float **inputs, float **outputs, VstInt32 sampleframes);
    virtual void processReplacing(float **inputs, float **outputs, VstInt32 sampleframes);

    virtual void setProgram(VstInt32 program);
    virtual void setProgramName(char *name);
    virtual void getProgramName(char *name);
    virtual void setParameter(VstInt32 index, float value);
    virtual float getParameter(VstInt32 index);
    virtual void getParameterLabel(VstInt32 index, char *label);
    virtual void getParameterDisplay(VstInt32 index, char *text);
    virtual void getParameterName(VstInt32 index, char *text);
    virtual void setBlockSize(VstInt32 blockSize);
    virtual void resume();

    virtual bool getProgramNameIndexed(VstInt32 category, VstInt32 index, char *text);
    virtual bool copyProgram(VstInt32 destination);
    virtual bool getEffectName(char *name);
    virtual bool getVendorString(char *text);
    virtual bool getProductString(char *text);
    virtual VstInt32 getVendorVersion() { return 1; }
    virtual VstInt32 canDo(char *text);

    virtual VstInt32 getNumMidiInputChannels() { return 1; }

    VstInt32 guiUpdate;
    void guiGetDisplay(VstInt32 index, char *label);

  private:
    void update(); // my parameter update
    void suspend() {}
    void noteOn(VstInt32 note, VstInt32 velocity);
    void fillpatch(VstInt32 p, const char *name, float p0, float p1, float p2, float p3, float p4,
                   float p5, float p6, float p7, float p8, float p9, float p10, float p11);

    float param[NPARAMS];
    mdaPianoProgram *programs;
    float Fs, iFs;

#define EVENTBUFFER 120
#define EVENTS_DONE 99999999
    VstInt32 notes[EVENTBUFFER + 8]; // list of delta|note|velocity for current block

    /// global internal variables
    KGRP kgrp[16];
    VOICE voice[NVOICES];
    VstInt32 activevoices, poly, cpos;
    short *waves;
    VstInt32 cmax;
    float *comb, cdep, width, trim;
    VstInt32 size, sustain;
    float tune, fine, random, stretch;
    float muff, muffvel, sizevel, velsens, volume;

    int curProgram{0};
    int blockSize{8};

  public:
    double sampleRate{44100};
    void setSampleRate(double d) { sampleRate = d; }
    double getSampleRate() { return sampleRate; }
};

#endif

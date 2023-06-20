//
// VST2 Plug-in: "mda Piano" v1.0
//
// Copyright(c)1999-2000 Paul Kellett (maxim digital audio)
// Based on VST2 SDK (c)1996-1999 Steinberg Soft und Hardware GmbH, All Rights Reserved
//

#include "mdaPianoData.h"
#include "mdaPiano.h"

#include <stdio.h>
#include <math.h>

// #include "AEffEditor.hpp" ////for GUI

mdaPianoProgram::mdaPianoProgram()
{
    param[0] = 0.50f; // Decay
    param[1] = 0.50f; // Release
    param[2] = 0.50f; // Hardness

    param[3] = 0.50f; // Vel>Hard
    param[4] = 1.00f; // Muffle
    param[5] = 0.50f; // Vel>Muff

    param[6] = 0.33f; // Vel Curve
    param[7] = 0.50f; // Stereo
    param[8] = 0.33f; // Max Poly

    param[9] = 0.50f;  // Tune
    param[10] = 0.00f; // Random
    param[11] = 0.50f; // Stretch

    strcpy(name, "mda Piano");
}

mdaPiano::mdaPiano()
{
    Fs = 44100.0f;
    iFs = 1.0f / Fs;
    cmax = 0x7F; // just in case...

    programs = new mdaPianoProgram[NPROGS];
    if (programs)
    {
        // fill patches...
        VstInt32 i = 0;
        fillpatch(i++, "mda Piano", 0.500f, 0.500f, 0.500f, 0.5f, 0.803f, 0.251f, 0.376f, 0.500f,
                  0.330f, 0.500f, 0.246f, 0.500f);
        fillpatch(i++, "Plain Piano", 0.500f, 0.500f, 0.500f, 0.5f, 0.751f, 0.000f, 0.452f, 0.000f,
                  0.000f, 0.500f, 0.000f, 0.500f);
        fillpatch(i++, "Compressed Piano", 0.902f, 0.399f, 0.623f, 0.5f, 1.000f, 0.331f, 0.299f,
                  0.499f, 0.330f, 0.500f, 0.000f, 0.500f);
        fillpatch(i++, "Dance Piano", 0.399f, 0.251f, 1.000f, 0.5f, 0.672f, 0.124f, 0.127f, 0.249f,
                  0.330f, 0.500f, 0.283f, 0.667f);
        fillpatch(i++, "Concert Piano", 0.648f, 0.500f, 0.500f, 0.5f, 0.298f, 0.602f, 0.550f,
                  0.850f, 0.356f, 0.500f, 0.339f, 0.660f);
        fillpatch(i++, "Dark Piano", 0.500f, 0.602f, 0.000f, 0.5f, 0.304f, 0.200f, 0.336f, 0.651f,
                  0.330f, 0.500f, 0.317f, 0.500f);
        fillpatch(i++, "School Piano", 0.450f, 0.598f, 0.626f, 0.5f, 0.603f, 0.500f, 0.174f, 0.331f,
                  0.330f, 0.500f, 0.421f, 0.801f);
        fillpatch(i++, "Broken Piano", 0.050f, 0.957f, 0.500f, 0.5f, 0.299f, 1.000f, 0.000f, 0.500f,
                  0.330f, 0.450f, 0.718f, 0.000f);

        setProgram(0);
    }

    waves = pianoData;

    // clang-format off
    //Waveform data and keymapping is hard-wired in *this* version
    kgrp[ 0].root = 36;  kgrp[ 0].high = 37;  kgrp[ 0].pos = 0;       kgrp[ 0].end = 36275;   kgrp[ 0].loop = 14774;
    kgrp[ 1].root = 40;  kgrp[ 1].high = 41;  kgrp[ 1].pos = 36278;   kgrp[ 1].end = 83135;   kgrp[ 1].loop = 16268;
    kgrp[ 2].root = 43;  kgrp[ 2].high = 45;  kgrp[ 2].pos = 83137;   kgrp[ 2].end = 146756;  kgrp[ 2].loop = 33541;
    kgrp[ 3].root = 48;  kgrp[ 3].high = 49;  kgrp[ 3].pos = 146758;  kgrp[ 3].end = 204997;  kgrp[ 3].loop = 21156;
    kgrp[ 4].root = 52;  kgrp[ 4].high = 53;  kgrp[ 4].pos = 204999;  kgrp[ 4].end = 244908;  kgrp[ 4].loop = 17191;
    kgrp[ 5].root = 55;  kgrp[ 5].high = 57;  kgrp[ 5].pos = 244910;  kgrp[ 5].end = 290978;  kgrp[ 5].loop = 23286;
    kgrp[ 6].root = 60;  kgrp[ 6].high = 61;  kgrp[ 6].pos = 290980;  kgrp[ 6].end = 342948;  kgrp[ 6].loop = 18002;
    kgrp[ 7].root = 64;  kgrp[ 7].high = 65;  kgrp[ 7].pos = 342950;  kgrp[ 7].end = 391750;  kgrp[ 7].loop = 19746;
    kgrp[ 8].root = 67;  kgrp[ 8].high = 69;  kgrp[ 8].pos = 391752;  kgrp[ 8].end = 436915;  kgrp[ 8].loop = 22253;
    kgrp[ 9].root = 72;  kgrp[ 9].high = 73;  kgrp[ 9].pos = 436917;  kgrp[ 9].end = 468807;  kgrp[ 9].loop = 8852;
    kgrp[10].root = 76;  kgrp[10].high = 77;  kgrp[10].pos = 468809;  kgrp[10].end = 492772;  kgrp[10].loop = 9693;
    kgrp[11].root = 79;  kgrp[11].high = 81;  kgrp[11].pos = 492774;  kgrp[11].end = 532293;  kgrp[11].loop = 10596;
    kgrp[12].root = 84;  kgrp[12].high = 85;  kgrp[12].pos = 532295;  kgrp[12].end = 560192;  kgrp[12].loop = 6011;
    kgrp[13].root = 88;  kgrp[13].high = 89;  kgrp[13].pos = 560194;  kgrp[13].end = 574121;  kgrp[13].loop = 3414;
    kgrp[14].root = 93;  kgrp[14].high = 999; kgrp[14].pos = 574123;  kgrp[14].end = 586343;  kgrp[14].loop = 2399;
    // clang-format on

    // initialise...
    for (VstInt32 v = 0; v < NVOICES; v++)
    {
        voice[v].env = 0.0f;
        voice[v].dec = 0.99f; // all notes off
    }
    notes[0] = EVENTS_DONE;
    volume = 0.2f;
    muff = 160.0f;
    cpos = sustain = activevoices = 0;
    comb = new float[256];

    guiUpdate = 0;

    update();
    suspend();
}

void mdaPiano::update() // parameter change
{
    float *param = programs[curProgram].param;
    size = (VstInt32)(12.0f * param[2] - 6.0f);
    sizevel = 0.12f * param[3];
    muffvel = param[5] * param[5] * 5.0f;

    velsens = 1.0f + param[6] + param[6];
    if (param[6] < 0.25f)
        velsens -= 0.75f - 3.0f * param[6];

    fine = param[9] - 0.5f;
    random = 0.077f * param[10] * param[10];
    stretch = 0.000434f * (param[11] - 0.5f);

    cdep = param[7] * param[7];
    trim = 1.50f - 0.79f * cdep;
    width = 0.04f * param[7];
    if (width > 0.03f)
        width = 0.03f;

    poly = 8 + (VstInt32)(24.9f * param[8]);
}

void mdaPiano::resume()
{
    Fs = getSampleRate();
    iFs = 1.0f / Fs;
    if (Fs > 64000.0f)
        cmax = 0xFF;
    else
        cmax = 0x7F;
    memset(comb, 0, sizeof(float) * 256);
}

mdaPiano::~mdaPiano() // destroy any buffers...
{
    if (programs)
        delete[] programs;
    if (comb)
        delete[] comb;
}

void mdaPiano::setProgram(VstInt32 program)
{
    curProgram = program;
    update();

    // TODO: guiUpdate ???
}

void mdaPiano::setParameter(VstInt32 index, float value)
{
    programs[curProgram].param[index] = value;
    update();

    //  if(editor) editor->postUpdate(); //For GUI

    guiUpdate = index + 0x100 + (guiUpdate & 0xFFFF00);
}

void mdaPiano::fillpatch(VstInt32 p, const char *name, float p0, float p1, float p2, float p3,
                         float p4, float p5, float p6, float p7, float p8, float p9, float p10,
                         float p11)
{
    strcpy(programs[p].name, name);
    programs[p].param[0] = p0;
    programs[p].param[1] = p1;
    programs[p].param[2] = p2;
    programs[p].param[3] = p3;
    programs[p].param[4] = p4;
    programs[p].param[5] = p5;
    programs[p].param[6] = p6;
    programs[p].param[7] = p7;
    programs[p].param[8] = p8;
    programs[p].param[9] = p9;
    programs[p].param[10] = p10;
    programs[p].param[11] = p11;
}

float mdaPiano::getParameter(VstInt32 index) { return programs[curProgram].param[index]; }
void mdaPiano::setProgramName(char *name) { strcpy(programs[curProgram].name, name); }
void mdaPiano::getProgramName(char *name) { strcpy(name, programs[curProgram].name); }
void mdaPiano::setBlockSize(VstInt32 bs) { this->blockSize = bs; }
bool mdaPiano::getEffectName(char *name)
{
    strcpy(name, "Piano");
    return true;
}
bool mdaPiano::getVendorString(char *text)
{
    strcpy(text, "mda");
    return true;
}
bool mdaPiano::getProductString(char *text)
{
    strcpy(text, "mda Piano");
    return true;
}


bool mdaPiano::getProgramNameIndexed(VstInt32 category, VstInt32 index, char *text)
{
    if ((unsigned int)index < NPROGS)
    {
        strcpy(text, programs[index].name);
        return true;
    }
    return false;
}

bool mdaPiano::copyProgram(VstInt32 destination)
{
    if (destination < NPROGS)
    {
        programs[destination] = programs[curProgram];
        return true;
    }
    return false;
}

VstInt32 mdaPiano::canDo(char *text)
{
    if (strcmp(text, "receiveVstEvents") == 0)
        return 1;
    if (strcmp(text, "receiveVstMidiEvent") == 0)
        return 1;
    return -1;
}

void mdaPiano::getParameterName(VstInt32 index, char *label)
{
    switch (index)
    {
    case 0:
        strcpy(label, "Envelope Decay");
        break;
    case 1:
        strcpy(label, "Envelope Release");
        break;
    case 2:
        strcpy(label, "Hardness Offset");
        break;

    case 3:
        strcpy(label, "Velocity to Hardness");
        break;
    case 4:
        strcpy(label, "Muffling Filter");
        break;
    case 5:
        strcpy(label, "Velocity to Muffling");
        break;

    case 6:
        strcpy(label, "Velocity Sensitivity");
        break;
    case 7:
        strcpy(label, "Stereo Width");
        break;
    case 8:
        strcpy(label, "Polyphony");
        break;

    case 9:
        strcpy(label, "Fine Tuning");
        break;
    case 10:
        strcpy(label, "Random Detuning");
        break;
    default:
        strcpy(label, "Stretch Tuning");
        break;
    }
}

void mdaPiano::getParameterDisplay(VstInt32 index, char *text)
{
    char string[16];
    float *param = programs[curProgram].param;

    switch (index)
    {
    case 4:
        sprintf(string, "%.0f", 100.0f - 100.0f * param[index]);
        break;
    case 7:
        sprintf(string, "%.0f", 200.0f * param[index]);
        break;
    case 8:
        sprintf(string, "%d", poly);
        break;
    case 10:
        sprintf(string, "%.1f", 50.0f * param[index] * param[index]);
        break;
    case 2:
    case 9:
    case 11:
        sprintf(string, "%+.1f", 100.0f * param[index] - 50.0f);
        break;
    default:
        sprintf(string, "%.0f", 100.0f * param[index]);
    }
    string[8] = 0;
    strcpy(text, (char *)string);
}

void mdaPiano::getParameterLabel(VstInt32 index, char *label)
{
    switch (index)
    {
    case 8:
        strcpy(label, "voices");
        break;
    case 9:
    case 10:
    case 11:
        strcpy(label, "cents");
        break;
    default:
        strcpy(label, "%");
    }
}

void mdaPiano::guiGetDisplay(VstInt32 index, char *label)
{
    getParameterName(index, label);
    strcat(label, " = ");
    getParameterDisplay(index, label + strlen(label));
    getParameterLabel(index, label + strlen(label));
}

void mdaPiano::process(float **inputs, float **outputs, VstInt32 sampleFrames)
{
    float *out0 = outputs[0];
    float *out1 = outputs[1];
    VstInt32 event = 0, frame = 0, frames, v;
    float x, l, r;
    VstInt32 i;

    while (frame < sampleFrames)
    {
        frames = notes[event++];
        if (frames > sampleFrames)
            frames = sampleFrames;
        frames -= frame;
        frame += frames;

        while (--frames >= 0)
        {
            VOICE *V = voice;
            l = r = 0.0f;

            for (v = 0; v < activevoices; v++)
            {
                V->frac += V->delta; // integer-based linear interpolation
                V->pos += V->frac >> 16;
                V->frac &= 0xFFFF;
                if (V->pos > V->end)
                    V->pos -= V->loop;
                i = waves[V->pos];
                i = (i << 7) + (V->frac >> 9) * (waves[V->pos + 1] - i) + 0x40400000;
                x = V->env * (*(float *)&i - 3.0f); // fast int->float

                V->env = V->env * V->dec;             // envelope
                V->f0 += V->ff * (x + V->f1 - V->f0); // muffle filter
                V->f1 = x;

                l += V->outl * V->f0;
                r += V->outr * V->f0;

                V++;
            }
            comb[cpos] = l + r;
            ++cpos &= cmax;
            x = cdep * comb[cpos]; // stereo simulator

            *out0++ += l + x;
            *out1++ += r - x;
        }

        if (frame < sampleFrames)
        {
            VstInt32 note = notes[event++];
            VstInt32 vel = notes[event++];
            noteOn(note, vel);
        }
    }
    for (v = 0; v < activevoices; v++)
        if (voice[v].env < SILENCE)
            voice[v] = voice[--activevoices];
    notes[0] = EVENTS_DONE; // mark events buffer as done
}

void mdaPiano::processReplacing(float **inputs, float **outputs, VstInt32 sampleFrames)
{
    float *out0 = outputs[0];
    float *out1 = outputs[1];
    VstInt32 event = 0, frame = 0, frames, v;
    float x, l, r;
    VstInt32 i;

    while (frame < sampleFrames)
    {
        frames = notes[event++];
        if (frames > sampleFrames)
            frames = sampleFrames;
        frames -= frame;
        frame += frames;

        while (--frames >= 0)
        {
            VOICE *V = voice;
            l = r = 0.0f;

            for (v = 0; v < activevoices; v++)
            {
                V->frac += V->delta; // integer-based linear interpolation
                V->pos += V->frac >> 16;
                V->frac &= 0xFFFF;
                if (V->pos > V->end)
                    V->pos -= V->loop;
                // i = (i << 7) + (V->frac >> 9) * (waves[V->pos + 1] - i) + 0x40400000;   //not
                // working on intel mac !?!
                i = waves[V->pos] + ((V->frac * (waves[V->pos + 1] - waves[V->pos])) >> 16);
                x = V->env * (float)i / 32768.0f;
                // x = V->env * (*(float *)&i - 3.0f);  //fast int->float

                V->env = V->env * V->dec;             // envelope
                V->f0 += V->ff * (x + V->f1 - V->f0); // muffle filter
                V->f1 = x;

                l += V->outl * V->f0;
                r += V->outr * V->f0;

                if (!(l > -2.0f) || !(l < 2.0f))
                {
                    printf("what is this shit?   %d,  %f,  %f\n", i, x, V->f0);
                    l = 0.0f;
                }
                if (!(r > -2.0f) || !(r < 2.0f))
                {
                    r = 0.0f;
                }

                V++;
            }
            comb[cpos] = l + r;
            ++cpos &= cmax;
            x = cdep * comb[cpos]; // stereo simulator

            *out0++ = l + x;
            *out1++ = r - x;
        }

        if (frame < sampleFrames)
        {
            VstInt32 note = notes[event++];
            VstInt32 vel = notes[event++];
            noteOn(note, vel);
        }
    }
    for (v = 0; v < activevoices; v++)
        if (voice[v].env < SILENCE)
            voice[v] = voice[--activevoices];
    notes[0] = EVENTS_DONE; // mark events buffer as done
}

void mdaPiano::noteOn(VstInt32 note, VstInt32 velocity)
{
    float *param = programs[curProgram].param;
    float l = 99.0f;
    VstInt32 v, vl = 0, k, s;

    if (velocity > 0)
    {
        if (activevoices < poly) // add a note
        {
            vl = activevoices;
            activevoices++;
        }
        else // steal a note
        {
            for (v = 0; v < poly; v++) // find quietest voice
            {
                if (voice[v].env < l)
                {
                    l = voice[v].env;
                    vl = v;
                }
            }
        }

        k = (note - 60) * (note - 60);
        l = fine + random * ((float)(k % 13) - 6.5f); // random & fine tune
        if (note > 60)
            l += stretch * (float)k; // stretch

        s = size;
        if (velocity > 40)
            s += (VstInt32)(sizevel * (float)(velocity - 40));

        k = 0;
        while (note > (kgrp[k].high + s))
            k++; // find keygroup

        l += (float)(note - kgrp[k].root); // pitch
        l = 22050.0f * iFs * (float)exp(0.05776226505 * l);
        voice[vl].delta = (VstInt32)(65536.0f * l);
        voice[vl].frac = 0;
        voice[vl].pos = kgrp[k].pos;
        voice[vl].end = kgrp[k].end;
        voice[vl].loop = kgrp[k].loop;

        voice[vl].env = (0.5f + velsens) * (float)pow(0.0078f * velocity, velsens); // velocity

        l = 50.0f + param[4] * param[4] * muff + muffvel * (float)(velocity - 64); // muffle
        if (l < (55.0f + 0.25f * (float)note))
            l = 55.0f + 0.25f * (float)note;
        if (l > 210.0f)
            l = 210.0f;
        voice[vl].ff = l * l * iFs;
        voice[vl].f0 = voice[vl].f1 = 0.0f;

        voice[vl].note = note; // note->pan
        if (note < 12)
            note = 12;
        if (note > 108)
            note = 108;
        l = volume * trim;
        voice[vl].outr = l + l * width * (float)(note - 60);
        voice[vl].outl = l + l - voice[vl].outr;

        if (note < 44)
            note = 44; // limit max decay length
        l = 2.0f * param[0];
        if (l < 1.0f)
            l += 0.25f - 0.5f * param[0];
        voice[vl].dec = (float)exp(-iFs * exp(-0.6 + 0.033 * (double)note - l));
    }
    else // note off
    {
        for (v = 0; v < NVOICES; v++)
            if (voice[v].note == note) // any voices playing that note?
            {
                if (sustain == 0)
                {
                    if (note < 94 || note == SUSTAIN) // no release on highest notes
                        voice[v].dec =
                            (float)exp(-iFs * exp(2.0 + 0.017 * (double)note - 2.0 * param[1]));
                }
                else
                    voice[v].note = SUSTAIN;
            }
    }
}

#if 0
VstInt32 mdaPiano::processEvents(VstEvents *ev)
{
    VstInt32 npos = 0;

    for (VstInt32 i = 0; i < ev->numEvents; i++)
    {
        if ((ev->events[i])->type != kVstMidiType)
            continue;
        VstMidiEvent *event = (VstMidiEvent *)ev->events[i];
        char *midiData = event->midiData;

        switch (midiData[0] & 0xf0) // status byte (all channels)
        {
        case 0x80:                              // note off
            notes[npos++] = event->deltaFrames; // delta
            notes[npos++] = midiData[1] & 0x7F; // note
            notes[npos++] = 0;                  // vel
            break;

        case 0x90:                              // note on
            notes[npos++] = event->deltaFrames; // delta
            notes[npos++] = midiData[1] & 0x7F; // note
            notes[npos++] = midiData[2] & 0x7F; // vel
            break;

        case 0xB0: // controller
            switch (midiData[1])
            {
            case 0x01: // mod wheel
            case 0x43: // soft pedal
                muff = 0.01f * (float)((127 - midiData[2]) * (127 - midiData[2]));
                break;

            case 0x07: // volume
                volume = 0.00002f * (float)(midiData[2] * midiData[2]);
                break;

            case 0x40: // sustain pedal
            case 0x42: // sustenuto pedal
                sustain = midiData[2] & 0x40;
                if (sustain == 0)
                {
                    notes[npos++] = event->deltaFrames;
                    notes[npos++] = SUSTAIN; // end all sustained notes
                    notes[npos++] = 0;
                }
                break;

            default: // all notes off
                if (midiData[1] > 0x7A)
                {
                    for (VstInt32 v = 0; v < NVOICES; v++)
                        voice[v].dec = 0.99f;
                    sustain = 0;
                    muff = 160.0f;
                }
                break;
            }
            break;

        case 0xC0: // program change
            if (midiData[1] < NPROGS)
                setProgram(midiData[1]);
            break;

        default:
            break;
        }

        if (npos > EVENTBUFFER)
            npos -= 3; // discard events if buffer full!!
        event++;       //?
    }
    notes[npos] = EVENTS_DONE;
    return 1;
}
#endif

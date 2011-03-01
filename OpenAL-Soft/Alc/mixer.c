/**
 * OpenAL cross platform audio library
 * Copyright (C) 1999-2007 by authors.
 * This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the
 *  Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA  02111-1307, USA.
 * Or go to http://www.gnu.org/copyleft/lgpl.html
 */

#include "config.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "alMain.h"
#include "AL/al.h"
#include "AL/alc.h"
#include "alSource.h"
#include "alBuffer.h"
#include "alListener.h"
#include "alAuxEffectSlot.h"
#include "alu.h"
#include "bs2b.h"


static __inline ALfloat point32(ALfloat val1, ALfloat val2, ALint frac)
{
    return val1;
    (void)val2;
    (void)frac;
}
static __inline ALfloat lerp32(ALfloat val1, ALfloat val2, ALint frac)
{
    val1 += ((val2-val1) * (frac * (1.0/(1<<FRACTIONBITS))));
    return val1;
}
static __inline ALfloat cos_lerp32(ALfloat val1, ALfloat val2, ALint frac)
{
    val1 += ((val2-val1) * ((1.0-cos(frac * (1.0/(1<<FRACTIONBITS)) * M_PI)) * 0.5));
    return val1;
}

static __inline ALfloat point16(ALfloat val1, ALfloat val2, ALint frac)
{
    return val1 / 32767.0f;
    (void)val2;
    (void)frac;
}
static __inline ALfloat lerp16(ALfloat val1, ALfloat val2, ALint frac)
{
    val1 += (val2-val1) * (frac * (1.0/(1<<FRACTIONBITS)));
    return val1 / 32767.0f;
}
static __inline ALfloat cos_lerp16(ALfloat val1, ALfloat val2, ALint frac)
{
    val1 += (val2-val1) * ((1.0-cos(frac * (1.0/(1<<FRACTIONBITS)) * M_PI)) * 0.5);
    return val1 / 32767.0f;
}

static __inline ALfloat point8(ALfloat val1, ALfloat val2, ALint frac)
{
    return (val1-128.0f) / 127.0f;
    (void)val2;
    (void)frac;
}
static __inline ALfloat lerp8(ALfloat val1, ALfloat val2, ALint frac)
{
    val1 += (val2-val1) * (frac * (1.0/(1<<FRACTIONBITS)));
    return (val1-128.0f) / 127.0f;
}
static __inline ALfloat cos_lerp8(ALfloat val1, ALfloat val2, ALint frac)
{
    val1 += (val2-val1) * ((1.0-cos(frac * (1.0/(1<<FRACTIONBITS)) * M_PI)) * 0.5);
    return (val1-128.0f) / 127.0f;
}


#define DECL_MIX_MONO(T,sampler)                                              \
static void MixMono_##T##_##sampler(ALsource *Source, ALCdevice *Device,      \
  const T *data, ALuint *DataPosInt, ALuint *DataPosFrac,                     \
  ALuint j, ALuint SamplesToDo, ALuint BufferSize)                            \
{                                                                             \
    ALfloat (*DryBuffer)[OUTPUTCHANNELS];                                     \
    ALfloat *ClickRemoval, *PendingClicks;                                    \
    ALuint pos, frac;                                                         \
    ALfloat DrySend[OUTPUTCHANNELS];                                          \
    FILTER *DryFilter;                                                        \
    ALuint BufferIdx;                                                         \
    ALuint increment;                                                         \
    ALuint i, out;                                                            \
    ALfloat value;                                                            \
                                                                              \
    increment = Source->Params.Step;                                          \
                                                                              \
    DryBuffer = Device->DryBuffer;                                            \
    ClickRemoval = Device->ClickRemoval;                                      \
    PendingClicks = Device->PendingClicks;                                    \
    DryFilter = &Source->Params.iirFilter;                                    \
    for(i = 0;i < OUTPUTCHANNELS;i++)                                         \
        DrySend[i] = Source->Params.DryGains[i];                              \
                                                                              \
    pos = 0;                                                                  \
    frac = *DataPosFrac;                                                      \
                                                                              \
    if(j == 0)                                                                \
    {                                                                         \
        value = sampler(data[pos], data[pos+1], frac);                        \
                                                                              \
        value = lpFilter4PC(DryFilter, 0, value);                             \
        ClickRemoval[FRONT_LEFT]   -= value*DrySend[FRONT_LEFT];              \
        ClickRemoval[FRONT_RIGHT]  -= value*DrySend[FRONT_RIGHT];             \
        ClickRemoval[SIDE_LEFT]    -= value*DrySend[SIDE_LEFT];               \
        ClickRemoval[SIDE_RIGHT]   -= value*DrySend[SIDE_RIGHT];              \
        ClickRemoval[BACK_LEFT]    -= value*DrySend[BACK_LEFT];               \
        ClickRemoval[BACK_RIGHT]   -= value*DrySend[BACK_RIGHT];              \
        ClickRemoval[FRONT_CENTER] -= value*DrySend[FRONT_CENTER];            \
        ClickRemoval[BACK_CENTER]  -= value*DrySend[BACK_CENTER];             \
    }                                                                         \
    for(BufferIdx = 0;BufferIdx < BufferSize;BufferIdx++)                     \
    {                                                                         \
        /* First order interpolator */                                        \
        value = sampler(data[pos], data[pos+1], frac);                        \
                                                                              \
        /* Direct path final mix buffer and panning */                        \
        value = lpFilter4P(DryFilter, 0, value);                              \
        DryBuffer[j][FRONT_LEFT]   += value*DrySend[FRONT_LEFT];              \
        DryBuffer[j][FRONT_RIGHT]  += value*DrySend[FRONT_RIGHT];             \
        DryBuffer[j][SIDE_LEFT]    += value*DrySend[SIDE_LEFT];               \
        DryBuffer[j][SIDE_RIGHT]   += value*DrySend[SIDE_RIGHT];              \
        DryBuffer[j][BACK_LEFT]    += value*DrySend[BACK_LEFT];               \
        DryBuffer[j][BACK_RIGHT]   += value*DrySend[BACK_RIGHT];              \
        DryBuffer[j][FRONT_CENTER] += value*DrySend[FRONT_CENTER];            \
        DryBuffer[j][BACK_CENTER]  += value*DrySend[BACK_CENTER];             \
                                                                              \
        frac += increment;                                                    \
        pos  += frac>>FRACTIONBITS;                                           \
        frac &= FRACTIONMASK;                                                 \
        j++;                                                                  \
    }                                                                         \
    if(j == SamplesToDo)                                                      \
    {                                                                         \
        value = sampler(data[pos], data[pos+1], frac);                        \
                                                                              \
        value = lpFilter4PC(DryFilter, 0, value);                             \
        PendingClicks[FRONT_LEFT]   += value*DrySend[FRONT_LEFT];             \
        PendingClicks[FRONT_RIGHT]  += value*DrySend[FRONT_RIGHT];            \
        PendingClicks[SIDE_LEFT]    += value*DrySend[SIDE_LEFT];              \
        PendingClicks[SIDE_RIGHT]   += value*DrySend[SIDE_RIGHT];             \
        PendingClicks[BACK_LEFT]    += value*DrySend[BACK_LEFT];              \
        PendingClicks[BACK_RIGHT]   += value*DrySend[BACK_RIGHT];             \
        PendingClicks[FRONT_CENTER] += value*DrySend[FRONT_CENTER];           \
        PendingClicks[BACK_CENTER]  += value*DrySend[BACK_CENTER];            \
    }                                                                         \
                                                                              \
    for(out = 0;out < Device->NumAuxSends;out++)                              \
    {                                                                         \
        ALfloat  WetSend;                                                     \
        ALfloat *WetBuffer;                                                   \
        ALfloat *WetClickRemoval;                                             \
        ALfloat *WetPendingClicks;                                            \
        FILTER  *WetFilter;                                                   \
                                                                              \
        if(!Source->Send[out].Slot ||                                         \
           Source->Send[out].Slot->effect.type == AL_EFFECT_NULL)             \
            continue;                                                         \
                                                                              \
        WetBuffer = Source->Send[out].Slot->WetBuffer;                        \
        WetClickRemoval = Source->Send[out].Slot->ClickRemoval;               \
        WetPendingClicks = Source->Send[out].Slot->PendingClicks;             \
        WetFilter = &Source->Params.Send[out].iirFilter;                      \
        WetSend = Source->Params.Send[out].WetGain;                           \
                                                                              \
        pos = 0;                                                              \
        frac = *DataPosFrac;                                                  \
        j -= BufferSize;                                                      \
                                                                              \
        if(j == 0)                                                            \
        {                                                                     \
            value = sampler(data[pos], data[pos+1], frac);                    \
                                                                              \
            value = lpFilter2PC(WetFilter, 0, value);                         \
            WetClickRemoval[0] -= value*WetSend;                              \
        }                                                                     \
        for(BufferIdx = 0;BufferIdx < BufferSize;BufferIdx++)                 \
        {                                                                     \
            /* First order interpolator */                                    \
            value = sampler(data[pos], data[pos+1], frac);                    \
                                                                              \
            /* Room path final mix buffer and panning */                      \
            value = lpFilter2P(WetFilter, 0, value);                          \
            WetBuffer[j] += value*WetSend;                                    \
                                                                              \
            frac += increment;                                                \
            pos  += frac>>FRACTIONBITS;                                       \
            frac &= FRACTIONMASK;                                             \
            j++;                                                              \
        }                                                                     \
        if(j == SamplesToDo)                                                  \
        {                                                                     \
            value = sampler(data[pos], data[pos+1], frac);                    \
                                                                              \
            value = lpFilter2PC(WetFilter, 0, value);                         \
            WetPendingClicks[0] += value*WetSend;                             \
        }                                                                     \
    }                                                                         \
    *DataPosInt += pos;                                                       \
    *DataPosFrac = frac;                                                      \
}

DECL_MIX_MONO(ALfloat, point32)
DECL_MIX_MONO(ALfloat, lerp32)
DECL_MIX_MONO(ALfloat, cos_lerp32)

DECL_MIX_MONO(ALshort, point16)
DECL_MIX_MONO(ALshort, lerp16)
DECL_MIX_MONO(ALshort, cos_lerp16)

DECL_MIX_MONO(ALubyte, point8)
DECL_MIX_MONO(ALubyte, lerp8)
DECL_MIX_MONO(ALubyte, cos_lerp8)


#define DECL_MIX_STEREO(T,sampler)                                            \
static void MixStereo_##T##_##sampler(ALsource *Source, ALCdevice *Device,    \
  const T *data, ALuint *DataPosInt, ALuint *DataPosFrac,                     \
  ALuint j, ALuint SamplesToDo, ALuint BufferSize)                            \
{                                                                             \
    static const ALuint Channels = 2;                                         \
    static const Channel chans[] = {                                          \
        FRONT_LEFT, FRONT_RIGHT,                                              \
        SIDE_LEFT, SIDE_RIGHT,                                                \
        BACK_LEFT, BACK_RIGHT                                                 \
    };                                                                        \
    const ALfloat scaler = 1.0f/Channels;                                     \
    ALfloat (*DryBuffer)[OUTPUTCHANNELS];                                     \
    ALfloat *ClickRemoval, *PendingClicks;                                    \
    ALuint pos, frac;                                                         \
    ALfloat DrySend[OUTPUTCHANNELS];                                          \
    FILTER *DryFilter;                                                        \
    ALuint BufferIdx;                                                         \
    ALuint increment;                                                         \
    ALuint i, out;                                                            \
    ALfloat value;                                                            \
                                                                              \
    increment = Source->Params.Step;                                          \
                                                                              \
    DryBuffer = Device->DryBuffer;                                            \
    ClickRemoval = Device->ClickRemoval;                                      \
    PendingClicks = Device->PendingClicks;                                    \
    DryFilter = &Source->Params.iirFilter;                                    \
    for(i = 0;i < OUTPUTCHANNELS;i++)                                         \
        DrySend[i] = Source->Params.DryGains[i];                              \
                                                                              \
    pos = 0;                                                                  \
    frac = *DataPosFrac;                                                      \
                                                                              \
    if(j == 0)                                                                \
    {                                                                         \
        for(i = 0;i < Channels;i++)                                           \
        {                                                                     \
            value = sampler(data[pos*Channels + i],                           \
                            data[(pos+1)*Channels + i], frac);                \
                                                                              \
            value = lpFilter2PC(DryFilter, chans[i]*2, value);                \
            ClickRemoval[chans[i+0]] -= value*DrySend[chans[i+0]];            \
            ClickRemoval[chans[i+2]] -= value*DrySend[chans[i+2]];            \
            ClickRemoval[chans[i+4]] -= value*DrySend[chans[i+4]];            \
        }                                                                     \
    }                                                                         \
    for(BufferIdx = 0;BufferIdx < BufferSize;BufferIdx++)                     \
    {                                                                         \
        for(i = 0;i < Channels;i++)                                           \
        {                                                                     \
            value = sampler(data[pos*Channels + i],                           \
                            data[(pos+1)*Channels + i], frac);                \
                                                                              \
            value = lpFilter2P(DryFilter, chans[i]*2, value);                 \
            DryBuffer[j][chans[i+0]] += value*DrySend[chans[i+0]];            \
            DryBuffer[j][chans[i+2]] += value*DrySend[chans[i+2]];            \
            DryBuffer[j][chans[i+4]] += value*DrySend[chans[i+4]];            \
        }                                                                     \
                                                                              \
        frac += increment;                                                    \
        pos  += frac>>FRACTIONBITS;                                           \
        frac &= FRACTIONMASK;                                                 \
        j++;                                                                  \
    }                                                                         \
    if(j == SamplesToDo)                                                      \
    {                                                                         \
        for(i = 0;i < Channels;i++)                                           \
        {                                                                     \
            value = sampler(data[pos*Channels + i],                           \
                            data[(pos+1)*Channels + i], frac);                \
                                                                              \
            value = lpFilter2PC(DryFilter, chans[i]*2, value);                \
            PendingClicks[chans[i+0]] += value*DrySend[chans[i+0]];           \
            PendingClicks[chans[i+2]] += value*DrySend[chans[i+2]];           \
            PendingClicks[chans[i+4]] += value*DrySend[chans[i+4]];           \
        }                                                                     \
    }                                                                         \
                                                                              \
    for(out = 0;out < Device->NumAuxSends;out++)                              \
    {                                                                         \
        ALfloat  WetSend;                                                     \
        ALfloat *WetBuffer;                                                   \
        ALfloat *WetClickRemoval;                                             \
        ALfloat *WetPendingClicks;                                            \
        FILTER  *WetFilter;                                                   \
                                                                              \
        if(!Source->Send[out].Slot ||                                         \
           Source->Send[out].Slot->effect.type == AL_EFFECT_NULL)             \
            continue;                                                         \
                                                                              \
        WetBuffer = Source->Send[out].Slot->WetBuffer;                        \
        WetClickRemoval = Source->Send[out].Slot->ClickRemoval;               \
        WetPendingClicks = Source->Send[out].Slot->PendingClicks;             \
        WetFilter = &Source->Params.Send[out].iirFilter;                      \
        WetSend = Source->Params.Send[out].WetGain;                           \
                                                                              \
        pos = 0;                                                              \
        frac = *DataPosFrac;                                                  \
        j -= BufferSize;                                                      \
                                                                              \
        if(j == 0)                                                            \
        {                                                                     \
            for(i = 0;i < Channels;i++)                                       \
            {                                                                 \
                value = sampler(data[pos*Channels + i],                       \
                                data[(pos+1)*Channels + i], frac);            \
                                                                              \
                value = lpFilter1PC(WetFilter, chans[i], value);              \
                WetClickRemoval[0] -= value*WetSend * scaler;                 \
            }                                                                 \
        }                                                                     \
        for(BufferIdx = 0;BufferIdx < BufferSize;BufferIdx++)                 \
        {                                                                     \
            for(i = 0;i < Channels;i++)                                       \
            {                                                                 \
                value = sampler(data[pos*Channels + i],                       \
                                data[(pos+1)*Channels + i], frac);            \
                                                                              \
                value = lpFilter1P(WetFilter, chans[i], value);               \
                WetBuffer[j] += value*WetSend * scaler;                       \
            }                                                                 \
                                                                              \
            frac += increment;                                                \
            pos  += frac>>FRACTIONBITS;                                       \
            frac &= FRACTIONMASK;                                             \
            j++;                                                              \
        }                                                                     \
        if(j == SamplesToDo)                                                  \
        {                                                                     \
            for(i = 0;i < Channels;i++)                                       \
            {                                                                 \
                value = sampler(data[pos*Channels + i],                       \
                                data[(pos+1)*Channels + i], frac);            \
                                                                              \
                value = lpFilter1PC(WetFilter, chans[i], value);              \
                WetPendingClicks[0] += value*WetSend * scaler;                \
            }                                                                 \
        }                                                                     \
    }                                                                         \
    *DataPosInt += pos;                                                       \
    *DataPosFrac = frac;                                                      \
}

DECL_MIX_STEREO(ALfloat, point32)
DECL_MIX_STEREO(ALfloat, lerp32)
DECL_MIX_STEREO(ALfloat, cos_lerp32)

DECL_MIX_STEREO(ALshort, point16)
DECL_MIX_STEREO(ALshort, lerp16)
DECL_MIX_STEREO(ALshort, cos_lerp16)

DECL_MIX_STEREO(ALubyte, point8)
DECL_MIX_STEREO(ALubyte, lerp8)
DECL_MIX_STEREO(ALubyte, cos_lerp8)


#define DECL_MIX_MC(T,chans,sampler)                                          \
static void MixMC_##T##_##chans##_##sampler(ALsource *Source, ALCdevice *Device,\
  const T *data, ALuint *DataPosInt, ALuint *DataPosFrac,                     \
  ALuint j, ALuint SamplesToDo, ALuint BufferSize)                            \
{                                                                             \
    static const ALuint Channels = sizeof(chans)/sizeof(chans[0]);            \
    const ALfloat scaler = 1.0f/Channels;                                     \
    ALfloat (*DryBuffer)[OUTPUTCHANNELS];                                     \
    ALfloat *ClickRemoval, *PendingClicks;                                    \
    ALuint pos, frac;                                                         \
    ALfloat DrySend[OUTPUTCHANNELS];                                          \
    FILTER *DryFilter;                                                        \
    ALuint BufferIdx;                                                         \
    ALuint increment;                                                         \
    ALuint i, out;                                                            \
    ALfloat value;                                                            \
                                                                              \
    increment = Source->Params.Step;                                          \
                                                                              \
    DryBuffer = Device->DryBuffer;                                            \
    ClickRemoval = Device->ClickRemoval;                                      \
    PendingClicks = Device->PendingClicks;                                    \
    DryFilter = &Source->Params.iirFilter;                                    \
    for(i = 0;i < OUTPUTCHANNELS;i++)                                         \
        DrySend[i] = Source->Params.DryGains[i];                              \
                                                                              \
    pos = 0;                                                                  \
    frac = *DataPosFrac;                                                      \
                                                                              \
    if(j == 0)                                                                \
    {                                                                         \
        for(i = 0;i < Channels;i++)                                           \
        {                                                                     \
            value = sampler(data[pos*Channels + i],                           \
                            data[(pos+1)*Channels + i], frac);                \
                                                                              \
            value = lpFilter2PC(DryFilter, chans[i]*2, value);                \
            ClickRemoval[chans[i]] -= value*DrySend[chans[i]];                \
        }                                                                     \
    }                                                                         \
    for(BufferIdx = 0;BufferIdx < BufferSize;BufferIdx++)                     \
    {                                                                         \
        for(i = 0;i < Channels;i++)                                           \
        {                                                                     \
            value = sampler(data[pos*Channels + i],                           \
                            data[(pos+1)*Channels + i], frac);                \
                                                                              \
            value = lpFilter2P(DryFilter, chans[i]*2, value);                 \
            DryBuffer[j][chans[i]] += value*DrySend[chans[i]];                \
        }                                                                     \
                                                                              \
        frac += increment;                                                    \
        pos  += frac>>FRACTIONBITS;                                           \
        frac &= FRACTIONMASK;                                                 \
        j++;                                                                  \
    }                                                                         \
    if(j == SamplesToDo)                                                      \
    {                                                                         \
        for(i = 0;i < Channels;i++)                                           \
        {                                                                     \
            value = sampler(data[pos*Channels + i],                           \
                            data[(pos+1)*Channels + i], frac);                \
                                                                              \
            value = lpFilter2PC(DryFilter, chans[i]*2, value);                \
            PendingClicks[chans[i]] += value*DrySend[chans[i]];               \
        }                                                                     \
    }                                                                         \
                                                                              \
    for(out = 0;out < Device->NumAuxSends;out++)                              \
    {                                                                         \
        ALfloat  WetSend;                                                     \
        ALfloat *WetBuffer;                                                   \
        ALfloat *WetClickRemoval;                                             \
        ALfloat *WetPendingClicks;                                            \
        FILTER  *WetFilter;                                                   \
                                                                              \
        if(!Source->Send[out].Slot ||                                         \
           Source->Send[out].Slot->effect.type == AL_EFFECT_NULL)             \
            continue;                                                         \
                                                                              \
        WetBuffer = Source->Send[out].Slot->WetBuffer;                        \
        WetClickRemoval = Source->Send[out].Slot->ClickRemoval;               \
        WetPendingClicks = Source->Send[out].Slot->PendingClicks;             \
        WetFilter = &Source->Params.Send[out].iirFilter;                      \
        WetSend = Source->Params.Send[out].WetGain;                           \
                                                                              \
        pos = 0;                                                              \
        frac = *DataPosFrac;                                                  \
        j -= BufferSize;                                                      \
                                                                              \
        if(j == 0)                                                            \
        {                                                                     \
            for(i = 0;i < Channels;i++)                                       \
            {                                                                 \
                value = sampler(data[pos*Channels + i],                       \
                                data[(pos+1)*Channels + i], frac);            \
                                                                              \
                value = lpFilter1PC(WetFilter, chans[i], value);              \
                WetClickRemoval[0] -= value*WetSend * scaler;                 \
            }                                                                 \
        }                                                                     \
        for(BufferIdx = 0;BufferIdx < BufferSize;BufferIdx++)                 \
        {                                                                     \
            for(i = 0;i < Channels;i++)                                       \
            {                                                                 \
                value = sampler(data[pos*Channels + i],                       \
                                data[(pos+1)*Channels + i], frac);            \
                                                                              \
                value = lpFilter1P(WetFilter, chans[i], value);               \
                WetBuffer[j] += value*WetSend * scaler;                       \
            }                                                                 \
                                                                              \
            frac += increment;                                                \
            pos  += frac>>FRACTIONBITS;                                       \
            frac &= FRACTIONMASK;                                             \
            j++;                                                              \
        }                                                                     \
        if(j == SamplesToDo)                                                  \
        {                                                                     \
            for(i = 0;i < Channels;i++)                                       \
            {                                                                 \
                value = sampler(data[pos*Channels + i],                       \
                                data[(pos+1)*Channels + i], frac);            \
                                                                              \
                value = lpFilter1PC(WetFilter, chans[i], value);              \
                WetPendingClicks[0] += value*WetSend * scaler;                \
            }                                                                 \
        }                                                                     \
    }                                                                         \
    *DataPosInt += pos;                                                       \
    *DataPosFrac = frac;                                                      \
}

static const Channel QuadChans[] = { FRONT_LEFT, FRONT_RIGHT,
                                     BACK_LEFT,  BACK_RIGHT };
DECL_MIX_MC(ALfloat, QuadChans, point32)
DECL_MIX_MC(ALfloat, QuadChans, lerp32)
DECL_MIX_MC(ALfloat, QuadChans, cos_lerp32)

DECL_MIX_MC(ALshort, QuadChans, point16)
DECL_MIX_MC(ALshort, QuadChans, lerp16)
DECL_MIX_MC(ALshort, QuadChans, cos_lerp16)

DECL_MIX_MC(ALubyte, QuadChans, point8)
DECL_MIX_MC(ALubyte, QuadChans, lerp8)
DECL_MIX_MC(ALubyte, QuadChans, cos_lerp8)


static const Channel X51Chans[] = { FRONT_LEFT,   FRONT_RIGHT,
                                    FRONT_CENTER, LFE,
                                    BACK_LEFT,  BACK_RIGHT };
DECL_MIX_MC(ALfloat, X51Chans, point32)
DECL_MIX_MC(ALfloat, X51Chans, lerp32)
DECL_MIX_MC(ALfloat, X51Chans, cos_lerp32)

DECL_MIX_MC(ALshort, X51Chans, point16)
DECL_MIX_MC(ALshort, X51Chans, lerp16)
DECL_MIX_MC(ALshort, X51Chans, cos_lerp16)

DECL_MIX_MC(ALubyte, X51Chans, point8)
DECL_MIX_MC(ALubyte, X51Chans, lerp8)
DECL_MIX_MC(ALubyte, X51Chans, cos_lerp8)


static const Channel X61Chans[] = { FRONT_LEFT,   FRONT_RIGHT,
                                    FRONT_CENTER, LFE,
                                    BACK_CENTER,
                                    SIDE_LEFT,    SIDE_RIGHT };
DECL_MIX_MC(ALfloat, X61Chans, point32)
DECL_MIX_MC(ALfloat, X61Chans, lerp32)
DECL_MIX_MC(ALfloat, X61Chans, cos_lerp32)

DECL_MIX_MC(ALshort, X61Chans, point16)
DECL_MIX_MC(ALshort, X61Chans, lerp16)
DECL_MIX_MC(ALshort, X61Chans, cos_lerp16)

DECL_MIX_MC(ALubyte, X61Chans, point8)
DECL_MIX_MC(ALubyte, X61Chans, lerp8)
DECL_MIX_MC(ALubyte, X61Chans, cos_lerp8)


static const Channel X71Chans[] = { FRONT_LEFT,   FRONT_RIGHT,
                                    FRONT_CENTER, LFE,
                                    BACK_LEFT,    BACK_RIGHT,
                                    SIDE_LEFT,    SIDE_RIGHT };
DECL_MIX_MC(ALfloat, X71Chans, point32)
DECL_MIX_MC(ALfloat, X71Chans, lerp32)
DECL_MIX_MC(ALfloat, X71Chans, cos_lerp32)

DECL_MIX_MC(ALshort, X71Chans, point16)
DECL_MIX_MC(ALshort, X71Chans, lerp16)
DECL_MIX_MC(ALshort, X71Chans, cos_lerp16)

DECL_MIX_MC(ALubyte, X71Chans, point8)
DECL_MIX_MC(ALubyte, X71Chans, lerp8)
DECL_MIX_MC(ALubyte, X71Chans, cos_lerp8)


#define DECL_MIX(T, sampler)                                                  \
static void Mix_##T##_##sampler(ALsource *Source, ALCdevice *Device, ALuint Channels, \
  const ALvoid *Data, ALuint *DataPosInt, ALuint *DataPosFrac,                \
  ALuint j, ALuint SamplesToDo, ALuint BufferSize)                            \
{                                                                             \
    switch(Channels)                                                          \
    {                                                                         \
    case 1: /* Mono */                                                        \
        MixMono_##T##_##sampler(Source, Device,                               \
                                Data, DataPosInt, DataPosFrac,                \
                                j, SamplesToDo, BufferSize);                  \
        break;                                                                \
    case 2: /* Stereo */                                                      \
        MixStereo_##T##_##sampler(Source, Device,                             \
                                  Data, DataPosInt, DataPosFrac,              \
                                  j, SamplesToDo, BufferSize);                \
        break;                                                                \
    case 4: /* Quad */                                                        \
        MixMC_##T##_QuadChans_##sampler(Source, Device,                       \
                                        Data, DataPosInt, DataPosFrac,        \
                                        j, SamplesToDo, BufferSize);          \
        break;                                                                \
    case 6: /* 5.1 */                                                         \
        MixMC_##T##_X51Chans_##sampler(Source, Device,                        \
                                       Data, DataPosInt, DataPosFrac,         \
                                       j, SamplesToDo, BufferSize);           \
        break;                                                                \
    case 7: /* 6.1 */                                                         \
        MixMC_##T##_X61Chans_##sampler(Source, Device,                        \
                                       Data, DataPosInt, DataPosFrac,         \
                                       j, SamplesToDo, BufferSize);           \
        break;                                                                \
    case 8: /* 7.1 */                                                         \
        MixMC_##T##_X71Chans_##sampler(Source, Device,                        \
                                       Data, DataPosInt, DataPosFrac,         \
                                       j, SamplesToDo, BufferSize);           \
        break;                                                                \
    }                                                                         \
}

DECL_MIX(ALfloat, point32)
DECL_MIX(ALfloat, lerp32)
DECL_MIX(ALfloat, cos_lerp32)

DECL_MIX(ALshort, point16)
DECL_MIX(ALshort, lerp16)
DECL_MIX(ALshort, cos_lerp16)

DECL_MIX(ALubyte, point8)
DECL_MIX(ALubyte, lerp8)
DECL_MIX(ALubyte, cos_lerp8)


/* Stack data size can be whatever. Larger values need more stack, while
 * smaller values may need more iterations */
#ifndef STACK_DATA_SIZE
#define STACK_DATA_SIZE  16384
#endif

ALvoid MixSource(ALsource *Source, ALCdevice *Device, ALuint SamplesToDo)
{
    ALbufferlistitem *BufferListItem;
    ALuint FrameSize, Channels, Bytes;
    ALuint DataPosInt, DataPosFrac;
    ALuint BuffersPlayed;
    ALboolean Looping;
    ALuint increment;
    ALenum State;
    ALuint i, j;
    ALint64 DataSize64;

    /* Get source info */
    State         = Source->state;
    BuffersPlayed = Source->BuffersPlayed;
    DataPosInt    = Source->position;
    DataPosFrac   = Source->position_fraction;
    Looping       = Source->bLooping;
    increment     = Source->Params.Step;

    /* Get buffer info */
    FrameSize = Channels = Bytes = 0;
    BufferListItem = Source->queue;
    for(i = 0;i < Source->BuffersInQueue;i++)
    {
        const ALbuffer *ALBuffer;
        if((ALBuffer=BufferListItem->buffer) != NULL)
        {
            FrameSize = aluFrameSizeFromFormat(ALBuffer->format);
            Channels = aluChannelsFromFormat(ALBuffer->format);
            Bytes = aluBytesFromFormat(ALBuffer->format);
            break;
        }
        BufferListItem = BufferListItem->next;
    }

    /* Get current buffer queue item */
    BufferListItem = Source->queue;
    for(i = 0;i < BuffersPlayed;i++)
        BufferListItem = BufferListItem->next;

    j = 0;
    do {
        ALubyte SrcData[STACK_DATA_SIZE];
        ALuint SrcDataSize = 0;
        ALuint BufferSize;

        /* Figure out how many buffer bytes will be needed */
        DataSize64  = SamplesToDo-j+1;
        DataSize64 *= increment;
        DataSize64 += DataPosFrac+FRACTIONMASK;
        DataSize64 >>= FRACTIONBITS;
        DataSize64 += BUFFER_PADDING;
        DataSize64 *= FrameSize;

        BufferSize = sizeof(SrcData) - SrcDataSize;
        BufferSize = min(DataSize64, BufferSize);

        if(Source->lSourceType == AL_STATIC)
        {
            const ALbuffer *ALBuffer = Source->Buffer;
            const ALubyte *Data = ALBuffer->data;
            ALuint DataSize;

            /* If current pos is beyond the loop range, do not loop */
            if(Looping == AL_FALSE || DataPosInt >= (ALuint)ALBuffer->LoopEnd)
            {
                Looping = AL_FALSE;

                /* Copy what's left to play in the source buffer, and clear the
                 * rest of the temp buffer */
                DataSize = ALBuffer->size - DataPosInt*FrameSize;
                DataSize = min(BufferSize, DataSize);

                memcpy(&SrcData[SrcDataSize], &Data[DataPosInt*FrameSize], DataSize);
                SrcDataSize += DataSize;
                BufferSize -= DataSize;

                memset(&SrcData[SrcDataSize], (Bytes==1)?0x80:0, BufferSize);
                SrcDataSize += BufferSize;
                BufferSize -= BufferSize;
            }
            else
            {
                ALuint LoopStart = ALBuffer->LoopStart;
                ALuint LoopEnd   = ALBuffer->LoopEnd;

                /* Copy what's left of this loop iteration, then copy repeats
                 * of the loop section */
                DataSize = (LoopEnd-DataPosInt) * FrameSize;
                DataSize = min(BufferSize, DataSize);

                memcpy(&SrcData[SrcDataSize], &Data[DataPosInt*FrameSize], DataSize);
                SrcDataSize += DataSize;
                BufferSize -= DataSize;

                DataSize = (LoopEnd-LoopStart) * FrameSize;
                while(BufferSize > 0)
                {
                    DataSize = min(BufferSize, DataSize);

                    memcpy(&SrcData[SrcDataSize], &Data[LoopStart*FrameSize], DataSize);
                    SrcDataSize += DataSize;
                    BufferSize -= DataSize;
                }
            }
        }
        else
        {
            /* Crawl the buffer queue to fill in the temp buffer */
            ALbufferlistitem *BufferListIter = BufferListItem;
            ALuint pos = DataPosInt*FrameSize;

            while(BufferListIter && BufferSize > 0)
            {
                const ALbuffer *ALBuffer;
                if((ALBuffer=BufferListIter->buffer) != NULL)
                {
                    const ALubyte *Data = ALBuffer->data;
                    ALuint DataSize = ALBuffer->size;

                    /* Skip the data already played */
                    if(DataSize <= pos)
                        pos -= DataSize;
                    else
                    {
                        Data += pos;
                        DataSize -= pos;
                        pos -= pos;

                        DataSize = min(BufferSize, DataSize);
                        memcpy(&SrcData[SrcDataSize], Data, DataSize);
                        SrcDataSize += DataSize;
                        BufferSize -= DataSize;
                    }
                }
                BufferListIter = BufferListIter->next;
                if(!BufferListIter && Looping)
                    BufferListIter = Source->queue;
                else if(!BufferListIter)
                {
                    memset(&SrcData[SrcDataSize], (Bytes==1)?0x80:0, BufferSize);
                    SrcDataSize += BufferSize;
                    BufferSize -= BufferSize;
                }
            }
        }

        /* Figure out how many samples we can mix. */
        DataSize64  = SrcDataSize / FrameSize;
        DataSize64 -= BUFFER_PADDING;
        DataSize64 <<= FRACTIONBITS;
        DataSize64 -= increment;

        BufferSize = (ALuint)((DataSize64-DataPosFrac+(increment-1)) / increment);
        BufferSize = min(BufferSize, (SamplesToDo-j));
        if(BufferSize == 0)
        {
            AL_PRINT("No samples to mix! Pitch too high (%u, %g)?\n",
                     increment, increment/(double)(1<<FRACTIONBITS));
            State = AL_STOPPED;
            BufferListItem = Source->queue;
            BuffersPlayed = Source->BuffersInQueue;
            DataPosInt = 0;
            DataPosFrac = 0;
            break;
        }

        switch(Source->Resampler)
        {
            case POINT_RESAMPLER:
                if(Bytes == 4)
                    Mix_ALfloat_point32(Source, Device, Channels,
                                        SrcData, &DataPosInt, &DataPosFrac,
                                        j, SamplesToDo, BufferSize);
                else if(Bytes == 2)
                    Mix_ALshort_point16(Source, Device, Channels,
                                        SrcData, &DataPosInt, &DataPosFrac,
                                        j, SamplesToDo, BufferSize);
                else if(Bytes == 1)
                    Mix_ALubyte_point8(Source, Device, Channels,
                                       SrcData, &DataPosInt, &DataPosFrac,
                                       j, SamplesToDo, BufferSize);
                break;
            case LINEAR_RESAMPLER:
                if(Bytes == 4)
                    Mix_ALfloat_lerp32(Source, Device, Channels,
                                       SrcData, &DataPosInt, &DataPosFrac,
                                       j, SamplesToDo, BufferSize);
                else if(Bytes == 2)
                    Mix_ALshort_lerp16(Source, Device, Channels,
                                       SrcData, &DataPosInt, &DataPosFrac,
                                       j, SamplesToDo, BufferSize);
                else if(Bytes == 1)
                    Mix_ALubyte_lerp8(Source, Device, Channels,
                                      SrcData, &DataPosInt, &DataPosFrac,
                                      j, SamplesToDo, BufferSize);
                break;
            case COSINE_RESAMPLER:
                if(Bytes == 4)
                    Mix_ALfloat_cos_lerp32(Source, Device, Channels,
                                           SrcData, &DataPosInt, &DataPosFrac,
                                           j, SamplesToDo, BufferSize);
                else if(Bytes == 2)
                    Mix_ALshort_cos_lerp16(Source, Device, Channels,
                                           SrcData, &DataPosInt, &DataPosFrac,
                                           j, SamplesToDo, BufferSize);
                else if(Bytes == 1)
                    Mix_ALubyte_cos_lerp8(Source, Device, Channels,
                                          SrcData, &DataPosInt, &DataPosFrac,
                                          j, SamplesToDo, BufferSize);
                break;
            case RESAMPLER_MIN:
            case RESAMPLER_MAX:
            break;
        }
        j += BufferSize;

        /* Handle looping sources */
        while(1)
        {
            const ALbuffer *ALBuffer;
            ALuint DataSize = 0;
            ALuint LoopStart = 0;
            ALuint LoopEnd = 0;

            if((ALBuffer=BufferListItem->buffer) != NULL)
            {
                DataSize = ALBuffer->size / FrameSize;
                if(DataSize > DataPosInt)
                    break;
                LoopStart = ALBuffer->LoopStart;
                LoopEnd = ALBuffer->LoopEnd;
            }

            if(BufferListItem->next)
            {
                BufferListItem = BufferListItem->next;
                BuffersPlayed++;
            }
            else if(Looping)
            {
                BufferListItem = Source->queue;
                BuffersPlayed = 0;
                if(Source->lSourceType == AL_STATIC)
                {
                    DataPosInt = ((DataPosInt-LoopStart)%(LoopEnd-LoopStart)) + LoopStart;
                    break;
                }
            }
            else
            {
                State = AL_STOPPED;
                BufferListItem = Source->queue;
                BuffersPlayed = Source->BuffersInQueue;
                DataPosInt = 0;
                DataPosFrac = 0;
                break;
            }

            DataPosInt -= DataSize;
        }
    } while(State == AL_PLAYING && j < SamplesToDo);

    /* Update source info */
    Source->state             = State;
    Source->BuffersPlayed     = BuffersPlayed;
    Source->position          = DataPosInt;
    Source->position_fraction = DataPosFrac;
    Source->Buffer            = BufferListItem->buffer;
}

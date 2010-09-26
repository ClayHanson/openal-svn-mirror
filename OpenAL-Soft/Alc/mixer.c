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


static __inline ALfloat aluF2F(ALfloat Value)
{
    return Value;
}

static __inline ALshort aluF2S(ALfloat Value)
{
    ALint i;

    if(Value <= -1.0f) i = -32768;
    else if(Value >= 1.0f) i = 32767;
    else i = (ALint)(Value*32767.5f - 0.5f);

    return ((ALshort)i);
}

static __inline ALubyte aluF2UB(ALfloat Value)
{
    ALshort i = aluF2S(Value);
    return (i>>8)+128;
}


static __inline ALfloat point32(ALfloat val1, ALfloat val2, ALint frac)
{
    return val1;
    (void)val2;
    (void)frac;
}
static __inline ALfloat lerp32(ALfloat val1, ALfloat val2, ALint frac)
{
    return val1 + ((val2-val1)*(frac * (1.0f/(1<<FRACTIONBITS))));
}
static __inline ALfloat cos_lerp32(ALfloat val1, ALfloat val2, ALint frac)
{
    ALfloat mult = (1.0f-cos(frac * (1.0f/(1<<FRACTIONBITS)) * M_PI)) * 0.5f;
    return val1 + ((val2-val1)*mult);
}

static __inline ALfloat point16(ALfloat val1, ALfloat val2, ALint frac)
{
    return (val1+0.5f) / 32767.5f;
    (void)val2;
    (void)frac;
}
static __inline ALfloat lerp16(ALfloat val1, ALfloat val2, ALint frac)
{
    val1 += ((val2-val1)*(frac * (1.0f/(1<<FRACTIONBITS))));
    return (val1+0.5f) / 32767.5f;
}
static __inline ALfloat cos_lerp16(ALfloat val1, ALfloat val2, ALint frac)
{
    ALfloat mult = (1.0f-cos(frac * (1.0f/(1<<FRACTIONBITS)) * M_PI)) * 0.5f;
    val1 += ((val2-val1)*mult);
    return (val1+0.5f) / 32767.5f;
}


#define DO_MIX_MONO(S,sampler) do {                                           \
    if(j == 0)                                                                \
    {                                                                         \
        value = sampler##S(Data.p##S[DataPosInt], Data.p##S[DataPosInt+1],    \
                           DataPosFrac);                                      \
                                                                              \
        outsamp = lpFilter4PC(DryFilter, 0, value);                           \
        ClickRemoval[FRONT_LEFT]   -= outsamp*DrySend[FRONT_LEFT];            \
        ClickRemoval[FRONT_RIGHT]  -= outsamp*DrySend[FRONT_RIGHT];           \
        ClickRemoval[SIDE_LEFT]    -= outsamp*DrySend[SIDE_LEFT];             \
        ClickRemoval[SIDE_RIGHT]   -= outsamp*DrySend[SIDE_RIGHT];            \
        ClickRemoval[BACK_LEFT]    -= outsamp*DrySend[BACK_LEFT];             \
        ClickRemoval[BACK_RIGHT]   -= outsamp*DrySend[BACK_RIGHT];            \
        ClickRemoval[FRONT_CENTER] -= outsamp*DrySend[FRONT_CENTER];          \
        ClickRemoval[BACK_CENTER]  -= outsamp*DrySend[BACK_CENTER];           \
                                                                              \
        for(out = 0;out < MAX_SENDS;out++)                                    \
        {                                                                     \
            outsamp = lpFilter2PC(WetFilter[out], 0, value);                  \
            WetClickRemoval[out][0] -= outsamp*WetSend[out];                  \
        }                                                                     \
    }                                                                         \
    while(BufferSize--)                                                       \
    {                                                                         \
        /* First order interpolator */                                        \
        value = sampler##S(Data.p##S[DataPosInt], Data.p##S[DataPosInt+1],    \
                           DataPosFrac);                                      \
                                                                              \
        /* Direct path final mix buffer and panning */                        \
        outsamp = lpFilter4P(DryFilter, 0, value);                            \
        DryBuffer[j][FRONT_LEFT]   += outsamp*DrySend[FRONT_LEFT];            \
        DryBuffer[j][FRONT_RIGHT]  += outsamp*DrySend[FRONT_RIGHT];           \
        DryBuffer[j][SIDE_LEFT]    += outsamp*DrySend[SIDE_LEFT];             \
        DryBuffer[j][SIDE_RIGHT]   += outsamp*DrySend[SIDE_RIGHT];            \
        DryBuffer[j][BACK_LEFT]    += outsamp*DrySend[BACK_LEFT];             \
        DryBuffer[j][BACK_RIGHT]   += outsamp*DrySend[BACK_RIGHT];            \
        DryBuffer[j][FRONT_CENTER] += outsamp*DrySend[FRONT_CENTER];          \
        DryBuffer[j][BACK_CENTER]  += outsamp*DrySend[BACK_CENTER];           \
                                                                              \
        /* Room path final mix buffer and panning */                          \
        for(out = 0;out < MAX_SENDS;out++)                                    \
        {                                                                     \
            outsamp = lpFilter2P(WetFilter[out], 0, value);                   \
            WetBuffer[out][j] += outsamp*WetSend[out];                        \
        }                                                                     \
                                                                              \
        DataPosFrac += increment;                                             \
        DataPosInt += DataPosFrac>>FRACTIONBITS;                              \
        DataPosFrac &= FRACTIONMASK;                                          \
        j++;                                                                  \
    }                                                                         \
    if(j == SamplesToDo)                                                      \
    {                                                                         \
        ALuint pos = ((DataPosInt < DataSize) ? DataPosInt : (DataPosInt-1)); \
        ALuint frac = ((DataPosInt < DataSize) ? DataPosFrac :                \
                       ((DataPosFrac-increment)&FRACTIONMASK));               \
        value = sampler##S(Data.p##S[pos], Data.p##S[pos+1], frac);           \
                                                                              \
        outsamp = lpFilter4PC(DryFilter, 0, value);                           \
        PendingClicks[FRONT_LEFT]   += outsamp*DrySend[FRONT_LEFT];           \
        PendingClicks[FRONT_RIGHT]  += outsamp*DrySend[FRONT_RIGHT];          \
        PendingClicks[SIDE_LEFT]    += outsamp*DrySend[SIDE_LEFT];            \
        PendingClicks[SIDE_RIGHT]   += outsamp*DrySend[SIDE_RIGHT];           \
        PendingClicks[BACK_LEFT]    += outsamp*DrySend[BACK_LEFT];            \
        PendingClicks[BACK_RIGHT]   += outsamp*DrySend[BACK_RIGHT];           \
        PendingClicks[FRONT_CENTER] += outsamp*DrySend[FRONT_CENTER];         \
        PendingClicks[BACK_CENTER]  += outsamp*DrySend[BACK_CENTER];          \
                                                                              \
        for(out = 0;out < MAX_SENDS;out++)                                    \
        {                                                                     \
            outsamp = lpFilter2PC(WetFilter[out], 0, value);                  \
            WetPendingClicks[out][0] += outsamp*WetSend[out];                 \
        }                                                                     \
    }                                                                         \
} while(0)

#define DO_MIX_STEREO(S,sampler) do {                                         \
    const ALfloat scaler = 1.0f/Channels;                                     \
    if(j == 0)                                                                \
    {                                                                         \
        for(i = 0;i < Channels;i++)                                           \
        {                                                                     \
            value = sampler##S(Data.p##S[DataPosInt*Channels + i],            \
                               Data.p##S[(DataPosInt+1)*Channels + i],        \
                               DataPosFrac);                                  \
                                                                              \
            outsamp = lpFilter2PC(DryFilter, chans[i]*2, value);              \
            ClickRemoval[chans[i+0]] -= outsamp*DrySend[chans[i+0]];          \
            ClickRemoval[chans[i+2]] -= outsamp*DrySend[chans[i+2]];          \
            ClickRemoval[chans[i+4]] -= outsamp*DrySend[chans[i+4]];          \
                                                                              \
            for(out = 0;out < MAX_SENDS;out++)                                \
            {                                                                 \
                outsamp = lpFilter1PC(WetFilter[out], chans[i], value);       \
                WetClickRemoval[out][0] -= outsamp*WetSend[out] * scaler;     \
            }                                                                 \
        }                                                                     \
    }                                                                         \
    while(BufferSize--)                                                       \
    {                                                                         \
        for(i = 0;i < Channels;i++)                                           \
        {                                                                     \
            value = sampler##S(Data.p##S[DataPosInt*Channels + i],            \
                               Data.p##S[(DataPosInt+1)*Channels + i],        \
                               DataPosFrac);                                  \
                                                                              \
            outsamp = lpFilter2P(DryFilter, chans[i]*2, value);               \
            DryBuffer[j][chans[i+0]] += outsamp*DrySend[chans[i+0]];          \
            DryBuffer[j][chans[i+2]] += outsamp*DrySend[chans[i+2]];          \
            DryBuffer[j][chans[i+4]] += outsamp*DrySend[chans[i+4]];          \
                                                                              \
            for(out = 0;out < MAX_SENDS;out++)                                \
            {                                                                 \
                outsamp = lpFilter1P(WetFilter[out], chans[i], value);        \
                WetBuffer[out][j] += outsamp*WetSend[out] * scaler;           \
            }                                                                 \
        }                                                                     \
                                                                              \
        DataPosFrac += increment;                                             \
        DataPosInt += DataPosFrac>>FRACTIONBITS;                              \
        DataPosFrac &= FRACTIONMASK;                                          \
        j++;                                                                  \
    }                                                                         \
    if(j == SamplesToDo)                                                      \
    {                                                                         \
        ALuint pos = ((DataPosInt < DataSize) ? DataPosInt : (DataPosInt-1)); \
        ALuint frac = ((DataPosInt < DataSize) ? DataPosFrac :                \
                       ((DataPosFrac-increment)&FRACTIONMASK));               \
        for(i = 0;i < Channels;i++)                                           \
        {                                                                     \
            value = sampler##S(Data.p##S[pos*Channels + i],                   \
                               Data.p##S[(pos+1)*Channels + i],               \
                               frac);                                         \
                                                                              \
            outsamp = lpFilter2PC(DryFilter, chans[i]*2, value);              \
            PendingClicks[chans[i+0]] += outsamp*DrySend[chans[i+0]];         \
            PendingClicks[chans[i+2]] += outsamp*DrySend[chans[i+2]];         \
            PendingClicks[chans[i+4]] += outsamp*DrySend[chans[i+4]];         \
                                                                              \
            for(out = 0;out < MAX_SENDS;out++)                                \
            {                                                                 \
                outsamp = lpFilter1PC(WetFilter[out], chans[i], value);       \
                WetPendingClicks[out][0] += outsamp*WetSend[out] * scaler;    \
            }                                                                 \
        }                                                                     \
    }                                                                         \
} while(0)

#define DO_MIX_MC(S,sampler) do {                                             \
    const ALfloat scaler = 1.0f/Channels;                                     \
    if(j == 0)                                                                \
    {                                                                         \
        for(i = 0;i < Channels;i++)                                           \
        {                                                                     \
            value = sampler##S(Data.p##S[DataPosInt*Channels + i],            \
                               Data.p##S[(DataPosInt+1)*Channels + i],        \
                               DataPosFrac);                                  \
                                                                              \
            outsamp = lpFilter2PC(DryFilter, chans[i]*2, value);              \
            ClickRemoval[chans[i]] -= outsamp*DrySend[chans[i]];              \
                                                                              \
            for(out = 0;out < MAX_SENDS;out++)                                \
            {                                                                 \
                outsamp = lpFilter1PC(WetFilter[out], chans[out], value) * scaler;\
                WetClickRemoval[out][0] -= outsamp*WetSend[out];              \
            }                                                                 \
        }                                                                     \
    }                                                                         \
    while(BufferSize--)                                                       \
    {                                                                         \
        for(i = 0;i < Channels;i++)                                           \
        {                                                                     \
            value = sampler##S(Data.p##S[DataPosInt*Channels + i],            \
                               Data.p##S[(DataPosInt+1)*Channels + i],        \
                               DataPosFrac);                                  \
                                                                              \
            outsamp = lpFilter2P(DryFilter, chans[i]*2, value);               \
            DryBuffer[j][chans[i]] += outsamp*DrySend[chans[i]];              \
                                                                              \
            for(out = 0;out < MAX_SENDS;out++)                                \
            {                                                                 \
                outsamp = lpFilter1P(WetFilter[out], chans[i], value);        \
                WetBuffer[out][j] += outsamp*WetSend[out]*scaler;             \
            }                                                                 \
        }                                                                     \
                                                                              \
        DataPosFrac += increment;                                             \
        DataPosInt += DataPosFrac>>FRACTIONBITS;                              \
        DataPosFrac &= FRACTIONMASK;                                          \
        j++;                                                                  \
    }                                                                         \
    if(j == SamplesToDo)                                                      \
    {                                                                         \
        ALuint pos = ((DataPosInt < DataSize) ? DataPosInt : (DataPosInt-1)); \
        ALuint frac = ((DataPosInt < DataSize) ? DataPosFrac :                \
                       ((DataPosFrac-increment)&FRACTIONMASK));               \
        for(i = 0;i < Channels;i++)                                           \
        {                                                                     \
            value = sampler##S(Data.p##S[pos*Channels + i],                   \
                               Data.p##S[(pos+1)*Channels + i],               \
                               frac);                                         \
                                                                              \
            outsamp = lpFilter2PC(DryFilter, chans[i]*2, value);              \
            PendingClicks[chans[i]] += outsamp*DrySend[chans[i]];             \
                                                                              \
            for(out = 0;out < MAX_SENDS;out++)                                \
            {                                                                 \
                outsamp = lpFilter1PC(WetFilter[out], chans[out], value) * scaler;\
                WetPendingClicks[out][0] += outsamp*WetSend[out];             \
            }                                                                 \
        }                                                                     \
    }                                                                         \
} while(0)


#define MIX_MONO(S) do {                                                      \
    switch(Source->Resampler)                                                 \
    {                                                                         \
        case POINT_RESAMPLER:                                                 \
        DO_MIX_MONO(S,point); break;                                          \
        case LINEAR_RESAMPLER:                                                \
        DO_MIX_MONO(S,lerp); break;                                           \
        case COSINE_RESAMPLER:                                                \
        DO_MIX_MONO(S,cos_lerp); break;                                       \
        case RESAMPLER_MIN:                                                   \
        case RESAMPLER_MAX:                                                   \
        break;                                                                \
    }                                                                         \
} while(0)

#define MIX_STEREO(S) do {                                                    \
    const int chans[] = {                                                     \
        FRONT_LEFT, FRONT_RIGHT,                                              \
        SIDE_LEFT, SIDE_RIGHT,                                                \
        BACK_LEFT, BACK_RIGHT                                                 \
    };                                                                        \
                                                                              \
    switch(Source->Resampler)                                                 \
    {                                                                         \
        case POINT_RESAMPLER:                                                 \
        DO_MIX_STEREO(S,point); break;                                        \
        case LINEAR_RESAMPLER:                                                \
        DO_MIX_STEREO(S,lerp); break;                                         \
        case COSINE_RESAMPLER:                                                \
        DO_MIX_STEREO(S,cos_lerp); break;                                     \
        case RESAMPLER_MIN:                                                   \
        case RESAMPLER_MAX:                                                   \
        break;                                                                \
    }                                                                         \
} while(0)

#define MIX_MC(S,...) do {                                                    \
    const int chans[] = { __VA_ARGS__ };                                      \
                                                                              \
    switch(Source->Resampler)                                                 \
    {                                                                         \
        case POINT_RESAMPLER:                                                 \
        DO_MIX_MC(S,point); break;                                            \
        case LINEAR_RESAMPLER:                                                \
        DO_MIX_MC(S,lerp); break;                                             \
        case COSINE_RESAMPLER:                                                \
        DO_MIX_MC(S,cos_lerp); break;                                         \
        case RESAMPLER_MIN:                                                   \
        case RESAMPLER_MAX:                                                   \
        break;                                                                \
    }                                                                         \
} while(0)


#define MIX(S) do {                                                           \
    if(Channels == 1) /* Mono */                                              \
        MIX_MONO(S);                                                          \
    else if(Channels == 2) /* Stereo */                                       \
        MIX_STEREO(S);                                                        \
    else if(Channels == 4) /* Quad */                                         \
        MIX_MC(S, FRONT_LEFT, FRONT_RIGHT,                                    \
                  BACK_LEFT,  BACK_RIGHT);                                    \
    else if(Channels == 6) /* 5.1 */                                          \
        MIX_MC(S, FRONT_LEFT,   FRONT_RIGHT,                                  \
                  FRONT_CENTER, LFE,                                          \
                  BACK_LEFT,    BACK_RIGHT);                                  \
    else if(Channels == 7) /* 6.1 */                                          \
        MIX_MC(S, FRONT_LEFT,   FRONT_RIGHT,                                  \
                  FRONT_CENTER, LFE,                                          \
                  BACK_CENTER,                                                \
                  SIDE_LEFT,    SIDE_RIGHT);                                  \
    else if(Channels == 8) /* 7.1 */                                          \
        MIX_MC(S, FRONT_LEFT,   FRONT_RIGHT,                                  \
                  FRONT_CENTER, LFE,                                          \
                  BACK_LEFT,    BACK_RIGHT,                                   \
                  SIDE_LEFT,    SIDE_RIGHT);                                  \
} while(0)


static void MixSource(ALsource *Source, ALCcontext *Context,
                      ALfloat (*DryBuffer)[OUTPUTCHANNELS], ALuint SamplesToDo,
                      ALfloat *ClickRemoval, ALfloat *PendingClicks)
{
    static ALfloat DummyBuffer[BUFFERSIZE];
    static ALfloat DummyClickRemoval[OUTPUTCHANNELS];
    ALfloat *WetBuffer[MAX_SENDS];
    ALfloat DrySend[OUTPUTCHANNELS];
    ALfloat *WetClickRemoval[MAX_SENDS];
    ALfloat *WetPendingClicks[MAX_SENDS];
    ALuint i, j, out;
    ALfloat value, outsamp;
    ALbufferlistitem *BufferListItem;
    ALint64 DataSize64,DataPos64;
    FILTER *DryFilter, *WetFilter[MAX_SENDS];
    ALfloat WetSend[MAX_SENDS];
    ALint increment;
    ALuint DataPosInt, DataPosFrac;
    ALuint BuffersPlayed;
    ALboolean Looping;
    ALenum State;

    if(Source->NeedsUpdate)
    {
        ALsource_Update(Source, Context);
        Source->NeedsUpdate = AL_FALSE;
    }

    /* Get source info */
    State         = Source->state;
    BuffersPlayed = Source->BuffersPlayed;
    DataPosInt    = Source->position;
    DataPosFrac   = Source->position_fraction;
    Looping       = Source->bLooping;

    for(i = 0;i < OUTPUTCHANNELS;i++)
        DrySend[i] = Source->Params.DryGains[i];
    for(i = 0;i < MAX_SENDS;i++)
        WetSend[i] = Source->Params.WetGains[i];

    /* Get fixed point step */
    increment = Source->Params.Step;

    DryFilter = &Source->Params.iirFilter;
    for(i = 0;i < MAX_SENDS;i++)
    {
        WetFilter[i] = &Source->Params.Send[i].iirFilter;
        if(Source->Send[i].Slot)
        {
            WetBuffer[i] = Source->Send[i].Slot->WetBuffer;
            WetClickRemoval[i] = Source->Send[i].Slot->ClickRemoval;
            WetPendingClicks[i] = Source->Send[i].Slot->PendingClicks;
        }
        else
        {
            WetBuffer[i] = DummyBuffer;
            WetClickRemoval[i] = DummyClickRemoval;
            WetPendingClicks[i] = DummyClickRemoval;
        }
    }

    /* Get current buffer queue item */
    BufferListItem = Source->queue;
    for(i = 0;i < BuffersPlayed;i++)
        BufferListItem = BufferListItem->next;

    j = 0;
    do {
        const ALbuffer *ALBuffer;
        union {
            ALfloat *p32;
            ALshort *p16;
            ALubyte *p8;
        } Data = { NULL };
        ALuint DataSize = 0;
        ALuint LoopStart = 0;
        ALuint LoopEnd = 0;
        ALuint Channels, Bytes;
        ALuint BufferSize;

        /* Get buffer info */
        if((ALBuffer=BufferListItem->buffer) != NULL)
        {
            Data.p8   = ALBuffer->data;
            DataSize  = ALBuffer->size;
            DataSize /= aluFrameSizeFromFormat(ALBuffer->format);
            Channels  = aluChannelsFromFormat(ALBuffer->format);
            Bytes     = aluBytesFromFormat(ALBuffer->format);

            LoopStart = 0;
            LoopEnd   = DataSize;
            if(Looping && Source->lSourceType == AL_STATIC)
            {
                /* If current pos is beyond the loop range, do not loop */
                if(DataPosInt >= LoopEnd)
                    Looping = AL_FALSE;
                else
                {
                    LoopStart = ALBuffer->LoopStart;
                    LoopEnd   = ALBuffer->LoopEnd;
                }
            }
        }

        if(DataPosInt >= DataSize)
            goto skipmix;

        if(BufferListItem->next)
        {
            ALbuffer *NextBuf = BufferListItem->next->buffer;
            if(NextBuf && NextBuf->size)
            {
                ALint ulExtraSamples = BUFFER_PADDING*Channels*Bytes;
                ulExtraSamples = min(NextBuf->size, ulExtraSamples);
                memcpy(&Data.p8[DataSize*Channels*Bytes],
                       NextBuf->data, ulExtraSamples);
            }
        }
        else if(Looping)
        {
            ALbuffer *NextBuf = Source->queue->buffer;
            if(NextBuf && NextBuf->size)
            {
                ALint ulExtraSamples = BUFFER_PADDING*Channels*Bytes;
                ulExtraSamples = min(NextBuf->size, ulExtraSamples);
                memcpy(&Data.p8[DataSize*Channels*Bytes],
                       &((ALubyte*)NextBuf->data)[LoopStart*Channels*Bytes],
                       ulExtraSamples);
            }
        }
        else
            memset(&Data.p8[DataSize*Channels*Bytes], 0, (BUFFER_PADDING*Channels*Bytes));

        /* Figure out how many samples we can mix. */
        DataSize64 = LoopEnd;
        DataSize64 <<= FRACTIONBITS;
        DataPos64 = DataPosInt;
        DataPos64 <<= FRACTIONBITS;
        DataPos64 += DataPosFrac;
        BufferSize = (ALuint)((DataSize64-DataPos64+(increment-1)) / increment);

        BufferSize = min(BufferSize, (SamplesToDo-j));

        if(Bytes == 4) /* 32-bit float */
            MIX(32);
        else if(Bytes == 2) /* signed 16-bit */
            MIX(16);

    skipmix:
        /* Handle looping sources */
        if(DataPosInt >= LoopEnd)
        {
            if(BuffersPlayed < (Source->BuffersInQueue-1))
            {
                BufferListItem = BufferListItem->next;
                BuffersPlayed++;
                DataPosInt -= DataSize;
            }
            else if(Looping)
            {
                BufferListItem = Source->queue;
                BuffersPlayed = 0;
                if(Source->lSourceType == AL_STATIC)
                    DataPosInt = ((DataPosInt-LoopStart)%(LoopEnd-LoopStart)) + LoopStart;
                else
                    DataPosInt -= DataSize;
            }
            else
            {
                State = AL_STOPPED;
                BufferListItem = Source->queue;
                BuffersPlayed = Source->BuffersInQueue;
                DataPosInt = 0;
                DataPosFrac = 0;
            }
        }
    } while(State == AL_PLAYING && j < SamplesToDo);

    /* Update source info */
    Source->state             = State;
    Source->BuffersPlayed     = BuffersPlayed;
    Source->position          = DataPosInt;
    Source->position_fraction = DataPosFrac;
    Source->Buffer            = BufferListItem->buffer;
}

#undef DO_MIX_MC
#undef DO_MIX_STEREO
#undef DO_MIX_MONO

ALvoid aluMixData(ALCdevice *device, ALvoid *buffer, ALsizei size)
{
    ALfloat (*DryBuffer)[OUTPUTCHANNELS];
    ALfloat (*Matrix)[OUTPUTCHANNELS];
    ALfloat *ClickRemoval;
    const ALuint *ChanMap;
    ALuint SamplesToDo;
    ALeffectslot *ALEffectSlot;
    ALCcontext **ctx, **ctx_end;
    ALsource **src, **src_end;
    ALfloat samp;
    int fpuState;
    ALuint i, j, c;
    ALsizei e;

#if defined(HAVE_FESETROUND)
    fpuState = fegetround();
    fesetround(FE_TOWARDZERO);
#elif defined(HAVE__CONTROLFP)
    fpuState = _controlfp(0, 0);
    _controlfp(_RC_CHOP, _MCW_RC);
#else
    (void)fpuState;
#endif

    DryBuffer = device->DryBuffer;
    while(size > 0)
    {
        /* Setup variables */
        SamplesToDo = min(size, BUFFERSIZE);

        /* Clear mixing buffer */
        memset(DryBuffer, 0, SamplesToDo*OUTPUTCHANNELS*sizeof(ALfloat));

        SuspendContext(NULL);
        ctx = device->Contexts;
        ctx_end = ctx + device->NumContexts;
        while(ctx != ctx_end)
        {
            SuspendContext(*ctx);

            src = (*ctx)->ActiveSources;
            src_end = src + (*ctx)->ActiveSourceCount;
            while(src != src_end)
            {
                if((*src)->state != AL_PLAYING)
                {
                    --((*ctx)->ActiveSourceCount);
                    *src = *(--src_end);
                    continue;
                }
                MixSource(*src, *ctx, DryBuffer, SamplesToDo,
                          device->ClickRemoval, device->PendingClicks);
                src++;
            }

            /* effect slot processing */
            for(e = 0;e < (*ctx)->EffectSlotMap.size;e++)
            {
                ALEffectSlot = (*ctx)->EffectSlotMap.array[e].value;

                ClickRemoval = ALEffectSlot->ClickRemoval;
                for(i = 0;i < SamplesToDo;i++)
                {
                    ClickRemoval[0] -= ClickRemoval[0] / 256.0f;
                    ALEffectSlot->WetBuffer[i] += ClickRemoval[0];
                }
                for(i = 0;i < 1;i++)
                {
                    ALEffectSlot->ClickRemoval[i] += ALEffectSlot->PendingClicks[i];
                    ALEffectSlot->PendingClicks[i] = 0.0f;
                }

                ALEffect_Process(ALEffectSlot->EffectState, ALEffectSlot, SamplesToDo, ALEffectSlot->WetBuffer, DryBuffer);

                for(i = 0;i < SamplesToDo;i++)
                    ALEffectSlot->WetBuffer[i] = 0.0f;
            }

            ProcessContext(*ctx);
            ctx++;
        }
        device->SamplesPlayed += SamplesToDo;
        ProcessContext(NULL);

        //Post processing loop
        ClickRemoval = device->ClickRemoval;
        for(i = 0;i < SamplesToDo;i++)
        {
            for(c = 0;c < OUTPUTCHANNELS;c++)
            {
                ClickRemoval[c] -= ClickRemoval[c] / 256.0f;
                DryBuffer[i][c] += ClickRemoval[c];
            }
        }
        for(i = 0;i < OUTPUTCHANNELS;i++)
        {
            device->ClickRemoval[i] += device->PendingClicks[i];
            device->PendingClicks[i] = 0.0f;
        }

        ChanMap = device->DevChannels;
        Matrix = device->ChannelMatrix;
        switch(device->Format)
        {
#define CHECK_WRITE_FORMAT(bits, type, func)                                  \
        case AL_FORMAT_MONO##bits:                                            \
            for(i = 0;i < SamplesToDo;i++)                                    \
            {                                                                 \
                samp = 0.0f;                                                  \
                for(c = 0;c < OUTPUTCHANNELS;c++)                             \
                    samp += DryBuffer[i][c] * Matrix[c][FRONT_CENTER];        \
                ((type*)buffer)[ChanMap[FRONT_CENTER]] = (func)(samp);        \
                buffer = ((type*)buffer) + 1;                                 \
            }                                                                 \
            break;                                                            \
        case AL_FORMAT_STEREO##bits:                                          \
            if(device->Bs2b)                                                  \
            {                                                                 \
                for(i = 0;i < SamplesToDo;i++)                                \
                {                                                             \
                    float samples[2] = { 0.0f, 0.0f };                        \
                    for(c = 0;c < OUTPUTCHANNELS;c++)                         \
                    {                                                         \
                        samples[0] += DryBuffer[i][c]*Matrix[c][FRONT_LEFT];  \
                        samples[1] += DryBuffer[i][c]*Matrix[c][FRONT_RIGHT]; \
                    }                                                         \
                    bs2b_cross_feed(device->Bs2b, samples);                   \
                    ((type*)buffer)[ChanMap[FRONT_LEFT]] = (func)(samples[0]);\
                    ((type*)buffer)[ChanMap[FRONT_RIGHT]]= (func)(samples[1]);\
                    buffer = ((type*)buffer) + 2;                             \
                }                                                             \
            }                                                                 \
            else                                                              \
            {                                                                 \
                for(i = 0;i < SamplesToDo;i++)                                \
                {                                                             \
                    static const Channel chans[] = {                          \
                        FRONT_LEFT, FRONT_RIGHT                               \
                    };                                                        \
                    for(j = 0;j < 2;j++)                                      \
                    {                                                         \
                        samp = 0.0f;                                          \
                        for(c = 0;c < OUTPUTCHANNELS;c++)                     \
                            samp += DryBuffer[i][c] * Matrix[c][chans[j]];    \
                        ((type*)buffer)[ChanMap[chans[j]]] = (func)(samp);    \
                    }                                                         \
                    buffer = ((type*)buffer) + 2;                             \
                }                                                             \
            }                                                                 \
            break;                                                            \
        case AL_FORMAT_QUAD##bits:                                            \
            for(i = 0;i < SamplesToDo;i++)                                    \
            {                                                                 \
                static const Channel chans[] = {                              \
                    FRONT_LEFT, FRONT_RIGHT,                                  \
                    BACK_LEFT,  BACK_RIGHT,                                   \
                };                                                            \
                for(j = 0;j < 4;j++)                                          \
                {                                                             \
                    samp = 0.0f;                                              \
                    for(c = 0;c < OUTPUTCHANNELS;c++)                         \
                        samp += DryBuffer[i][c] * Matrix[c][chans[j]];        \
                    ((type*)buffer)[ChanMap[chans[j]]] = (func)(samp);        \
                }                                                             \
                buffer = ((type*)buffer) + 4;                                 \
            }                                                                 \
            break;                                                            \
        case AL_FORMAT_51CHN##bits:                                           \
            for(i = 0;i < SamplesToDo;i++)                                    \
            {                                                                 \
                static const Channel chans[] = {                              \
                    FRONT_LEFT, FRONT_RIGHT,                                  \
                    FRONT_CENTER, LFE,                                        \
                    BACK_LEFT,  BACK_RIGHT,                                   \
                };                                                            \
                for(j = 0;j < 6;j++)                                          \
                {                                                             \
                    samp = 0.0f;                                              \
                    for(c = 0;c < OUTPUTCHANNELS;c++)                         \
                        samp += DryBuffer[i][c] * Matrix[c][chans[j]];        \
                    ((type*)buffer)[ChanMap[chans[j]]] = (func)(samp);        \
                }                                                             \
                buffer = ((type*)buffer) + 6;                                 \
            }                                                                 \
            break;                                                            \
        case AL_FORMAT_61CHN##bits:                                           \
            for(i = 0;i < SamplesToDo;i++)                                    \
            {                                                                 \
                static const Channel chans[] = {                              \
                    FRONT_LEFT, FRONT_RIGHT,                                  \
                    FRONT_CENTER, LFE, BACK_CENTER,                           \
                    SIDE_LEFT,  SIDE_RIGHT,                                   \
                };                                                            \
                for(j = 0;j < 7;j++)                                          \
                {                                                             \
                    samp = 0.0f;                                              \
                    for(c = 0;c < OUTPUTCHANNELS;c++)                         \
                        samp += DryBuffer[i][c] * Matrix[c][chans[j]];        \
                    ((type*)buffer)[ChanMap[chans[j]]] = (func)(samp);        \
                }                                                             \
                buffer = ((type*)buffer) + 7;                                 \
            }                                                                 \
            break;                                                            \
        case AL_FORMAT_71CHN##bits:                                           \
            for(i = 0;i < SamplesToDo;i++)                                    \
            {                                                                 \
                static const Channel chans[] = {                              \
                    FRONT_LEFT, FRONT_RIGHT,                                  \
                    FRONT_CENTER, LFE,                                        \
                    BACK_LEFT,  BACK_RIGHT,                                   \
                    SIDE_LEFT,  SIDE_RIGHT                                    \
                };                                                            \
                for(j = 0;j < 8;j++)                                          \
                {                                                             \
                    samp = 0.0f;                                              \
                    for(c = 0;c < OUTPUTCHANNELS;c++)                         \
                        samp += DryBuffer[i][c] * Matrix[c][chans[j]];        \
                    ((type*)buffer)[ChanMap[chans[j]]] = (func)(samp);        \
                }                                                             \
                buffer = ((type*)buffer) + 8;                                 \
            }                                                                 \
            break;

#define AL_FORMAT_MONO32 AL_FORMAT_MONO_FLOAT32
#define AL_FORMAT_STEREO32 AL_FORMAT_STEREO_FLOAT32
            CHECK_WRITE_FORMAT(8,  ALubyte, aluF2UB)
            CHECK_WRITE_FORMAT(16, ALshort, aluF2S)
            CHECK_WRITE_FORMAT(32, ALfloat, aluF2F)
#undef AL_FORMAT_STEREO32
#undef AL_FORMAT_MONO32
#undef CHECK_WRITE_FORMAT

            default:
                break;
        }

        size -= SamplesToDo;
    }

#if defined(HAVE_FESETROUND)
    fesetround(fpuState);
#elif defined(HAVE__CONTROLFP)
    _controlfp(fpuState, 0xfffff);
#endif
}

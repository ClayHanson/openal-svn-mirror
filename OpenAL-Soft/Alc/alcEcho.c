/**
 * OpenAL cross platform audio library
 * Copyright (C) 2009 by Chris Robinson.
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

#include "alMain.h"
#include "alFilter.h"
#include "alAuxEffectSlot.h"
#include "alError.h"
#include "alu.h"

// Just a soft maximum. Being higher will cause EchoUpdate to reallocate the
// sample buffer which may cause an abort if realloc fails
#define MAX_ECHO_FREQ  192000

typedef struct ALechoState {
    // Must be first in all effects!
    ALeffectState state;

    ALfloat *SampleBuffer;
    ALuint BufferLength;

    // The echo is two tap. The delay is the number of samples from before the
    // current offset
    struct {
        ALuint delay;
    } Tap[2];
    ALuint Offset;
    // The LR gains for the first tap. The second tap uses the reverse
    ALfloat GainL;
    ALfloat GainR;

    ALfloat FeedGain;

    FILTER iirFilter;
    ALfloat history[2];
} ALechoState;

// Find the next power of 2.  Actually, this will return the input value if
// it is already a power of 2.
static ALuint NextPowerOf2(ALuint value)
{
    ALuint powerOf2 = 1;

    if(value)
    {
        value--;
        while(value)
        {
            value >>= 1;
            powerOf2 <<= 1;
        }
    }
    return powerOf2;
}

ALvoid EchoDestroy(ALeffectState *effect)
{
    ALechoState *state = (ALechoState*)effect;
    if(state)
    {
        free(state->SampleBuffer);
        state->SampleBuffer = NULL;
        free(state);
    }
}

ALvoid EchoUpdate(ALeffectState *effect, ALCcontext *Context, const ALeffect *Effect)
{
    ALechoState *state = (ALechoState*)effect;
    ALfloat lrpan, cw, a, g;
    ALuint maxlen;

    maxlen  = (ALuint)(AL_ECHO_MAX_DELAY * Context->Frequency);
    maxlen += (ALuint)(AL_ECHO_MAX_LRDELAY * Context->Frequency);
    maxlen  = NextPowerOf2(maxlen+1);

    if(maxlen > state->BufferLength)
    {
        void *temp;
        ALuint i;

        state->BufferLength = maxlen;
        temp = realloc(state->SampleBuffer, state->BufferLength * sizeof(ALfloat));
        if(!temp)
        {
            AL_PRINT("Failed reallocation!");
            abort();
        }
        for(i = 0;i < state->BufferLength;i++)
            state->SampleBuffer[i] = 0.0f;
    }

    state->Tap[0].delay = (ALuint)(Effect->Echo.Delay * Context->Frequency);
    state->Tap[1].delay = (ALuint)(Effect->Echo.LRDelay * Context->Frequency);
    state->Tap[1].delay += state->Tap[0].delay;

    lrpan = Effect->Echo.Spread*0.5f + 0.5f;
    state->GainL = aluSqrt(     lrpan);
    state->GainR = aluSqrt(1.0f-lrpan);

    state->FeedGain = Effect->Echo.Feedback;

    cw = cos(2.0*M_PI * LOWPASSFREQCUTOFF / Context->Frequency);
    g = 1.0f - Effect->Echo.Damping;
    a = 0.0f;
    if(g < 0.9999f) // 1-epsilon
        a = (1 - g*cw - aluSqrt(2*g*(1-cw) - g*g*(1 - cw*cw))) / (1 - g);
    state->iirFilter.coeff = a;
}

ALvoid EchoProcess(ALeffectState *effect, const ALeffectslot *Slot, ALuint SamplesToDo, const ALfloat *SamplesIn, ALfloat (*SamplesOut)[OUTPUTCHANNELS])
{
    ALechoState *state = (ALechoState*)effect;
    const ALuint mask = state->BufferLength-1;
    const ALuint tap1 = state->Tap[0].delay;
    const ALuint tap2 = state->Tap[1].delay;
    ALuint offset = state->Offset;
    const ALfloat gain = Slot->Gain;
    ALfloat samp[2], smp;
    ALuint i;

    for(i = 0;i < SamplesToDo;i++,offset++)
    {
        // Sample first tap
        smp = state->SampleBuffer[(offset-tap1) & mask];
        samp[0] = smp * state->GainL;
        samp[1] = smp * state->GainR;
        // Sample second tap. Reverse LR panning
        smp = state->SampleBuffer[(offset-tap2) & mask];
        samp[0] += smp * state->GainR;
        samp[1] += smp * state->GainL;

        // Apply damping and feedback gain to the second tap, and mix in the
        // new sample
        smp = lpFilter2P(&state->iirFilter, 0, smp+SamplesIn[i]);
        state->SampleBuffer[offset&mask] = smp * state->FeedGain;

        // Apply slot gain
        samp[0] *= gain;
        samp[1] *= gain;

        SamplesOut[i][FRONT_LEFT]  += samp[0];
        SamplesOut[i][FRONT_RIGHT] += samp[1];
        SamplesOut[i][SIDE_LEFT]   += samp[0];
        SamplesOut[i][SIDE_RIGHT]  += samp[1];
        SamplesOut[i][BACK_LEFT]   += samp[0];
        SamplesOut[i][BACK_RIGHT]  += samp[1];
    }
    state->Offset = offset;
}

ALeffectState *EchoCreate(void)
{
    ALechoState *state;
    ALuint i, maxlen;

    state = malloc(sizeof(*state));
    if(!state)
    {
        alSetError(AL_OUT_OF_MEMORY);
        return NULL;
    }

    state->state.Destroy = EchoDestroy;
    state->state.Update = EchoUpdate;
    state->state.Process = EchoProcess;

    maxlen  = (ALuint)(AL_ECHO_MAX_DELAY * MAX_ECHO_FREQ);
    maxlen += (ALuint)(AL_ECHO_MAX_LRDELAY * MAX_ECHO_FREQ);

    // Use the next power of 2 for the buffer length, so the tap offsets can be
    // wrapped using a mask instead of a modulo
    state->BufferLength = NextPowerOf2(maxlen+1);
    state->SampleBuffer = malloc(state->BufferLength * sizeof(ALfloat));
    if(!state->SampleBuffer)
    {
        free(state);
        alSetError(AL_OUT_OF_MEMORY);
        return NULL;
    }

    for(i = 0;i < state->BufferLength;i++)
        state->SampleBuffer[i] = 0.0f;

    state->Tap[0].delay = 0;
    state->Tap[1].delay = 0;
    state->Offset = 0;
    state->GainL = 0.0f;
    state->GainR = 0.0f;

    for(i = 0;i < 2;i++)
        state->iirFilter.history[i] = 0.0f;
    state->iirFilter.coeff = 0.0f;

    return &state->state;
}

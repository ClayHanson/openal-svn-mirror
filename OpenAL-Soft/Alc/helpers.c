/**
 * OpenAL cross platform audio library
 * Copyright (C) 2011 by authors.
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

#include <stdlib.h>
#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#endif

#include "alMain.h"

#ifdef _WIN32
void pthread_once(pthread_once_t *once, void (*callback)(void))
{
    LONG ret;
    while((ret=InterlockedExchange(once, 1)) == 1)
        Sleep(0);
    if(ret == 0)
        callback();
    InterlockedExchange(once, 2);
}

void *LoadLib(const char *name)
{ return LoadLibraryA(name); }
void CloseLib(void *handle)
{ FreeLibrary((HANDLE)handle); }
void *GetSymbol(void *handle, const char *name)
{
    void *ret;

    ret = (void*)GetProcAddress((HANDLE)handle, name);
    if(ret == NULL)
        ERR("Failed to load %s\n", name);
    return ret;
}

#else

void InitializeCriticalSection(CRITICAL_SECTION *cs)
{
    pthread_mutexattr_t attrib;
    int ret;

    ret = pthread_mutexattr_init(&attrib);
    assert(ret == 0);

    ret = pthread_mutexattr_settype(&attrib, PTHREAD_MUTEX_RECURSIVE);
#ifdef HAVE_PTHREAD_NP_H
    if(ret != 0)
        ret = pthread_mutexattr_setkind_np(&attrib, PTHREAD_MUTEX_RECURSIVE);
#endif
    assert(ret == 0);
    ret = pthread_mutex_init(cs, &attrib);
    assert(ret == 0);

    pthread_mutexattr_destroy(&attrib);
}
void DeleteCriticalSection(CRITICAL_SECTION *cs)
{
    int ret;
    ret = pthread_mutex_destroy(cs);
    assert(ret == 0);
}
void EnterCriticalSection(CRITICAL_SECTION *cs)
{
    int ret;
    ret = pthread_mutex_lock(cs);
    assert(ret == 0);
}
void LeaveCriticalSection(CRITICAL_SECTION *cs)
{
    int ret;
    ret = pthread_mutex_unlock(cs);
    assert(ret == 0);
}

/* NOTE: This wrapper isn't quite accurate as it returns an ALuint, as opposed
 * to the expected DWORD. Both are defined as unsigned 32-bit types, however.
 * Additionally, Win32 is supposed to measure the time since Windows started,
 * as opposed to the actual time. */
ALuint timeGetTime(void)
{
#if _POSIX_TIMERS > 0
    struct timespec ts;
    int ret = -1;

#if defined(_POSIX_MONOTONIC_CLOCK) && (_POSIX_MONOTONIC_CLOCK >= 0)
#if _POSIX_MONOTONIC_CLOCK == 0
    static int hasmono = 0;
    if(hasmono > 0 || (hasmono == 0 &&
                       (hasmono=sysconf(_SC_MONOTONIC_CLOCK)) > 0))
#endif
        ret = clock_gettime(CLOCK_MONOTONIC, &ts);
#endif
    if(ret != 0)
        ret = clock_gettime(CLOCK_REALTIME, &ts);
    assert(ret == 0);

    return ts.tv_nsec/1000000 + ts.tv_sec*1000;
#else
    struct timeval tv;
    int ret;

    ret = gettimeofday(&tv, NULL);
    assert(ret == 0);

    return tv.tv_usec/1000 + tv.tv_sec*1000;
#endif
}

#ifdef HAVE_DLFCN_H

void *LoadLib(const char *name)
{
    const char *err;
    void *handle;

    dlerror();
    handle = dlopen(name, RTLD_NOW);
    if((err=dlerror()) != NULL)
        handle = NULL;
    return handle;
}
void CloseLib(void *handle)
{ dlclose(handle); }
void *GetSymbol(void *handle, const char *name)
{
    const char *err;
    void *sym;

    dlerror();
    sym = dlsym(handle, name);
    if((err=dlerror()) != NULL)
    {
        ERR("Failed to load %s: %s\n", name, err);
        sym = NULL;
    }
    return sym;
}

#endif
#endif


void al_print(const char *fname, unsigned int line, const char *fmt, ...)
{
    const char *fn;
    char str[256];
    int i;

    fn = strrchr(fname, '/');
    if(!fn) fn = strrchr(fname, '\\');
    if(!fn) fn = fname;
    else fn += 1;

    i = snprintf(str, sizeof(str), "AL lib: %s:%d: ", fn, line);
    if(i < (int)sizeof(str) && i > 0)
    {
        va_list ap;
        va_start(ap, fmt);
        vsnprintf(str+i, sizeof(str)-i, fmt, ap);
        va_end(ap);
    }
    str[sizeof(str)-1] = 0;

    fprintf(LogFile, "%s", str);
    fflush(LogFile);
}


void SetRTPriority(void)
{
    ALboolean failed;

#ifdef _WIN32
    if(RTPrioLevel > 0)
        failed = !SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
    else
        failed = !SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
#elif defined(HAVE_PTHREAD_SETSCHEDPARAM) && !defined(__OpenBSD__)
    struct sched_param param;

    if(RTPrioLevel > 0)
    {
        /* Use the minimum real-time priority possible for now (on Linux this
         * should be 1 for SCHED_RR) */
        param.sched_priority = sched_get_priority_min(SCHED_RR);
        failed = !!pthread_setschedparam(pthread_self(), SCHED_RR, &param);
    }
    else
    {
        param.sched_priority = 0;
        failed = !!pthread_setschedparam(pthread_self(), SCHED_OTHER, &param);
    }
#else
    /* Real-time priority not available */
    failed = (RTPrioLevel>0);
#endif
    if(failed)
        ERR("Failed to set priority level for thread\n");
}


void InitUIntMap(UIntMap *map)
{
    map->array = NULL;
    map->size = 0;
    map->maxsize = 0;
}

void ResetUIntMap(UIntMap *map)
{
    free(map->array);
    map->array = NULL;
    map->size = 0;
    map->maxsize = 0;
}

ALenum InsertUIntMapEntry(UIntMap *map, ALuint key, ALvoid *value)
{
    ALsizei pos = 0;

    if(map->size > 0)
    {
        ALsizei low = 0;
        ALsizei high = map->size - 1;
        while(low < high)
        {
            ALsizei mid = low + (high-low)/2;
            if(map->array[mid].key < key)
                low = mid + 1;
            else
                high = mid;
        }
        if(map->array[low].key < key)
            low++;
        pos = low;
    }

    if(pos == map->size || map->array[pos].key != key)
    {
        if(map->size == map->maxsize)
        {
            ALvoid *temp;
            ALsizei newsize;

            newsize = (map->maxsize ? (map->maxsize<<1) : 4);
            if(newsize < map->maxsize)
                return AL_OUT_OF_MEMORY;

            temp = realloc(map->array, newsize*sizeof(map->array[0]));
            if(!temp) return AL_OUT_OF_MEMORY;
            map->array = temp;
            map->maxsize = newsize;
        }

        map->size++;
        if(pos < map->size-1)
            memmove(&map->array[pos+1], &map->array[pos],
                    (map->size-1-pos)*sizeof(map->array[0]));
    }
    map->array[pos].key = key;
    map->array[pos].value = value;

    return AL_NO_ERROR;
}

void RemoveUIntMapKey(UIntMap *map, ALuint key)
{
    if(map->size > 0)
    {
        ALsizei low = 0;
        ALsizei high = map->size - 1;
        while(low < high)
        {
            ALsizei mid = low + (high-low)/2;
            if(map->array[mid].key < key)
                low = mid + 1;
            else
                high = mid;
        }
        if(map->array[low].key == key)
        {
            if(low < map->size-1)
                memmove(&map->array[low], &map->array[low+1],
                        (map->size-1-low)*sizeof(map->array[0]));
            map->size--;
        }
    }
}

ALvoid *LookupUIntMapKey(UIntMap *map, ALuint key)
{
    if(map->size > 0)
    {
        ALsizei low = 0;
        ALsizei high = map->size - 1;
        while(low < high)
        {
            ALsizei mid = low + (high-low)/2;
            if(map->array[mid].key < key)
                low = mid + 1;
            else
                high = mid;
        }
        if(map->array[low].key == key)
            return map->array[low].value;
    }
    return NULL;
}

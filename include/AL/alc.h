#ifndef ALC_CONTEXT_H_
#define ALC_CONTEXT_H_

#include "altypes.h"
#include "alctypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ALC_VERSION_0_1         1

#ifdef _WIN32
 #ifdef _OPENAL32LIB
  #define ALCAPI __declspec(dllexport)
 #else
  #define ALCAPI __declspec(dllimport)
 #endif

 typedef struct ALCdevice_struct ALCdevice;
 typedef struct ALCcontext_struct ALCcontext;

 #define ALCAPIENTRY __cdecl
#else
 #ifdef TARGET_OS_MAC
  #if TARGET_OS_MAC
   #pragma export on
  #endif
 #endif

 #define ALCAPI
 #define ALCAPIENTRY
#endif

#ifndef AL_NO_PROTOTYPES

ALCAPI ALCcontext * ALCAPIENTRY alcCreateContext( ALCdevice *dev,
						ALCint* attrlist );

/**
 * There is no current context, as we can mix
 *  several active contexts. But al* calls
 *  only affect the current context.
 */
/* ***** GH
TEMPORARY ifdef -- Linux will switch to bool
*/
#ifdef __GNUC__
ALCAPI ALCenum ALCAPIENTRY alcMakeContextCurrent( ALCcontext *alcHandle );
#else
ALCAPI ALCboolean ALCAPIENTRY alcMakeContextCurrent(ALCcontext *alcHandle);
#endif

/**
 * Perform processing on a synced context, non-op on a asynchronous
 * context.
 */

/* ***** GH
 * temporary fix for Windows compilation
 */
#ifdef __GNUC__
ALCAPI ALCcontext * ALCAPIENTRY alcProcessContext( ALCcontext *alcHandle );
#else
ALCAPI ALvoid ALCAPIENTRY alcProcessContext(ALCcontext* context);
#endif

/**
 * Suspend processing on an asynchronous context, non-op on a
 * synced context.
 */
ALCAPI void ALCAPIENTRY alcSuspendContext( ALCcontext *alcHandle );

/* ***** GH
 * temporary fix to benefit Windows compilation
 */
#ifdef __GNUC__
ALCAPI ALCenum ALCAPIENTRY alcDestroyContext( ALCcontext *alcHandle );
#else
ALCAPI ALvoid ALCAPIENTRY alcDestroyContext(ALCcontext* context);
#endif

ALCAPI ALCenum ALCAPIENTRY alcGetError( ALCdevice *dev );

ALCAPI ALCcontext * ALCAPIENTRY alcGetCurrentContext( ALvoid );

/* ***** GH
 * temporary change for Windows compilation
 */
#ifdef __GNUC__
ALCAPI ALCdevice *alcOpenDevice( const ALubyte *tokstr );
#else
ALCAPI ALCdevice *alcOpenDevice( ALubyte *tokstr );
#endif
ALCAPI void alcCloseDevice( ALCdevice *dev );

ALCAPI ALCboolean ALCAPIENTRY alcIsExtensionPresent(ALCdevice *device, ALCubyte *extName);
ALCAPI ALCvoid  * ALCAPIENTRY alcGetProcAddress(ALCdevice *device, ALCubyte *funcName);
ALCAPI ALCenum    ALCAPIENTRY alcGetEnumValue(ALCdevice *device, ALCubyte *enumName);

ALCAPI ALCdevice* ALCAPIENTRY alcGetContextsDevice(ALCcontext *context);


/**
 * Query functions
 */

/* ***** GH
 * temporary fix for Windows compilation
 */
#ifdef __GNUC__
const ALCubyte * alcGetString( ALCdevice *deviceHandle, ALCenum token );
#else
ALCAPI ALubyte* ALCAPIENTRY alcGetString(ALCdevice* device, ALenum param);
#endif
#ifdef __GNUC__
void alcGetIntegerv( ALCdevice *deviceHandle, ALCenum  token , ALCsizei  size , ALCint *dest );
#else
ALCAPI ALCvoid ALCAPIENTRY alcGetIntegerv(ALCdevice *device,ALCenum param,ALCsizei size,ALCint *data);
#endif

#else
      ALCAPI ALCcontext *    ALCAPIENTRY (*alcCreateContext)( ALCdevice *dev, ALCint* attrlist );
/* ***** GH
TEMPORARY ifdef -- Linux will switch to bool
*/
#ifdef __GNUC__
      ALCAPI ALCenum	     ALCAPIENTRY (*alcMakeContextCurrent)( ALCcontext *alcHandle );
#else
      ALCAPI ALCboolean      ALCAPIENTRY (*alcMakeContextCurrent)(ALCcontext *context);
#endif
      ALCAPI ALCcontext *    ALCAPIENTRY (*alcProcessContext)( ALCcontext *alcHandle );
      ALCAPI void            ALCAPIENTRY (*alcSuspendContext)( ALCcontext *alcHandle );
      ALCAPI ALCenum	     ALCAPIENTRY (*alcDestroyContext)( ALCcontext *alcHandle );
      ALCAPI ALCenum	     ALCAPIENTRY (*alcGetError)( ALCdevice *dev );
      ALCAPI ALCcontext *    ALCAPIENTRY (*alcGetCurrentContext)( ALCvoid );
      ALCAPI ALCdevice *     ALCAPIENTRY (*alcOpenDevice)( const ALCubyte *tokstr );
      ALCAPI void            ALCAPIENTRY (*alcCloseDevice)( ALCdevice *dev );
      ALCAPI ALCboolean      ALCAPIENTRY (*alcIsExtensionPresent)( ALCdevice *device, ALCubyte *extName );
      ALCAPI ALCvoid  *      ALCAPIENTRY (*alcGetProcAddress)(ALCdevice *device, ALCubyte *funcName );
      ALCAPI ALCenum         ALCAPIENTRY (*alcGetEnumValue)(ALCdevice *device, ALCubyte *enumName);
      ALCAPI ALCdevice*      ALCAPIENTRY (*alcGetContextsDevice)(ALCcontext *context);
      ALCAPI const ALCubyte* ALCAPIENTRY (*alcGetString)( ALCdevice *deviceHandle, ALCenum token );
      ALCAPI void            ALCAPIENTRY (*alcGetIntegerv*)( ALCdevice *deviceHandle, ALCenum  token , ALCsizei  size , ALCint *dest );

#endif /* AL_NO_PROTOTYPES */

#ifdef TARGET_OS_MAC
#if TARGET_OS_MAC
#pragma export off
#endif /* TARGET_OS_MAC */
#endif /* TARGET_OS_MAC */

#ifdef __cplusplus
}
#endif

#endif /* ALC_CONTEXT_H_ */

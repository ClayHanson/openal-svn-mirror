/**
 * OpenAL cross platform audio library
 * Copyright (C) 1999-2000 by authors.
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

#include "globals.h"
#include "alExtension.h"
#include "alc.h"
#include <string.h>

#ifdef MAC_OS_X
#include <mach-o/dyld.h>
#include "vorbis/codec.h"
#endif

#pragma export on 

#ifdef MAC_OS_X
void *pVorbisLib = NULL;
void *pOggLib = NULL;

ALboolean setVorbisFunctionPointers(void *pLib) 
{
    NSSymbol tmpSymbol;

    tmpSymbol = NSLookupSymbolInImage(pLib, "_vorbis_info_init", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    vorbis_info_init = NSAddressOfSymbol(tmpSymbol);
    if (vorbis_info_init == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_vorbis_info_clear", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    vorbis_info_clear = NSAddressOfSymbol(tmpSymbol);
    if (vorbis_info_clear == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_vorbis_info_blocksize", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    vorbis_info_blocksize = NSAddressOfSymbol(tmpSymbol);
    if (vorbis_info_blocksize == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_vorbis_comment_init", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    vorbis_comment_init = NSAddressOfSymbol(tmpSymbol);
    if (vorbis_comment_init == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_vorbis_comment_add", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    vorbis_comment_add = NSAddressOfSymbol(tmpSymbol);
    if (vorbis_comment_add == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_vorbis_comment_add_tag", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    vorbis_comment_add_tag = NSAddressOfSymbol(tmpSymbol);
    if (vorbis_comment_add_tag == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_vorbis_comment_query", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    vorbis_comment_query = NSAddressOfSymbol(tmpSymbol);
    if (vorbis_comment_query == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_vorbis_comment_query_count", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    vorbis_comment_query_count = NSAddressOfSymbol(tmpSymbol);
    if (vorbis_comment_query_count == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_vorbis_comment_clear", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    vorbis_comment_clear = NSAddressOfSymbol(tmpSymbol);
    if (vorbis_comment_clear == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_vorbis_block_init", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    vorbis_block_init = NSAddressOfSymbol(tmpSymbol);
    if (vorbis_block_init == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_vorbis_block_clear", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    vorbis_block_clear = NSAddressOfSymbol(tmpSymbol);
    if (vorbis_block_clear == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_vorbis_dsp_clear", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    vorbis_dsp_clear = NSAddressOfSymbol(tmpSymbol);
    if (vorbis_dsp_clear == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_vorbis_analysis_init", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    vorbis_analysis_init = NSAddressOfSymbol(tmpSymbol);
    if (vorbis_analysis_init == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_vorbis_commentheader_out", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    vorbis_commentheader_out = NSAddressOfSymbol(tmpSymbol);
    if (vorbis_commentheader_out == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_vorbis_analysis_headerout", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    vorbis_analysis_headerout = NSAddressOfSymbol(tmpSymbol);
    if (vorbis_analysis_headerout == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_vorbis_analysis_buffer", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    vorbis_analysis_buffer = NSAddressOfSymbol(tmpSymbol);
    if (vorbis_analysis_buffer == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_vorbis_analysis_wrote", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    vorbis_analysis_wrote = NSAddressOfSymbol(tmpSymbol);
    if (vorbis_analysis_wrote == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_vorbis_analysis_blockout", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    vorbis_analysis_blockout = NSAddressOfSymbol(tmpSymbol);
    if (vorbis_analysis_blockout == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_vorbis_analysis", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    vorbis_analysis = NSAddressOfSymbol(tmpSymbol);
    if (vorbis_analysis == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_vorbis_bitrate_addblock", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    vorbis_bitrate_addblock = NSAddressOfSymbol(tmpSymbol);
    if (vorbis_bitrate_addblock == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_vorbis_bitrate_flushpacket", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    vorbis_bitrate_flushpacket = NSAddressOfSymbol(tmpSymbol);
    if (vorbis_bitrate_flushpacket == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_vorbis_synthesis_headerin", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    vorbis_synthesis_headerin = NSAddressOfSymbol(tmpSymbol);
    if (vorbis_synthesis_headerin == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_vorbis_synthesis_init", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    vorbis_synthesis_init = NSAddressOfSymbol(tmpSymbol);
    if (vorbis_synthesis_init == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_vorbis_synthesis", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    vorbis_synthesis = NSAddressOfSymbol(tmpSymbol);
    if (vorbis_synthesis == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_vorbis_synthesis_trackonly", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    vorbis_synthesis_trackonly = NSAddressOfSymbol(tmpSymbol);
    if (vorbis_synthesis_trackonly == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_vorbis_synthesis_blockin", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    vorbis_synthesis_blockin = NSAddressOfSymbol(tmpSymbol);
    if (vorbis_synthesis_blockin == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_vorbis_synthesis_pcmout", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    vorbis_synthesis_pcmout = NSAddressOfSymbol(tmpSymbol);
    if (vorbis_synthesis_pcmout == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_vorbis_synthesis_read", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    vorbis_synthesis_read = NSAddressOfSymbol(tmpSymbol);
    if (vorbis_synthesis_read == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_vorbis_packet_blocksize", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    vorbis_packet_blocksize = NSAddressOfSymbol(tmpSymbol);
    if (vorbis_packet_blocksize == NULL) { return AL_FALSE; }

    return AL_TRUE;
}

ALboolean setOggFunctionPointers(void *pLib) 
{
    NSSymbol tmpSymbol;

    tmpSymbol = NSLookupSymbolInImage(pLib, "_oggpack_writeinit", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    oggpack_writeinit = NSAddressOfSymbol(tmpSymbol);
    if (oggpack_writeinit == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_oggpack_writetrunc", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    oggpack_writetrunc = NSAddressOfSymbol(tmpSymbol);
    if (oggpack_writetrunc == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_oggpack_writealign", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    oggpack_writealign = NSAddressOfSymbol(tmpSymbol);
    if (oggpack_writealign == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_oggpack_writecopy", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    oggpack_writecopy = NSAddressOfSymbol(tmpSymbol);
    if (oggpack_writecopy == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_oggpack_reset", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    oggpack_reset = NSAddressOfSymbol(tmpSymbol);
    if (oggpack_reset == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_oggpack_writeclear", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    oggpack_writeclear = NSAddressOfSymbol(tmpSymbol);
    if (oggpack_writeclear == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_oggpack_readinit", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    oggpack_readinit = NSAddressOfSymbol(tmpSymbol);
    if (oggpack_readinit == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_oggpack_write", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    oggpack_write = NSAddressOfSymbol(tmpSymbol);
    if (oggpack_write == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_oggpack_look", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    oggpack_look = NSAddressOfSymbol(tmpSymbol);
    if (oggpack_look == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_oggpack_look1", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    oggpack_look1 = NSAddressOfSymbol(tmpSymbol);
    if (oggpack_look1 == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_oggpack_adv", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    oggpack_adv = NSAddressOfSymbol(tmpSymbol);
    if (oggpack_adv == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_oggpack_adv1", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    oggpack_adv1 = NSAddressOfSymbol(tmpSymbol);
    if (oggpack_adv1 == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_oggpack_read", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    oggpack_read = NSAddressOfSymbol(tmpSymbol);
    if (oggpack_read == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_oggpack_read1", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    oggpack_read1 = NSAddressOfSymbol(tmpSymbol);
    if (oggpack_read1 == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_oggpack_bytes", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    oggpack_bytes = NSAddressOfSymbol(tmpSymbol);
    if (oggpack_bytes == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_oggpack_bits", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    oggpack_bits = NSAddressOfSymbol(tmpSymbol);
    if (oggpack_bits == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_oggpack_get_buffer", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    oggpack_get_buffer = NSAddressOfSymbol(tmpSymbol);
    if (oggpack_get_buffer == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_ogg_stream_packetin", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    ogg_stream_packetin = NSAddressOfSymbol(tmpSymbol);
    if (ogg_stream_packetin == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_ogg_stream_pageout", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    ogg_stream_pageout = NSAddressOfSymbol(tmpSymbol);
    if (ogg_stream_pageout == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_ogg_stream_flush", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    ogg_stream_flush = NSAddressOfSymbol(tmpSymbol);
    if (ogg_stream_flush == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_ogg_sync_init", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    ogg_sync_init = NSAddressOfSymbol(tmpSymbol);
    if (ogg_sync_init == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_ogg_sync_clear", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    ogg_sync_clear = NSAddressOfSymbol(tmpSymbol);
    if (ogg_sync_clear == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_ogg_sync_reset", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    ogg_sync_reset = NSAddressOfSymbol(tmpSymbol);
    if (ogg_sync_reset == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_ogg_sync_destroy", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    ogg_sync_destroy = NSAddressOfSymbol(tmpSymbol);
    if (ogg_sync_destroy == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_ogg_sync_buffer", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    ogg_sync_buffer = NSAddressOfSymbol(tmpSymbol);
    if (ogg_sync_buffer == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_ogg_sync_wrote", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    ogg_sync_wrote = NSAddressOfSymbol(tmpSymbol);
    if (ogg_sync_wrote == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_ogg_sync_pageseek", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    ogg_sync_pageseek = NSAddressOfSymbol(tmpSymbol);
    if (ogg_sync_pageseek == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_ogg_sync_pageout", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    ogg_sync_pageout = NSAddressOfSymbol(tmpSymbol);
    if (ogg_sync_pageout == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_ogg_stream_pagein", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    ogg_stream_pagein = NSAddressOfSymbol(tmpSymbol);
    if (ogg_stream_pagein == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_ogg_stream_packetout", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    ogg_stream_packetout = NSAddressOfSymbol(tmpSymbol);
    if (ogg_stream_packetout == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_ogg_stream_packetpeek", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    ogg_stream_packetpeek = NSAddressOfSymbol(tmpSymbol);
    if (ogg_stream_packetpeek == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_ogg_stream_init", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    ogg_stream_init = NSAddressOfSymbol(tmpSymbol);
    if (ogg_stream_init == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_ogg_stream_clear", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    ogg_stream_clear = NSAddressOfSymbol(tmpSymbol);
    if (ogg_stream_clear == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_ogg_stream_reset", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    ogg_stream_reset = NSAddressOfSymbol(tmpSymbol);
    if (ogg_stream_reset == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_ogg_stream_reset_serialno", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    ogg_stream_reset_serialno = NSAddressOfSymbol(tmpSymbol);
    if (ogg_stream_reset_serialno == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_ogg_stream_destroy", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    ogg_stream_destroy = NSAddressOfSymbol(tmpSymbol);
    if (ogg_stream_destroy == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_ogg_stream_eos", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    ogg_stream_eos = NSAddressOfSymbol(tmpSymbol);
    if (ogg_stream_eos == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_ogg_page_checksum_set", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    ogg_page_checksum_set = NSAddressOfSymbol(tmpSymbol);
    if (ogg_page_checksum_set == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_ogg_page_version", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    ogg_page_version = NSAddressOfSymbol(tmpSymbol);
    if (ogg_page_version == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_ogg_page_continued", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    ogg_page_continued = NSAddressOfSymbol(tmpSymbol);
    if (ogg_page_continued == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_ogg_page_bos", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    ogg_page_bos = NSAddressOfSymbol(tmpSymbol);
    if (ogg_page_bos == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_ogg_page_eos", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    ogg_page_eos = NSAddressOfSymbol(tmpSymbol);
    if (ogg_page_eos == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_ogg_page_granulepos", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    ogg_page_granulepos = NSAddressOfSymbol(tmpSymbol);
    if (ogg_page_granulepos == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_ogg_page_serialno", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    ogg_page_serialno = NSAddressOfSymbol(tmpSymbol);
    if (ogg_page_serialno == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_ogg_page_pageno", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    ogg_page_pageno = NSAddressOfSymbol(tmpSymbol);
    if (ogg_page_pageno == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_ogg_page_packets", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    ogg_page_packets = NSAddressOfSymbol(tmpSymbol);
    if (ogg_page_packets == NULL) { return AL_FALSE; }
    
    tmpSymbol = NSLookupSymbolInImage(pLib, "_ogg_packet_clear", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW);
    ogg_packet_clear = NSAddressOfSymbol(tmpSymbol);
    if (ogg_packet_clear == NULL) { return AL_FALSE; }

    return AL_TRUE;
}
#endif

ALAPI ALboolean ALAPIENTRY alIsExtensionPresent(ALubyte *extName)
{
    ALboolean extPresent;
    ALboolean havePointers;
    
    extPresent = AL_FALSE;
    havePointers = AL_TRUE;

#ifdef MAC_OS_X
    if (!strcmp(extName, "AL_EXT_vorbis")) {
        // check for required libraries
        pVorbisLib = (void *) NSAddImage("libvorbis.dylib", NSADDIMAGE_OPTION_RETURN_ON_ERROR);
        if (pVorbisLib == NULL) {
          // try Fink loc if first try fails
          pVorbisLib = (void *) NSAddImage("/sw/lib/libvorbis.dylib", NSADDIMAGE_OPTION_RETURN_ON_ERROR);
        }
        
        pOggLib = (void *) NSAddImage("libogg.dylib", NSADDIMAGE_OPTION_RETURN_ON_ERROR);
        if (pOggLib == NULL) {
          // try Fink loc if first try fails
          pOggLib = (void *) NSAddImage("/sw/lib/libogg.dylib", NSADDIMAGE_OPTION_RETURN_ON_ERROR);
        }
        
        // set function pointers
        if ((pVorbisLib != NULL) && (pOggLib != NULL)) {
            havePointers = setVorbisFunctionPointers(pVorbisLib);
            if (havePointers == AL_TRUE) {
                havePointers = setOggFunctionPointers(pOggLib);
            }
        }
            
        if ((pVorbisLib != NULL) && (pOggLib != NULL) && (havePointers == AL_TRUE)) {
          extPresent = AL_TRUE;
        }
    }
#endif
    
    return extPresent;
}

ALAPI ALvoid * ALAPIENTRY alGetProcAddress(ALubyte *fname)
{
	return NULL;
}

ALAPI ALenum ALAPIENTRY alGetEnumValue (ALubyte *ename)
{
	if (strcmp("AL_INVALID", (const char *)ename) == 0) { return AL_INVALID; }
	if (strcmp("ALC_INVALID", (const char *)ename) == 0) { return ALC_INVALID; }
	if (strcmp("AL_NONE", (const char *)ename) == 0) { return AL_NONE; }
	if (strcmp("AL_FALSE", (const char *)ename) == 0) { return AL_FALSE; }
	if (strcmp("ALC_FALSE", (const char *)ename) == 0) { return ALC_FALSE; }
	if (strcmp("AL_TRUE", (const char *)ename) == 0) { return AL_TRUE; }
	if (strcmp("ALC_TRUE", (const char *)ename) == 0) { return ALC_TRUE; }
	if (strcmp("AL_SOURCE_RELATIVE", (const char *)ename) == 0) { return AL_SOURCE_RELATIVE; }
	if (strcmp("AL_CONE_INNER_ANGLE", (const char *)ename) == 0) { return AL_CONE_INNER_ANGLE; }
	if (strcmp("AL_CONE_OUTER_ANGLE", (const char *)ename) == 0) { return AL_CONE_OUTER_ANGLE; }
	if (strcmp("AL_PITCH", (const char *)ename) == 0) { return AL_PITCH; }
	if (strcmp("AL_POSITION", (const char *)ename) == 0) { return AL_POSITION; }
	if (strcmp("AL_DIRECTION", (const char *)ename) == 0) { return AL_DIRECTION; }
	if (strcmp("AL_VELOCITY", (const char *)ename) == 0) { return AL_VELOCITY; }
	if (strcmp("AL_LOOPING", (const char *)ename) == 0) { return AL_LOOPING; }
	if (strcmp("AL_BUFFER", (const char *)ename) == 0) { return AL_BUFFER; }
	if (strcmp("AL_GAIN", (const char *)ename) == 0) { return AL_GAIN; }
	if (strcmp("AL_MIN_GAIN", (const char *)ename) == 0) { return AL_MIN_GAIN; }
	if (strcmp("AL_MAX_GAIN", (const char *)ename) == 0) { return AL_MAX_GAIN; }
	if (strcmp("AL_ORIENTATION", (const char *)ename) == 0) { return AL_ORIENTATION; }
	if (strcmp("AL_REFERENCE_DISTANCE", (const char *)ename) == 0) { return AL_REFERENCE_DISTANCE; }
	if (strcmp("AL_ROLLOFF_FACTOR", (const char *)ename) == 0) { return AL_ROLLOFF_FACTOR; }
	if (strcmp("AL_CONE_OUTER_GAIN", (const char *)ename) == 0) { return AL_CONE_OUTER_GAIN; }
	if (strcmp("AL_MAX_DISTANCE", (const char *)ename) == 0) { return AL_MAX_DISTANCE; }
	if (strcmp("AL_SOURCE_STATE", (const char *)ename) == 0) { return AL_SOURCE_STATE; }
	if (strcmp("AL_INITIAL", (const char *)ename) == 0) { return AL_INITIAL; }
	if (strcmp("AL_PLAYING", (const char *)ename) == 0) { return AL_PLAYING; }
	if (strcmp("AL_PAUSED", (const char *)ename) == 0) { return AL_PAUSED; }
	if (strcmp("AL_STOPPED", (const char *)ename) == 0) { return AL_STOPPED; }
	if (strcmp("AL_BUFFERS_QUEUED", (const char *)ename) == 0) { return AL_BUFFERS_QUEUED; }
	if (strcmp("AL_BUFFERS_PROCESSED", (const char *)ename) == 0) { return AL_BUFFERS_PROCESSED; }
	if (strcmp("AL_FORMAT_MONO8", (const char *)ename) == 0) { return AL_FORMAT_MONO8; }
	if (strcmp("AL_FORMAT_MONO16", (const char *)ename) == 0) { return AL_FORMAT_MONO16; }
	if (strcmp("AL_FORMAT_STEREO8", (const char *)ename) == 0) { return AL_FORMAT_STEREO8; }
	if (strcmp("AL_FORMAT_STEREO16", (const char *)ename) == 0) { return AL_FORMAT_STEREO16; }
	if (strcmp("AL_FREQUENCY", (const char *)ename) == 0) { return AL_FREQUENCY; }
	if (strcmp("AL_SIZE", (const char *)ename) == 0) { return AL_SIZE; }
	if (strcmp("AL_UNUSED", (const char *)ename) == 0) { return AL_UNUSED; }
	if (strcmp("AL_PENDING", (const char *)ename) == 0) { return AL_PENDING; }
	if (strcmp("AL_PROCESSED", (const char *)ename) == 0) { return AL_PROCESSED; }
	if (strcmp("ALC_MAJOR_VERSION", (const char *)ename) == 0) { return ALC_MAJOR_VERSION; }
	if (strcmp("ALC_MINOR_VERSION", (const char *)ename) == 0) { return ALC_MINOR_VERSION; }
	if (strcmp("ALC_ATTRIBUTES_SIZE", (const char *)ename) == 0) { return ALC_ATTRIBUTES_SIZE; }
	if (strcmp("ALC_ALL_ATTRIBUTES", (const char *)ename) == 0) { return ALC_ALL_ATTRIBUTES; }
	if (strcmp("ALC_DEFAULT_DEVICE_SPECIFIER", (const char *)ename) == 0) { return ALC_DEFAULT_DEVICE_SPECIFIER; }
	if (strcmp("ALC_DEVICE_SPECIFIER", (const char *)ename) == 0) { return ALC_DEVICE_SPECIFIER; }
	if (strcmp("ALC_EXTENSIONS", (const char *)ename) == 0) { return ALC_EXTENSIONS; }
	if (strcmp("ALC_FREQUENCY", (const char *)ename) == 0) { return ALC_FREQUENCY; }
	if (strcmp("ALC_REFRESH", (const char *)ename) == 0) { return ALC_REFRESH; }
	if (strcmp("ALC_SYNC", (const char *)ename) == 0) { return ALC_SYNC; }
	if (strcmp("AL_NO_ERROR", (const char *)ename) == 0) { return AL_NO_ERROR; }
	if (strcmp("AL_INVALID_NAME", (const char *)ename) == 0) { return AL_INVALID_NAME; }
	if (strcmp("AL_INVALID_ENUM", (const char *)ename) == 0) { return AL_INVALID_ENUM; }
	if (strcmp("AL_INVALID_VALUE", (const char *)ename) == 0) { return AL_INVALID_VALUE; }
	if (strcmp("AL_INVALID_OPERATION", (const char *)ename) == 0) { return AL_INVALID_OPERATION; }
	if (strcmp("AL_OUT_OF_MEMORY", (const char *)ename) == 0) { return AL_OUT_OF_MEMORY; }
	if (strcmp("ALC_NO_ERROR", (const char *)ename) == 0) { return ALC_NO_ERROR; }
	if (strcmp("ALC_INVALID_DEVICE", (const char *)ename) == 0) { return ALC_INVALID_DEVICE; }
	if (strcmp("ALC_INVALID_CONTEXT", (const char *)ename) == 0) { return ALC_INVALID_CONTEXT; }
	if (strcmp("ALC_INVALID_ENUM", (const char *)ename) == 0) { return ALC_INVALID_ENUM; }
	if (strcmp("ALC_INVALID_VALUE", (const char *)ename) == 0) { return ALC_INVALID_VALUE; }
	if (strcmp("ALC_OUT_OF_MEMORY", (const char *)ename) == 0) { return ALC_OUT_OF_MEMORY; }
	if (strcmp("AL_VENDOR", (const char *)ename) == 0) { return AL_VENDOR; }
	if (strcmp("AL_VERSION", (const char *)ename) == 0) { return AL_VERSION; }
	if (strcmp("AL_RENDERER", (const char *)ename) == 0) { return AL_RENDERER; }
	if (strcmp("AL_EXTENSIONS", (const char *)ename) == 0) { return AL_EXTENSIONS; }
	if (strcmp("AL_DOPPLER_FACTOR", (const char *)ename) == 0) { return AL_DOPPLER_FACTOR; }
	if (strcmp("AL_DOPPLER_VELOCITY", (const char *)ename) == 0) { return AL_DOPPLER_VELOCITY; }
	if (strcmp("AL_DISTANCE_MODEL", (const char *)ename) == 0) { return AL_DISTANCE_MODEL; }
	if (strcmp("AL_INVERSE_DISTANCE", (const char *)ename) == 0) { return AL_INVERSE_DISTANCE; }
	if (strcmp("AL_INVERSE_DISTANCE_CLAMPED", (const char *)ename) == 0) { return AL_INVERSE_DISTANCE_CLAMPED; }
	
	return -1;
}

#pragma export off

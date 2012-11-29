/*------------------------------------------------------------------
// Copyright (c) 1997 - 2011
// Robert Umbehant
// htmapp@wheresjames.com
// http://www.wheresjames.com
//
// Redistribution and use in source and binary forms, with or
// without modification, are permitted for commercial and
// non-commercial purposes, provided that the following
// conditions are met:
//
// * Redistributions of source code must retain the above copyright
//   notice, this list of conditions and the following disclaimer.
// * The names of the developers or contributors may not be used to
//   endorse or promote products derived from this software without
//   specific prior written permission.
//
//   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
//   CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
//   INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
//   MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
//   NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
//   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
//   OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
//   EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//----------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#if !defined( _WIN32 ) || defined( _WIN32_WCE ) || defined( __MINGW32__ )
#	include <stdarg.h>
#	include <wchar.h>
#	define VSNPRINTF		vsnprintf
#	define STRTOLL			strtoll
#	define STRTOULL			strtoll
#	if defined( __MINGW32__ )
#		define VSNWPRINTF	vsnwprintf
#	else
#		define VSNWPRINTF	vswprintf
#	endif
#	define WCSTOLL			wcstoll
#	define WCSTOULL			wcstoll
#	if defined( __LP64__ )
#		define CAST_VL(t)	(t)
#	else
#		define CAST_VL(t)	((tcVaList)t)
#	endif
#else
#	include <tchar.h>
#	define VSNPRINTF		_vsnprintf
#	define STRTOLL			_strtoi64
#	define STRTOULL			_strtoi64
#	define VSNWPRINTF		_vsnwprintf
#	define WCSTOLL			_wcstoi64
#	define WCSTOULL			_wcstoi64
#	define CAST_VL(t)		((va_list)t)
#endif

#include "htmapp.h"

namespace str
{

long vPrint( const char *x_pFmt, tcVaList x_pArgs )
{
	return vprintf( x_pFmt, CAST_VL( x_pArgs ) );
}

long Print( const char *x_pFmt, ... )
{
	// Sanity check
	if ( !x_pFmt)
		return -1;

	// Call vargs version
	tcVaList ap; tcVaStart( ap, x_pFmt );
	long lRet = vPrint( x_pFmt, ap );
	tcVaEnd( ap );

	return lRet;
}

//	wvsprintf( pDst, pFmt, (va_list)pArgs );
long vStrFmt( char *x_pDst, unsigned long x_uMax, const char *x_pFmt, tcVaList x_pArgs )
{
	// Verify data pointers
	if ( !x_pDst || !x_pFmt || !x_uMax )
		return -1;

	// Create format string
	long nRet = (long)VSNPRINTF( x_pDst, x_uMax, x_pFmt, CAST_VL( x_pArgs ) );
	if ( 0 > nRet || x_uMax < (unsigned long)nRet )
	{
		// Null terminate buffer
		x_pDst[ x_uMax - 1 ] = 0;

		// Let the user know what went wrong
		return -1;

	} // end if

	// Null terminate
	x_pDst[ nRet ] = 0;

	return nRet;
}

long StrFmt( char *x_pDst, unsigned long x_uMax, const char *x_pFmt, ... )
{
	// Sanity check
	if ( !x_pDst || !x_pFmt || !x_uMax )
		return -1;

	// Call vargs version
	tcVaList ap; tcVaStart( ap, x_pFmt );
	long lRet = vStrFmt( x_pDst, x_uMax, x_pFmt, ap );
	tcVaEnd( ap );

	return lRet;
}

int StrToInt( const char *x_pStr, long x_lRadix )
{
	if ( !x_pStr )
		return 0;
    return strtol( x_pStr, NULL, x_lRadix );
}

unsigned int StrToUInt( const char *x_pStr, long x_lRadix )
{
	if ( !x_pStr )
		return 0;
    return strtoul( x_pStr, NULL, x_lRadix );
}

tc_int64 StrToInt64( const char *x_pStr, long x_lRadix )
{
	if ( !x_pStr )
		return 0;
    return STRTOLL( x_pStr, NULL, x_lRadix );
}

tc_uint64 StrToUInt64( const char *x_pStr, long x_lRadix )
{
	if ( !x_pStr )
		return 0;
    return STRTOULL( x_pStr, NULL, x_lRadix );
}

long StrToLong( const char *x_pStr, long x_lRadix )
{
	if ( !x_pStr )
		return 0;
	return strtol( x_pStr, NULL, x_lRadix );
}

unsigned long StrToULong( const char *x_pStr, long x_lRadix )
{
	if ( !x_pStr )
		return 0;
	return strtoul( x_pStr, NULL, x_lRadix );
}

float StrToFloat( const char *x_pStr )
{
	if ( !x_pStr )
		return 0;
	return (float)strtod( x_pStr, NULL );
}

double StrToDouble( const char *x_pStr )
{
	if ( !x_pStr )
		return 0;
	return strtod( x_pStr, NULL );
}

#ifndef CII_NO_WCHAR

t_stringw ToWcs( const t_string8 &s )
{
	if ( !s.length() )
		return t_stringw();

	// Allocate space
	t_stringw r;
	try { r.resize( s.length() );	}
	catch( ... ) { return t_stringw(); }

	// Attempt conversion
	size_t sz = mbstowcs( &r[ 0 ], s.c_str(), s.length() );
	if ( 0 >= sz ) 
		return t_stringw();

	// Set new string length	
	r.resize( sz );
	return r;
}

t_string8 ToMbs( const t_stringw &s )
{
	if ( !s.length() )
		return t_string8();

	// Allocate space
	t_string8 r;
	try { r.resize( s.length() );	}
	catch( ... ) { return t_string8(); }

	// Attempt conversion
	size_t sz = wcstombs( &r[ 0 ], s.c_str(), s.length() );
	if ( 0 >= sz ) 
		return t_string8();

	// Set new string length	
	r.resize( sz );
	return r;
}

long vPrint( const wchar_t *x_pFmt, tcVaList x_pArgs )
{
	return vwprintf( x_pFmt, CAST_VL( x_pArgs ) );
}

long Print( const wchar_t *x_pFmt, ... )
{
	// Sanity check
	if ( !x_pFmt)
		return -1;

	// Call vargs version
	tcVaList ap; tcVaStart( ap, x_pFmt );
	long lRet = vPrint( x_pFmt, ap );
	tcVaEnd( ap );

	return lRet;
}

//	wvsprintf( pDst, pFmt, (va_list)pArgs );
long vStrFmt( wchar_t *x_pDst, unsigned long x_uMax, const wchar_t *x_pFmt, tcVaList x_pArgs )
{
	// Verify data pointers
	if ( !x_pDst || !x_pFmt || !x_uMax )
		return -1;

	// Create format string
#if defined( _WIN32 ) || defined( __MINGW32__ )
	long nRet = (long)VSNWPRINTF( x_pDst, x_uMax, x_pFmt, CAST_VL( x_pArgs ) );
#else
	long nRet = (long)VSNWPRINTF( x_pDst, x_pFmt, CAST_VL( x_pArgs ) );
#endif
	if ( 0 > nRet || x_uMax < (unsigned long)nRet )
	{
		// Null terminate buffer
		x_pDst[ x_uMax - 1 ] = 0;

		// Let the user know what went wrong
		return -1;

	} // end if

	// Null terminate
	x_pDst[ nRet ] = 0;

	return nRet;
}

long StrFmt( wchar_t *x_pDst, unsigned long x_uMax, const wchar_t *x_pFmt, ... )
{
	// Sanity check
	if ( !x_pDst || !x_pFmt || !x_uMax )
		return -1;

	// Call vargs version
	tcVaList ap; tcVaStart( ap, x_pFmt );
	long lRet = vStrFmt( x_pDst, x_uMax, x_pFmt, ap );
	tcVaEnd( ap );

	return lRet;
}

int StrToInt( const wchar_t *x_pStr, long x_lRadix )
{
	if ( !x_pStr )
		return 0;
	return wcstol( x_pStr, NULL, x_lRadix );
}

unsigned int StrToUInt( const wchar_t *x_pStr, long x_lRadix )
{
	if ( !x_pStr )
		return 0;
	return wcstoul( x_pStr, NULL, x_lRadix );
}

tc_int64 StrToInt64( const wchar_t *x_pStr, long x_lRadix )
{
	if ( !x_pStr )
		return 0;
	return WCSTOLL( x_pStr, NULL, x_lRadix );
}

tc_uint64 StrToUInt64( const wchar_t *x_pStr, long x_lRadix )
{
	if ( !x_pStr )
		return 0;
	return WCSTOULL( x_pStr, NULL, x_lRadix );
}

long StrToLong( const wchar_t *x_pStr, long x_lRadix )
{
	if ( !x_pStr )
		return 0;
	return wcstol( x_pStr, NULL, x_lRadix );
}

unsigned long StrToULong( const wchar_t *x_pStr, long x_lRadix )
{
	if ( !x_pStr )
		return 0;
	return wcstoul( x_pStr, NULL, x_lRadix );
}

float StrToFloat( const wchar_t *x_pStr )
{
	if ( !x_pStr )
		return 0;
	return (float)wcstod( x_pStr, NULL );
}

double StrToDouble( const wchar_t *x_pStr )
{
	if ( !x_pStr )
		return 0;
	return wcstod( x_pStr, NULL );
}

#endif

#define DO1( buf )  x_crc = g_crc_table[ ( x_crc ^ ( *buf++ ) ) & 0xff ] ^ ( x_crc >> 8 )
#define DO2( buf )  DO1( buf ), DO1( buf )
#define DO4( buf )  DO2( buf ), DO2( buf )
#define DO8( buf )  DO4( buf ), DO4( buf )

static unsigned int g_crc_table[ 256 ] = 
{
  0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
  0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
  0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
  0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
  0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
  0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
  0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
  0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
  0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
  0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
  0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
  0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
  0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
  0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
  0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
  0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
  0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
  0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
  0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
  0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
  0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
  0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
  0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
  0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
  0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
  0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
  0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
  0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
  0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
  0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
  0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
  0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
  0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
  0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
  0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
  0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
  0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
  0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
  0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
  0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
  0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
  0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
  0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
  0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
  0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
  0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
  0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
  0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
  0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
  0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
  0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
  0x2d02ef8dL
};

unsigned int CRC32( void *x_buf, unsigned long x_size, unsigned int x_crc )
{
	if ( !x_buf || 0 >= x_size )
		return x_crc;

	unsigned char *buf = (unsigned char*)x_buf;
	x_crc = x_crc ^ 0xffffffffL; 

	while ( x_size >= 8 ) 
	{	DO8 ( buf ); 
		x_size -= 8;
	} 

	if ( x_size >= 4 ) 
	{	DO4 ( buf ); 
		x_size -= 4;
	} // end if

	if ( x_size >= 2 ) 
	{	DO2( buf ); 
		x_size -= 2;
	} // end if

	if ( x_size )
		DO1 ( buf );

	return ( x_crc ^ 0xffffffffL ); 
}

}; // namespace str

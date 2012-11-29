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

#include "htmapp.h"

#if defined( _WIN32 )
#	include <windows.h>
#else
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

namespace sys
{
#if defined( _WIN32 )
#	include "sys-windows.hpp"
#else
#	include "sys-posix.hpp"
#endif

long get_gmt_timestamp()
{
	return time( 0 );
}

long get_local_timestamp()
{
	return time( 0 ) + get_local_tzbias() * 60;
}

static const char* s_htmapp_days[] = 
	{ "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "" };
static const char* s_htmapp_months[] = 
	{ "", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
static const char* s_htmapp_fdays[] = 
	{ "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "" };
static const char* s_htmapp_fmonths[] = 
	{ "", "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };

const char* get_abr_month_name( int x_m )
{
	if ( x_m < 0 || x_m > 12 ) return s_htmapp_months[ 0 ];
	return s_htmapp_months[ x_m ];
}

const char* get_month_name( int x_m )
{
	if ( x_m < 0 || x_m > 12 ) return s_htmapp_fmonths[ 0 ];
	return s_htmapp_fmonths[ x_m ];
}

const char* get_abr_day_name( int x_d )
{
	if ( x_d < 0 || x_d > 6 ) return s_htmapp_days[ 7 ];
	return s_htmapp_days[ x_d ];
}

const char* get_day_name( int x_d )
{
	if ( x_d < 0 || x_d > 6 ) return s_htmapp_fdays[ 7 ];
	return s_htmapp_fdays[ x_d ];
}

int randomize( void *p, int sz )
{
	if ( !p || 0 >= sz )
		return 0;

	// +++ Maybe there's a better way?
	srand( time( 0 ) );
	for ( int i = 0; i < sz; i++ )
		((unsigned char*)p)[ i ] = (unsigned char)rand();

	return sz;
}

}; // namespace sys


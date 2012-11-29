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

#include <sys/time.h>

void sleep( long lMSec )
{
    struct timespec req={0},rem={0};
	req.tv_sec = lMSec / 1000;
	req.tv_nsec = ( lMSec % 1000 ) * 1000000;
	nanosleep( &req, &rem );

//	sleep( lMSec );
}

void usleep( long lUSec )
{
    struct timespec req={0},rem={0};
	req.tv_sec = lUSec / 1000000;
	req.tv_nsec = ( lUSec % 1000000 ) * 1000;
	nanosleep( &req, &rem );
	
//	usleep( lUSec );
}

static void sys_SystemTimeToSTime( struct tm* tinfo, STime &t )
{
    t.uYear = 1900 + tinfo->tm_year;
    t.uMonth = tinfo->tm_mon + 1;
    t.uDayOfWeek = tinfo->tm_wday;
    t.uDay = tinfo->tm_mday;
    t.uHour = tinfo->tm_hour;
    t.uMinute = tinfo->tm_min;
    t.uSecond = tinfo->tm_sec;
    t.nTzBias = 0;
    t.uMillisecond = 0;
    t.uMicrosecond = 0;
    t.uNanosecond = 0;
}

static void sys_STimeToSystemTime( STime &t, struct tm* tinfo )
{
    tinfo->tm_year = t.uYear - 1900;
    tinfo->tm_mon = t.uMonth ? t.uMonth - 1 : 0;
    tinfo->tm_wday = t.uDayOfWeek;
    tinfo->tm_mday = t.uDay;
    tinfo->tm_hour = t.uHour;
    tinfo->tm_min = t.uMinute;
    tinfo->tm_sec = t.uSecond;
//   = t.uMillisecond;
//   = t.uMicrosecond;
//   = t.uNanosecond;
}

int get_local_time( STime &t )
{
	memset( &t, 0, sizeof( t ) );

	time_t current_time;
	time( &current_time );
	struct tm tinfo, *ptinfo = &tinfo;
	
#if !defined( HTM_NOLOCALTIME_R )
	localtime_r( &current_time, &tinfo );
#else
	ptinfo = localtime( &current_time );
	if ( !ptinfo )
		return 0;
#endif

	sys_SystemTimeToSTime( ptinfo, t );

#ifdef HTM_NANOSECONDS
	struct timespec	ts;
	clock_gettime( CLOCK_REALTIME, &ts );
	t.uMillisecond = ts.tv_nsec / 1000 / 1000;
	t.uMicrosecond = ts.tv_nsec / 1000 % 1000;
	t.uNanosecond = ts.tv_nsec % 1000;
#else
	struct timeval tp;
	gettimeofday( &tp, 0 );
	t.uMillisecond = tp.tv_usec / 1000;
	t.uMicrosecond = tp.tv_usec % 1000;
#endif

	// +++ Add time zone crap
//	t.nTzBias = tz.Bias;

    return 1;
}

int set_local_time( STime &t )
{
#if defined( HTM_NOSETTIME )
	return 0;
#else
	struct tm tinfo;
	sys_STimeToSystemTime( t, &tinfo );

	time_t new_time = timelocal( &tinfo );
	return 0 == stime( &new_time );

    return 1;
#endif
}

int get_local_tzbias()
{
	return 0;
}

int get_gmt_time( STime &t )
{
	memset( &t, 0, sizeof( t ) );

	time_t current_time;
	time( &current_time );
	struct tm tinfo, *ptinfo = &tinfo;

#if !defined( htm_NOGMTTIME_R )
	gmtime_r( &current_time, &tinfo );
#else
	ptinfo = gmttime( &current_time );
	if ( !ptinfo )
		return 0;
#endif

	sys_SystemTimeToSTime( ptinfo, t );

#ifdef HTM_NANOSECONDS
	struct timespec	ts;
	clock_gettime( CLOCK_REALTIME, &ts );
	t.uMillisecond = ts.tv_nsec / 1000 / 1000;
	t.uMicrosecond = ts.tv_nsec / 1000 % 1000;
	t.uNanosecond = ts.tv_nsec % 1000;
#else
	struct timeval tp;
	gettimeofday( &tp, 0 );
	t.uMillisecond = tp.tv_usec / 1000;
	t.uMicrosecond = tp.tv_usec % 1000;
#endif

    return 1;
}

int set_gmt_time( STime &t )
{
#if defined( HTM_NOSETTIME )
	return 0;
#else

	struct tm tinfo;
	sys_STimeToSystemTime( t, &tinfo );

	time_t new_time = timegm( &tinfo );
	return 0 == stime( &new_time );

#endif
}



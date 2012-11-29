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

void sleep( long lMSec )
{
	Sleep( lMSec );
}

void usleep( long lUSec )
{
	Sleep( lUSec / 1000 );
}

static void sys_SystemTimeToSTime( SYSTEMTIME &st, STime &t )
{
    t.uYear = st.wYear;
    t.uMonth = st.wMonth;
    t.uDayOfWeek = st.wDayOfWeek;
    t.uDay = st.wDay;
    t.uHour = st.wHour;
    t.uMinute = st.wMinute;
    t.uSecond = st.wSecond;
    t.uMillisecond = st.wMilliseconds;
}

static void sys_STimeToSystemTime( STime &t, SYSTEMTIME &st )
{
    st.wYear = t.uYear;
    st.wMonth = t.uMonth;
    st.wDayOfWeek = t.uDayOfWeek;
    st.wDay = t.uDay;
    st.wHour = t.uHour;
    st.wMinute = t.uMinute;
    st.wSecond = t.uSecond;
    st.wMilliseconds = t.uMillisecond;
}

int get_local_time( STime &t )
{
    memset( &t, 0, sizeof( t ) );

    SYSTEMTIME st;
    memset( &st, 0, sizeof( st ) );

    ::GetLocalTime( &st );
    sys_SystemTimeToSTime( st, t );

    TIME_ZONE_INFORMATION tz;
    memset( &tz, 0, sizeof( tz ) );

    ::GetTimeZoneInformation( &tz );
    t.nTzBias = tz.Bias;

    return 1;
}

int set_local_time( STime &t )
{
    SYSTEMTIME st;
    memset( &st, 0, sizeof( st ) );

	sys_STimeToSystemTime( t, st );

	// Set local time must be called twice to ensure
	// daylight saving is updated correctly.  Why the
	// function couldn't do this on it's own is beyond me.
    ::SetLocalTime( &st );
    ::SetLocalTime( &st );

    return 1;
}

int get_local_tzbias()
{
    TIME_ZONE_INFORMATION tz;
    memset( &tz, 0, sizeof( tz ) );

    ::GetTimeZoneInformation( &tz );
    return tz.Bias;
}

int set_local_tzbias( STime &t )
{
    memset( &t, 0, sizeof( t ) );

    SYSTEMTIME st;
    memset( &st, 0, sizeof( st ) );

    ::GetSystemTime( &st );
    sys_SystemTimeToSTime( st, t );

    return 1;
}

int get_gmt_time( STime &t )
{
    memset( &t, 0, sizeof( t ) );

    SYSTEMTIME st;
    memset( &st, 0, sizeof( st ) );

    ::GetSystemTime( &st );
    sys_SystemTimeToSTime( st, t );

    return 1;
}

int set_gmt_time( STime &t )
{
    SYSTEMTIME st;
    memset( &st, 0, sizeof( st ) );

	sys_STimeToSystemTime( t, st );

    ::SetSystemTime( &st );

    return 1;
}

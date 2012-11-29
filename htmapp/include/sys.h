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

#pragma once


namespace sys
{
    /// Contains time information
    struct STime
    {
        unsigned int	uYear;
        unsigned int	uMonth;
        unsigned int	uDayOfWeek;
        unsigned int	uDay;
        unsigned int	uHour;
        unsigned int	uMinute;
        unsigned int	uSecond;
        unsigned int	uMillisecond;
        unsigned int	uMicrosecond;
        unsigned int	uNanosecond;
        int				nTzBias;
    };

	/// Sleeps for the specified time in milli seconds
	void sleep( long lMSec );

	/// Sleeps the thread for the specified time in micro seconds
	void usleep( long lUSec );

	/// Returns GMT timestamp
	long get_gmt_timestamp();

	/// Returns the GMT time
	int get_gmt_time( STime &t );

	/// Sets GMT time info
	int set_gmt_time( STime *t );

	/// Returns Local timestamp
	long get_local_timestamp();

	/// Returns the local time
	int get_local_time( STime &t );

	/// Sets local time info
	int set_local_time( STime *t );

	/// Gets the local time zone bias
	int get_local_tzbias();

	/// Sets the local time zone bias
	int set_local_tzbias( STime &t );

	/// Randomizes a block of memory
	int randomize( void *p, int sz );

	/// Returns the abbriviated month name
	/**
		@param [in] x_m		- 1 based month index
	*/
	const char* get_abr_month_name( int x_m );

	/// Returns the month name
	/**
		@param [in] x_m		- 1 based month index
	*/
	const char* get_month_name( int x_m );

	/// Returns the abbriviated day name
	/**
		@param [in] x_m		- 0 based day index
	*/
	const char* get_abr_day_name( int x_d );

	/// Returns the day name
	/**
		@param [in] x_m		- 0 based day index
	*/
	const char* get_day_name( int x_d );

	//==============================================================
	// format_time()
	//==============================================================
	/// Returns a formated time string
	/**
		@param [in] x_t			-	Structure containing the time
		@param [in] x_sTmpl		-	Template string
		@param [out] x_bErrors	-	Optional parameter that is set
									to non-zero if any errors are
									detected while decoding.

		Formats a time string based on the specified template.

		-	\%h = hour 12 hour fixed 2 digits
		-	\%H = hour 12 hour
		-	\%g = hour 24 hour fixed 2 digits
		-	\%G = hour 24 hour
		-	\%m = minute fixed 2 digits
		-	\%M = minute
		-	\%s = second fixed 2 digits
		-	\%S = second
		-	\%l = milli seconds fixed 3 digits
		-	\%L = milli seconds
		-	\%u = micro seconds fixed 3 digits
		-	\%U = micro seconds
		-	\%n = nano seconds fixed 3 digits
		-	\%N = nano seconds
		-	\%a = am/pm
		-	\%A = AM/PM
		-	\%c = Month [01-12] fixed 2 digits
		-	\%C = Month [1-12]
		-	\%d = Day [01-31] fixed 2 digits
		-	\%D = Day [1-31]
		-	\%i = Day of the week [0-6]
		-	\%I = Day of the week [1-7]
		-	\%y = 2 digit year
		-	\%Y = 4 digit year
		-	\%k = 2 digit year with base "%k20", "%k19"
		-	\%a = am/pm
		-	\%A = AM/PM
		-	\%w = Abbreviated day of week [Sun,Mon,Tue,Wed,Thu,Fri,Sat]
		-	\%W = Day of week [Sunday,Monday,Tuesday,Wednesday,Thursday,Friday,Saturday]
		-	\%b = Abbreviated Month [Jan,Feb,Mar,Apr,May,Jun,Jul,Aug,Sep,Oct,Nov,Dec]
		-	\%B = Month [January,February,March,April,May,June,July,August,September,October,November,December]
		-	\%Za = Lower case time zone abbreviation [est,gmt,pst...]
		-	\%ZA = Upper case time zone abbreviation [EST,GMT,PST...]
		-	\%Zs = Time zone sign +/-
		-	\%Zh = Hours of offset in time zone fixed two digits
		-	\%ZH = Hours of offset in time zone
		-	\%Zm = Minutes of offset in time zone fixed two digits
		-	\%ZM = Minutes of offset in time zone
		-	\%Zz = Time zone offset in minutes with leading +/-
		-	\%ZZ = Time zone offset in seconds with leading +/-

		Some examples:

		-   "%W, %B %D, %Y - %h:%m:%s %A"	= Thursday, December 25, 1997 - 04:15:30 PM
		-   "%Y/%c/%d - %g:%m:%s.%l"		= 1997/12/25 - 16:15:30.500
		-   "%w, %d %b %Y %g:%m:%s GMT"		= Thu, 25 Dec 1997 16:15:30 GMT
		-	"%c/%b/%Y:%g:%m:%s %Zs%Zh%Zm"	= 25/Dec/1997:16:15:30 -0500

		@return Formated string

		@see parse_time()
	*/
	template< typename T_STR >
		T_STR format_time( const STime &x_t, const T_STR &x_sTmpl, typename T_STR::value_type x_cEsc = 0, int *x_pbErrors = 0 )
	{
		typedef typename T_STR::value_type T;

		// Default escape sequence
		if ( !x_cEsc )
			x_cEsc = tcTC( T, '%' );

		if ( x_pbErrors )
			*x_pbErrors = 0;

		T_STR str;
		typename T_STR::size_type x = 0, l = x_sTmpl.length();

		// Process the template
		while ( x < l )
		{
			// If not the escape character
			if ( x_sTmpl[ x ] != x_cEsc ) 
				str += x_sTmpl[ x ];

			// Replace escape sequence
			else if ( ++x < l ) switch( x_sTmpl[ x ] )
			{
				case tcTC( T, 'h' ) : str += str::StrFmt< T_STR >( tcTT( T, "%0.2u" ),
												( x_t.uHour > 12 ) ? ( x_t.uHour - 12 ) : x_t.uHour ); break;

				case tcTC( T, 'H' ) : str += str::StrFmt< T_STR >( tcTT( T, "%u" ),
												( x_t.uHour > 12 ) ? ( x_t.uHour - 12 ) : x_t.uHour ); break;

				case tcTC( T, 'g' ) : str += str::StrFmt< T_STR >( tcTT( T, "%0.2u" ), x_t.uHour ); break;

				case tcTC( T, 'G' ) : str += str::StrFmt< T_STR >( tcTT( T, "%u" ), x_t.uHour ); break;

				case tcTC( T, 'm' ) : str += str::StrFmt< T_STR >( tcTT( T, "%0.2u" ), x_t.uMinute ); break;

				case tcTC( T, 'M' ) : str += str::StrFmt< T_STR >( tcTT( T, "%u" ), x_t.uMinute ); break;

				case tcTC( T, 's' ) : str += str::StrFmt< T_STR >( tcTT( T, "%0.2u" ), x_t.uSecond ); break;

				case tcTC( T, 'S' ) : str += str::StrFmt< T_STR >( tcTT( T, "%u" ), x_t.uSecond ); break;

				case tcTC( T, 'l' ) : str += str::StrFmt< T_STR >( tcTT( T, "%0.3u" ), x_t.uMillisecond ); break;

				case tcTC( T, 'L' ) : str += str::StrFmt< T_STR >( tcTT( T, "%u" ), x_t.uMillisecond ); break;

				case tcTC( T, 'u' ) : str += str::StrFmt< T_STR >( tcTT( T, "%0.3u" ), x_t.uMicrosecond ); break;

				case tcTC( T, 'U' ) : str += str::StrFmt< T_STR >( tcTT( T, "%u" ), x_t.uMicrosecond ); break;

				case tcTC( T, 'n' ) : str += str::StrFmt< T_STR >( tcTT( T, "%0.3u" ), x_t.uNanosecond ); break;

				case tcTC( T, 'N' ) : str += str::StrFmt< T_STR >( tcTT( T, "%u" ), x_t.uNanosecond ); break;

				case tcTC( T, 'c' ) : str += str::StrFmt< T_STR >( tcTT( T, "%0.2u" ), x_t.uMonth ); break;

				case tcTC( T, 'C' ) : str += str::StrFmt< T_STR >( tcTT( T, "%u" ), x_t.uMonth ); break;

				case tcTC( T, 'd' ) : str += str::StrFmt< T_STR >( tcTT( T, "%0.2u" ), x_t.uDay ); break;

				case tcTC( T, 'D' ) : str += str::StrFmt< T_STR >( tcTT( T, "%u" ), x_t.uDay ); break;

				case tcTC( T, 'i' ) : str += str::StrFmt< T_STR >( tcTT( T, "%u" ), x_t.uDayOfWeek ); break;

				case tcTC( T, 'I' ) : str += str::StrFmt< T_STR >( tcTT( T, "%u" ), x_t.uDayOfWeek + 1 ); break;

				case tcTC( T, 'k' ) : x += 2;
				case tcTC( T, 'y' ) : str += str::StrFmt< T_STR >( tcTT( T, "%0.2u" ), ( x_t.uYear % 100 ) ); break;

				case tcTC( T, 'Y' ) : str += str::StrFmt< T_STR >( tcTT( T, "%u" ), x_t.uYear ); break;

				case tcTC( T, 'a' ) : str += ( ( x_t.uHour >= 12 ) ? tcTT( T, "pm" ) : tcTT( T, "am" ) ); break;

				case tcTC( T, 'A' ) : str += ( ( x_t.uHour >= 12 ) ? tcTT( T, "PM" ) : tcTT( T, "AM" ) ); break;

				case tcTC( T, 'w' ) : str += get_abr_day_name( x_t.uDayOfWeek ); break;

				case tcTC( T, 'W' ) : str += get_day_name( x_t.uDayOfWeek ); break;

				case tcTC( T, 'b' ) : str += get_abr_month_name( x_t.uMonth ); break;

				case tcTC( T, 'B' ) : str += get_month_name( x_t.uMonth ); break;

				case tcTC( T, 'Z' ) : case tcTC( T, 'z' ) :
				{
					switch( x_sTmpl[ ++x ] )
					{
						case tcTC( T, 's' ) :
						case tcTC( T, 'S' ) : str += ( ( 0 > x_t.nTzBias ) ? tcTC( T, '-' ) : tcTC( T, '+' ) ); break;

						case tcTC( T, 'h' ) : str += str::StrFmt< T_STR >( tcTT( T, "%0.2u" ), ( 0 > x_t.nTzBias ? -x_t.nTzBias : x_t.nTzBias ) / 60 ); break;

						case tcTC( T, 'H' ) : str += str::StrFmt< T_STR >( tcTT( T, "%u" ), ( 0 > x_t.nTzBias ? -x_t.nTzBias : x_t.nTzBias ) / 60 ); break;

						case tcTC( T, 'm' ) : str += str::StrFmt< T_STR >( tcTT( T, "%0.2u" ), ( 0 > x_t.nTzBias ? -x_t.nTzBias : x_t.nTzBias ) % 60 ); break;

						case tcTC( T, 'M' ) : str += str::StrFmt< T_STR >( tcTT( T, "%u" ), ( 0 > x_t.nTzBias ? -x_t.nTzBias : x_t.nTzBias ) % 60 ); break;

						// Unknown escape sequence
						default :
							if ( x_pbErrors ) 
								(*x_pbErrors)++;
							break;

					} // end switch

				} break;

				default :

					if ( x_pbErrors && x_cEsc != x_sTmpl[ x ] )
						(*x_pbErrors)++;

					str += x_sTmpl[ x ];

					break;

			}; // end switch

			// Next character
			x++;

		} // end while

		return str;
	}

	template< typename T_STR >
		T_STR format_gmt_time( const T_STR &x_sTmpl, typename T_STR::value_type x_cEsc = 0, int *x_pbErrors = 0 )
		{	STime gt; get_gmt_time( gt );
			return format_time( gt, x_sTmpl, x_cEsc, x_pbErrors );
		}

	template< typename T_STR >
		T_STR format_local_time( const T_STR &x_sTmpl, typename T_STR::value_type x_cEsc = 0, int *x_pbErrors = 0 )
		{	STime lt; get_local_time( lt );
			return format_time( lt, x_sTmpl, x_cEsc, x_pbErrors );
		}

	//==============================================================
	// ParseString()
	//==============================================================
	///
	/**
		@param [in] x_t		-	Structure that receives the time
		@param [in] x_sTmpl	-	Template string
		@param [in] x_sStr	-	String to parse

		This function decomposes an existing time string based
		on the template string.

		-	\%h = hour 12 hour fixed 2 digits
		-	\%H = hour 12 hour
		-	\%g = hour 24 hour fixed 2 digits
		-	\%G = hour 24 hour
		-	\%m = minute fixed 2 digits
		-	\%M = minute
		-	\%s = second fixed 2 digits
		-	\%S = second
		-	\%l = milli seconds fixed 3 digits
		-	\%L = milli seconds
		-	\%u = micro seconds fixed 3 digits
		-	\%U = micro seconds
		-	\%n = nano seconds fixed 3 digits
		-	\%N = nano seconds
		-	\%a = am/pm
		-	\%A = AM/PM
		-	\%c = Month [01-12] fixed 2 digits
		-	\%C = Month [1-12]
		-	\%d = Day [01-31] fixed 2 digits
		-	\%D = Day [1-31]
		-	\%i = Day of the week [0-6]
		-	\%I = Day of the week [1-7]
		-	\%y = 2 digit year
		-	\%Y = 4 digit year
		-	\%a = am/pm
		-	\%A = AM/PM
		-	\%w = Abbreviated day of week [Sun,Mon,Tue,Wed,Thu,Fri,Sat]
		-	\%W = Day of week [Sunday,Monday,Tuesday,Wednesday,Thursday,Friday,Saturday]
		-	\%b = Abbreviated Month [Jan,Feb,Mar,Apr,May,Jun,Jul,Aug,Sep,Oct,Nov,Dec]
		-	\%B = Month [January,February,March,April,May,June,July,August,September,October,November,December]
		-	\%Za = Lower case time zone abbreviation [est,gmt,pst...]
		-	\%ZA = Upper case time zone abbreviation [EST,GMT,PST...]
		-	\%Zs = Time zone sign +/-
		-	\%Zh = Hours of offset in time zone fixed two digits
		-	\%ZH = Hours of offset in time zone
		-	\%Zm = Minutes of offset in time zone fixed two digits
		-	\%ZM = Minutes of offset in time zone
		-	\%Zz = Time zone offset in minutes with leading +/-
		-	\%ZZ = Time zone offset in seconds with leading +/-

		For timezones, leading +/- is optional. + is the default.

		@return This function returns Zero if the format does not match the template

		@see format_time()
	*/
	template< typename T_STR >
		long parse_time( const STime &x_t, const T_STR &x_sTmpl, const T_STR &x_sStr, typename T_STR::value_type x_cEsc = 0, int *x_pbErrors = 0 )
	{
#		define HTM_2DIGIT_YEAR_BASE		2000
#		define HTM_INVALID_TIME 		0xffffffff
		typedef typename T_STR::value_type T;
	
		unsigned int uYear = HTM_INVALID_TIME;
		unsigned int uMonth = HTM_INVALID_TIME;
		unsigned int uDay = HTM_INVALID_TIME;
		unsigned int uHour = HTM_INVALID_TIME;
		unsigned int uMinute = HTM_INVALID_TIME;
		unsigned int uSecond = HTM_INVALID_TIME;
		unsigned int uMillisecond = HTM_INVALID_TIME;
		unsigned int uMicrosecond = HTM_INVALID_TIME;
		unsigned int uNanosecond = HTM_INVALID_TIME;
		unsigned int uDayOfWeek = HTM_INVALID_TIME;

		int bPM = 0;
		int bTzNeg = 0;
		int  nHBias = HTM_INVALID_TIME;
		int  nMBias = HTM_INVALID_TIME;

		typename T_STR::size_type x = 0, tl = x_sTmpl.length(), nEnd = 0, y = 0, sl = x_sStr.length();

		// Default escape sequence
		if ( !x_cEsc )
			x_cEsc = tcTC( T, '%' );

		int _bErrors = 0;
		if ( !x_pbErrors )
			x_pbErrors = &_bErrors;

		*x_pbErrors = 0;

		// Process the template
		while ( x < tl && y < sl )
		{
			// If not the escape character
			if ( x_cEsc != x_sTmpl[ x ]  )
			{
				// Skip char type?
				if ( tcTC( T, '*' ) == x_sTmpl[ x ] )
				{
					// Skip all occurences of the next character
					if ( ++x <  tl )
						while ( y < sl && x_sStr[ y ] == x_sTmpl[ x ] )
							y++;

				} // end if

				else
				{
					// Errors?
					if ( tcTC( T, '?' ) != x_sTmpl[ x ] && x_sStr[ y ] != x_sTmpl[ x ] )
						(*x_pbErrors)++;

					y++;

				} // end else
			}

			else if ( ++x < tl ) switch ( x_sTmpl[ x ] )
			{
				case tcTC( T, 'h' ) :
				case tcTC( T, 'H' ) :
					nEnd = str::atod( &x_sStr[ y ], &uHour, 2 ); y += nEnd;
					if ( !nEnd || 1 > uHour || 12 < uHour ) (*x_pbErrors)++;
					break;

				case tcTC( T, 'g' ) :
				case tcTC( T, 'G' ) :
					nEnd = str::atod( &x_sStr[ y ], &uHour, 2 ); y += nEnd;
					if ( !nEnd || 24 < uHour ) (*x_pbErrors)++;
					break;

				case tcTC( T, 'm' ) :
				case tcTC( T, 'M' ) :
					nEnd = str::atod( &x_sStr[ y ], &uMinute, 2 ); y += nEnd;
					if ( !nEnd || 59 < uMinute ) (*x_pbErrors)++;
					break;

				case tcTC( T, 's' ) :
				case tcTC( T, 'S' ) :
					nEnd = str::atod( &x_sStr[ y ], &uSecond, 2 ); y += nEnd;
					if ( !nEnd || 59 < uSecond ) (*x_pbErrors)++;
					break;

				case tcTC( T, 'l' ) :
				case tcTC( T, 'L' ) :
					nEnd = str::atod( &x_sStr[ y ], &uMillisecond, 3 ); y += nEnd;
					if ( !nEnd || 999 < uMillisecond ) (*x_pbErrors)++;
					break;

				case tcTC( T, 'u' ) :
				case tcTC( T, 'U' ) :
					nEnd = str::atod( &x_sStr[ y ], &uMicrosecond, 3 ); y += nEnd;
					if ( !nEnd || 999 < uMicrosecond ) (*x_pbErrors)++;
					break;

				case tcTC( T, 'n' ) :
				case tcTC( T, 'N' ) :
					nEnd = str::atod( &x_sStr[ y ], &uNanosecond, 3 ); y += nEnd;
					if ( !nEnd || 999 < uNanosecond ) (*x_pbErrors)++;
					break;

				case tcTC( T, 'c' ) :
				case tcTC( T, 'C' ) :
					nEnd = str::atod( &x_sStr[ y ], &uMonth, 2 ); y += nEnd;
					if ( !nEnd || 1 > uMonth || 12 < uMonth ) (*x_pbErrors)++;
					break;

				case tcTC( T, 'd' ) :
				case tcTC( T, 'D' ) :
					nEnd = str::atod( &x_sStr[ y ], &uDay, 2 ); y += nEnd;
					if ( !nEnd || 31 < uDay ) (*x_pbErrors)++;
					break;

				case tcTC( T, 'i' ) :
				case tcTC( T, 'I' ) :
					nEnd = str::atod( &x_sStr[ y ], &uDayOfWeek, 2 ); y += nEnd;
					if ( !nEnd || 6 < uDayOfWeek ) (*x_pbErrors)++;
					break;

				case tcTC( T, 'k' ) :
					if ( ( x + 2 ) < tl )
					{	unsigned int uBase = 0;
						if ( 2 != str::atod( &x_sTmpl[ x + 1 ], &uBase, 2 ) )
							(*x_pbErrors)++, uBase = 0;
						nEnd = str::atod( &x_sStr[ y ], &uYear, 2 ); y += nEnd;
						if ( !nEnd ) (*x_pbErrors)++;
						uYear += uBase * 100;
					} // end if

				case tcTC( T, 'y' ) :
					nEnd = str::atod( &x_sStr[ y ], &uYear, 2 ); y += nEnd;
					if ( !nEnd ) (*x_pbErrors)++; uYear += HTM_2DIGIT_YEAR_BASE;
					break;

				case tcTC( T, 'Y' ) :
					nEnd = str::atod( &x_sStr[ y ], &uYear, 4 ); y += nEnd;
					if ( !nEnd ) (*x_pbErrors)++;
					break;

				case tcTC( T, 'a' ) :
				case tcTC( T, 'A' ) :
				{	int bInvalid = 0;
					T_STR s = str::ToLower( T_STR( x_sStr, y, 2 ) );
					if ( s == "pm" )
						bPM = 1;
					else if ( s == "am" )
						bPM = 0;
					else
						(*x_pbErrors)++;
					y += 2;
				} break;

				case tcTC( T, 'w' ) :
				{   int k;
					for ( k = 0; k < 7; k++ )
					{	T_STR a( get_abr_day_name( k ) ), b( x_sStr, y, a.length() );
						if ( a == b )
						{   uDayOfWeek = k; 
							y += a.length(); 
							k = 99; 
						} // end if
					} // end for
					if ( k < 99 )
						(*x_pbErrors)++;
				} break;

				case tcTC( T, 'W' ) :
				{   int k;
					for ( k = 0; k < 7; k++ )
					{	T_STR a( get_day_name( k ) ), b( x_sStr, y, a.length() );
						if ( a == b )
						{   uDayOfWeek = k; 
							y += a.length(); 
							k = 99; 
						} // end if
					} // end for
					if ( k < 99 )
						(*x_pbErrors)++;
				} break;

				case tcTC( T, 'b' ) :
				{   int k;
					for ( k = 1; k <= 12; k++ )
					{	T_STR a( get_abr_month_name( k ) ), b( x_sStr, y, a.length() );
						if ( a == b )
						{   uMonth = k; 
							y += a.length(); 
							k = 99; 
						} // end if
					} // end for
					if ( k < 99 )
						(*x_pbErrors)++;
				} break;

				case tcTC( T, 'B' ) :
				{   int k;
					for ( k = 1; k <= 12; k++ )
					{	T_STR a( get_month_name( k ) ), b( x_sStr, y, a.length() );
						if ( a == b )
						{   uMonth = k; 
							y += a.length(); 
							k = 99; 
						} // end if
					} // end for
					if ( k < 99 )
						(*x_pbErrors)++;
				} break;

				case tcTC( T, 'Z' ) : 
				case tcTC( T, 'z' ) :
				{
					switch( x_sTmpl[ ++x ] )
					{
						case tcTC( T, 's' ) : 
						case tcTC( T, 'S' ) :
							if ( tcTC( T, '+' ) == x_sStr[ y ] ) y++, bTzNeg = 0;
							else if ( '-' == x_sStr[ y ] ) y++, bTzNeg = 1;
							else (*x_pbErrors)++;
							break;

						case tcTC( T, 'h' ) :
						case tcTC( T, 'H' ) :
							nEnd = str::atod( &x_sStr[ y ], &nHBias, 2 ); y += nEnd;
							if ( !nEnd ) (*x_pbErrors)++;
							break;

						case tcTC( T, 'm' ) :
						case tcTC( T, 'M' ) :
							nEnd = str::atod( &x_sStr[ y ], &nMBias, 2 ); y += nEnd;
							if ( !nEnd ) (*x_pbErrors)++;
							break;

						default :
							(*x_pbErrors)++;
							break;

					} // end switch

				} break;

				default :
					(*x_pbErrors)++;
					break;

			} // end else switch

			// Next character
			x++;

		} // end while

		// AM/PM
		if ( bPM )
			uHour = ( uHour + 12 ) % 24;

		// Calculate bias
		int nTzBias = HTM_INVALID_TIME;
		if ( nHBias != HTM_INVALID_TIME ) 
			nTzBias = nHBias * 60;
		if ( nMBias != HTM_INVALID_TIME ) 
			nTzBias = ( nTzBias == HTM_INVALID_TIME ) ? nMBias : nTzBias + nMBias;
		if ( HTM_INVALID_TIME != nTzBias && bTzNeg ) 
			nTzBias = -nTzBias;

		// Set valid parts of the time
#		define HTM_SET_IF_CHANGE( v ) if ( HTM_INVALID_TIME != v ) x_t.v = v;
		HTM_SET_IF_CHANGE( uYear );
		HTM_SET_IF_CHANGE( uMonth );
		HTM_SET_IF_CHANGE( uDay );
		HTM_SET_IF_CHANGE( uHour );
		HTM_SET_IF_CHANGE( uMinute );
		HTM_SET_IF_CHANGE( uSecond );
		HTM_SET_IF_CHANGE( uMillisecond );
		HTM_SET_IF_CHANGE( uMicrosecond );
		HTM_SET_IF_CHANGE( uNanosecond );
		HTM_SET_IF_CHANGE( uDayOfWeek );
		HTM_SET_IF_CHANGE( nTzBias );

		// How much did we process?
		return y;
	}

}; // namespace sys


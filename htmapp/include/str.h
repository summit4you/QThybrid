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

#include <string>
#include <vector>
#include <string.h>

#ifndef stdForeach
#	define stdForeach( _t, _i, _o ) for ( _t _i = _o.begin(); _o.end() != _i; _i++ )
#endif

#if ( defined( UNICODE ) || defined( _UNICODE ) ) && !defined( CII_NO_WCHAR )
#	define tcUNICODE
#	define tcT( s )			L##s
#	define tcStr2Mb( s )	str::ToMbs( s )
#	define tcStr2Wc( s )	s
#	define tcMb2Str( s )	str::ToWcs( s )
#	define tcWc2Str( s )	s
#else
#	define tcMULTIBYTE
#	define tcT( s )			s
#	define tcStr2Mb( s )	s
#	define tcStr2Wc( s )	str::ToWcs( s )
#	define tcMb2Str( s )	s
#	define tcWc2Str( s )	str::ToMbs( s )
#endif

// +++ Anyone know a better way?
#define tcTT( c, s )		( 1 == sizeof( c ) ? ( ( c* )( s ) ) : ( ( c* )( L##s ) ) )
#define tcTC( c, s )		( 1 == sizeof( c ) ? ( ( c )( s ) ) : ( ( c )( L##s ) ) )
#define tcTTEXT( c, s )		tcTT( c, s )
#define tcNL8				"\n"
#define tcNULL				(0)

#if defined( __GNUC__ )
#	define tcVaList				__builtin_va_list
#	define tcVaStart			__builtin_va_start
#	define tcVaEnd				__builtin_va_end
#	define tcVaArg				__builtin_va_arg
#elif defined( _WIN32 )
#	define tcVaList				void*
#	define tcVaStart( v, p )	( v = ( ( (void**)&p ) + 1 ) )
#	define tcVaEnd( v )
#	define tcVaArg( v, t )		( (t)( v++ ) )
#else
#	if defined( _WIN32_WCE )
#		include <windows.h>
#	endif
#	include <stdarg.h>
#	define tcVaList				va_list
#	define tcVaStart			va_start
#	define tcVaEnd				va_end
#	define tcVaArg				va_arg
#endif

#define tcM			( str::Print( "%s(%lu): MARKER\n", __FILE__, __LINE__ ) )
#define tcS( s )	( str::Print( "%s(%lu): %s = %s\n", __FILE__, __LINE__, #s, str::ToString< str::t_string8>( s ).c_str() ) )

#if defined( __MINGW64__ )
#define tcPtrType long long int
#else
#define tcPtrType unsigned long long int
#endif

#define tcPtrToInt( p ) ( (int)(tcPtrType)p )
#define tcPtrToUInt( p ) ( (unsigned int)(tcPtrType)p )
#define tcPtrToLong( p ) ( (long)(tcPtrType)p )
#define tcPtrToULong( p ) ( (unsigned long)(tcPtrType)p )

#define tcIntToPtr( p ) ( (tcPtrType)(int)p )
#define tcUIntToPtr( p ) ( (tcPtrType)(unsigned int)p )
#define tcLongToPtr( p ) ( (tcPtrType)(long)p )
#define tcULongToPtr( p ) ( (tcPtrType)(unsigned long)p )

namespace str
{
	// char string type
	typedef char t_char8;
	typedef std::basic_string< t_char8 > t_string8;

	// wide char string type
	typedef wchar_t t_charw;
	typedef std::basic_string< t_charw > t_stringw;

#if defined( tcUNICODE )
	typedef wchar_t t_char;
#else
	typedef char t_char;
#endif
	typedef std::basic_string< t_char > t_string;

	/// Size type
	typedef long t_size;

	/// 64 bit signed integer
	typedef signed long long int tc_int64;

	/// 64 bit unsigned integer
	typedef unsigned long long int tc_uint64;

	/// Default string size
	const t_size DEFSIZE = 1024;

	/// String format
	long vPrint( const char *x_pFmt, tcVaList x_pArgs );

	/// String format
	long Print( const char *x_pFmt, ... );

	/// String format
	long vStrFmt( char *x_pDst, unsigned long x_uMax, const char *x_pFmt, tcVaList x_pArgs );

	/// String format
	long StrFmt( char *x_pDst, unsigned long x_uMax, const char *x_pFmt, ... );

	/// String format
	template< typename T_STR >
		T_STR StrFmt( const typename T_STR::value_type *x_pFmt, ... )
	{
		T_STR s;
		long lRet;
		t_size sz = DEFSIZE;

		do
		{
			// Allocate space
			try { s.resize( sz ); }
			catch( ... ) { return T_STR(); }

			// Attempt conversion
			tcVaList ap; tcVaStart( ap, x_pFmt );
			lRet = vStrFmt( &s[ 0 ], sz, x_pFmt, ap );
			tcVaEnd( ap );

			// Try more buffer space if failed
			if ( 0 > lRet )
				sz <<= 1;

			// Set string size
			else
				s.resize( lRet );

		} while ( sz && 0 > lRet );

		return s;
	}

	/// String format
	template< typename T_STR >
		long StrFmt( T_STR &s, const typename T_STR::value_type *x_pFmt, ... )
	{
		long lRet;
		t_size sz = DEFSIZE;

		do
		{
			// Allocate space
			try { s.resize( sz ); }
			catch( ... ) { return T_STR(); }

			// Attempt conversion
			tcVaList ap; tcVaStart( ap, x_pFmt );
			lRet = vStrFmt( &s[ 0 ], sz, x_pFmt, ap );
			tcVaEnd( ap );

			// Try more buffer space if failed
			if ( 0 > lRet )
				sz <<= 1;

			// Set string size
			else
				s.resize( lRet );

		} while ( sz && 0 > lRet );

		return lRet;
	}

	/// Converts to int
	int StrToInt( const char *x_pStr, long x_lRadix = 10 );

	/// Converts to uint
	unsigned int StrToUInt( const char *x_pStr, long x_lRadix = 10 );

	/// Converts to int64
	tc_int64 StrToInt64( const char *x_pStr, long x_lRadix = 10 );

	/// Converts to uint64
	tc_uint64 StrToUInt64( const char *x_pStr, long x_lRadix = 10 );

	/// Converts to long
	long StrToLong( const char *x_pStr, long x_lRadix = 10 );

	/// Converts to unsigned long
	unsigned long StrToULong( const char *x_pStr, long x_lRadix = 10 );

	/// Converts to double
	float StrToFloat( const char *x_pStr );

	/// Converts to double
	double StrToDouble( const char *x_pStr );

#ifndef CII_NO_WCHAR

	/// Convert wc string to mb
	t_stringw ToWcs( const t_string8 &s );

	/// Convert mb char string to wc
	t_string8 ToMbs( const t_stringw &s );

	/// String format
	long vPrint( const wchar_t *x_pFmt, tcVaList x_pArgs );

	/// String format
	long Print( const wchar_t *x_pFmt, ... );

	/// String format
	long vStrFmt( wchar_t *x_pDst, unsigned long x_uMax, const wchar_t *x_pFmt, tcVaList x_pArgs );

	/// String format
	long StrFmt( wchar_t *x_pDst, unsigned long x_uMax, const wchar_t *x_pFmt, ... );

	/// Converts to int
	int StrToInt( const wchar_t *x_pStr, long x_lRadix = 10 );

	/// Converts to uint
	unsigned int StrToUInt( const wchar_t *x_pStr, long x_lRadix = 10 );

	/// Converts to int64
	tc_int64 StrToInt64( const wchar_t *x_pStr, long x_lRadix = 10 );

	/// Converts to uint64
	tc_uint64 StrToUInt64( const wchar_t *x_pStr, long x_lRadix = 10 );

	/// Converts to long
	long StrToLong( const wchar_t *x_pStr, long x_lRadix = 10 );

	/// Converts to unsigned long
	unsigned long StrToULong( const wchar_t *x_pStr, long x_lRadix = 10 );

	/// Converts to double
	float StrToFloat( const wchar_t *x_pStr );

	/// Converts to double
	double StrToDouble( const wchar_t *x_pStr );

#endif

	template< typename T_STR >
		T_STR ToString( int n )
		{	typedef typename T_STR::value_type T; T szNum[ 256 ] = { 0 };
			return T_STR( szNum, StrFmt( szNum, sizeof( szNum ), tcTT( T, "%d" ), n ) );
		}

	template< typename T_STR >
		T_STR ToString( unsigned int n )
		{	typedef typename T_STR::value_type T; T szNum[ 256 ] = { 0 };
			return T_STR( szNum, StrFmt( szNum, sizeof( szNum ), tcTT( T, "%u" ), n ) );
		}

	template< typename T_STR >
		T_STR ToString( long n )
		{	typedef typename T_STR::value_type T; T szNum[ 256 ] = { 0 };
			return T_STR( szNum, StrFmt( szNum, sizeof( szNum ), tcTT( T, "%li" ), n ) );
		}

	template< typename T_STR >
		T_STR ToString( unsigned long n )
		{	typedef typename T_STR::value_type T; T szNum[ 256 ] = { 0 };
			return T_STR( szNum, StrFmt( szNum, sizeof( szNum ), tcTT( T, "%lu" ), n ) );
		}

	template< typename T_STR >
		T_STR ToString( tc_int64 n )
		{	typedef typename T_STR::value_type T; T szNum[ 256 ] = { 0 };
			return T_STR( szNum, StrFmt( szNum, sizeof( szNum ), tcTT( T, "%lld" ), n ) );
		}

	template< typename T_STR >
		T_STR ToString( tc_uint64 n )
		{	typedef typename T_STR::value_type T; T szNum[ 256 ] = { 0 };
			return T_STR( szNum, StrFmt( szNum, sizeof( szNum ), tcTT( T, "%llu" ), n ) );
		}

	template< typename T_STR >
		T_STR ToString( float n )
		{	typedef typename T_STR::value_type T; T szNum[ 256 ] = { 0 };
			return T_STR( szNum, StrFmt( szNum, sizeof( szNum ), tcTT( T, "%f" ), (double)n ) );
		}

	template< typename T_STR >
		T_STR ToString( double n )
		{	typedef typename T_STR::value_type T; T szNum[ 256 ] = { 0 };
			return T_STR( szNum, StrFmt( szNum, sizeof( szNum ), tcTT( T, "%f" ), n ) );
		}

	template< typename T_STR >
		T_STR ToString( const T_STR &s )
		{	return s; }

	template< typename T_STR >
		T_STR ToString( const typename T_STR::value_type *s )
		{	return T_STR( s ); }

	template < typename T >
		long Compare( T *s1, long l1, T *s2, long l2 )
		{
			if ( !s1 || !s2 )
				return 0;

			if ( l1 != l2 )
				return l1 - l2;

			return memcmp( s1, s2, l1 * sizeof( T ) );
		}

	template< typename T_STR >
		long Compare( const T_STR &s1, const T_STR &s2 )
		{	return Compare( s1.data(), s1.length(), s2.data(), s2.length() ); }

	template< typename T_STR >
		long CompareI( const T_STR &s1, const T_STR &s2 )
		{	T_STR ls1 = ToLower( s1 ), ls2 = ToLower( s2 );
			return Compare( ls1.data(), ls1.length(), ls2.data(), ls2.length() );
		}

	template< typename T_STR >
		static T_STR TrimWsInPlace( const T_STR &x_str )
	{
		typedef typename T_STR::value_type T;

		typename T_STR::size_type s = 0, e = x_str.length();
		if ( !e )
			return T_STR();

		// Back into the string
		e--;

		// Skip white space at the end
		while ( e > s && tcTC( T, ' ' ) >= x_str[ e ] )
			e--;

		// Skip the white space at the beginning
		while ( s < e && tcTC( T, ' ' ) >= x_str[ s ] )
			s++;

		// Were we left with anything?
		if ( s >= e )
			return T_STR();

		// Return the string minus white space
		return T_STR( x_str, s, e - s + 1 );
	}

	template< typename T_STR >
		T_STR& RTrimInPlace( T_STR &s, typename T_STR::value_type c )
		{	return s.erase( s.find_last_not_of( c ) + 1 ); }

	template< typename T_STR >
		T_STR& RTrimInPlace( T_STR &s, const T_STR &t )
		{	return s.erase( s.find_last_not_of( t ) + 1 ); }

	template< typename T_STR >
		T_STR& LTrimInPlace( T_STR &s, typename T_STR::value_type c )
		{	return s.erase( 0, s.find_first_not_of( c ) ); }

	template< typename T_STR >
		T_STR& LTrimInPlace( T_STR &s, const T_STR &t )
		{	return s.erase( 0, s.find_first_not_of( t ) ); }

	template< typename T_STR >
		T_STR RTrim( T_STR s, typename T_STR::value_type c )
		{	return s.erase( s.find_last_not_of( c ) + 1 ); }

	template< typename T_STR >
		T_STR RTrim( T_STR s, const T_STR &t )
		{	return s.erase( s.find_last_not_of( t ) + 1 ); }

	template< typename T_STR >
		T_STR LTrim( T_STR s, typename T_STR::value_type c )
		{	return s.erase( 0, s.find_first_not_of( c ) ); }

	template< typename T_STR >
		T_STR LTrim( T_STR s, const T_STR &t )
		{	return s.erase( 0, s.find_first_not_of( t ) ); }

	template < typename T >
		T ReplaceStr( const T s, const T a, const T b )
		{	T _s( s );
			typename T::size_type i = 0;
			while( T::npos != ( i = _s.find_first_of( a, i ) ) )
				_s.replace( i, a.length(), b ), i += b.length();
			return _s;
		}

	template < typename T_STR >
		T_STR ReplaceChar( T_STR s, const typename T_STR::value_type a, const typename T_STR::value_type b )
		{	typename T_STR::size_type i = 0;
			while( T_STR::npos != ( i = s.find_first_of( a, i ) ) )
				s[ i++ ] = b;
			return s;
		}

	template < typename T_STR >
		T_STR& ReplaceCharInPlace( T_STR &s, const typename T_STR::value_type a, const typename T_STR::value_type b )
		{	typename T_STR::size_type i = 0;
			while( T_STR::npos != ( i = s.find_first_of( a, i ) ) )
				s[ i++ ] = b;
			return s;
		}

	template < typename T_STR >
		T_STR Token( const T_STR &s, const T_STR &seps )
	{	typename T_STR::size_type e = s.find_first_of( seps );
		if ( T_STR::npos == e )
			return s;
		return T_STR( s, 0, e );
	}

	template < typename T_STR >
		T_STR Token( const T_STR &s )
		{	typename T_STR::size_type i = 0, l = s.length();
			while( i < l && tcTC( typename T_STR::value_type, ' ' ) < s[ i ] )
				i++;
			return T_STR( s, 0, i );
		}

	/// Converts upper case letters to lower case
    /**
        \param [in] dst     -   String to modify
        \param [in] ln_dst  -   Length of string in dst
    */
    template< typename T, typename T_SIZE >
	    T* ToLower( T *dst, T_SIZE ln_dst )
	{
		if ( !dst || 0 >= ln_dst )
			return dst;

		while ( 0 < ln_dst-- )
		{
            if ( *dst >= tcTC( T, 'A' ) && *dst <= tcTC( T, 'Z' ) )
				*dst -= tcTC( T, 'A' ) - tcTC( T, 'a' );

			dst++;

		} // end while

        return dst;
	}

	template < typename T_STR >
		T_STR& ToLowerInPlace( T_STR &s )
	{	ToLower( &s[ 0 ], s.length() ); return s; }

	template < typename T_STR >
		T_STR ToLower( T_STR s )
	{	return ToLowerInPlace( s ); }

	/// Converts lower case letters to upper case
    /**
        \param [in] dst     -   String to modify
        \param [in] ln_dst  -   Length of string in dst
    */
    template< typename T, typename T_SIZE >
    	T* ToUpper( T *dst, T_SIZE ln_dst )
	{
		if ( !dst || 0 >= ln_dst )
			return dst;

		while ( 0 < ln_dst-- )
		{
            if ( *dst >= tcTT( T, 'a' ) && *dst <= tcTT( T, 'z' ) )
				*dst += tcTT( T, 'A' ) - tcTT( T, 'a' );

			dst++;

		} // end while

		return dst;
	}

	template < typename T_STR >
		T_STR& ToUpperInPlace( T_STR &s )
	{	ToUpper( &s[ 0 ], s.length() ); return s; }

	template < typename T_STR >
		T_STR ToUpper( T_STR s )
	{	return ToUpperInPlace( s ); }

    /// Returns the length of the null terminated string in s
    /**
        \param [in] s       -   Pointer to string buffer
	*/
	template < typename T, typename T_SIZE >
		static str::t_size Length( const T *x_pStr, T_SIZE ln_max )
	{
		if ( !x_pStr )
			return 0;

		str::t_size l = 0;
		while ( ln_max-- && *x_pStr )
			l++, x_pStr++;

		return l;
	}

}; // namespace str

/**
	@warning Functions in the zstr namespace depend on strings being null terminated.
*/
namespace zstr
{
    /// Returns the length of the null terminated string in s
    /**
        \param [in] s       -   Pointer to string buffer
	*/
	template < typename T >
		static str::t_size Length( const T *x_pStr )
	{
		if ( !x_pStr )
			return 0;

		str::t_size l = 0;
		while ( *x_pStr )
			l++, x_pStr++;

		return l;
	}

    /// Copies string, returns the number of bytes copied
    /**
        \param [out] dst    -   Buffer that receives the string copy
        \param [in] sz_dst  -   Size of the buffer in dst
        \param [in] src     -   Pointer to the string to copy
    */
    template < typename T, typename T_SZ >
        T_SZ Copy( T *dst, T_SZ sz_dst, const T *src )
        {
			if ( !dst || !src )
				return 0;

            if ( 0 >= sz_dst )
                return 0;

			T_SZ ln_src = zstr::Length( src );
            if ( ( ln_src + 1 ) > sz_dst )
                ln_src = sz_dst - 1;

            memcpy( dst, src, ln_src * sizeof( T ) );

            dst[ ln_src ] = 0;

            return ln_src;
        }

	template < typename T >
		long Compare( T *s1, T *s2 )
		{
			if ( !s1 || !s2 )
				return 0;

			long l1 = Length( s1 ), l2 = Length( s2 );
			if ( l1 != l2 )
				return l1 - l2;

			return memcmp( s1, s2, l1 * sizeof( T ) );
		}

	template < typename T >
		long Compare( T *s1, long l1, T *s2 )
		{
			if ( !s1 || !s2 )
				return 0;

			long l2 = Length( s2 );
			if ( l1 != l2 )
				return l1 - l2;

			return memcmp( s1, s2, l1 * sizeof( T ) );
		}

}; // namespace zstr

namespace str
{

	/// 'Fast' hex number to ascii conversion
    /**
        \param [out] b	-   Destination buffer
        \param [in] ch	-   Character to serialize
		\param [in] fix	-	If greater than zero, fixes number to specified size
    */
	template< typename T, typename N >
		void htoa( T *b, N ch, long fix = 0 )
		{
			T c;
			long sz = sizeof( N ) * 2;

			// Fixed size?
			if ( 0 < fix && sz > fix )
				sz = fix;

			// For each nibble
			for ( long i = 0; i < sz; i++ )
			{
				// Grab a nibble
				c = (T)( ch & 0x0f );
				ch >>= 4;

				if ( 9 >= c )
					b[ sz - i - 1 ] = tcTC( T, '0' ) + c;
				else
					b[ sz - i - 1 ] = tcTC( T, 'a' ) + ( c - 10 );

			} // end for

		}

	/// 'Fast' ascii to hex number conversion
    /**
        \param [out] b	-   Destination buffer
        \param [in] ch	-   Character to serialize
		\param [in] sz	-	Number of characters to process
    */
	template< typename T, typename N >
		N atoh( T *b, N *n, long sz )
		{
			// Initialize to zero
			*n = 0;

			// For each nibble
			for ( long i = 0; i < sz; i++ )
			{
				*n <<= 4;

				// Grab character
				const T c = *b; b++;
				if ( tcTC( T, '0' ) <= c && tcTC( T, '9' ) >= c )
					*n |= c - tcTC( T, '0' );
				else if ( tcTC( T, 'a' ) <= c && tcTC( T, 'f' ) >= c )
					*n |= c - tcTC( T, 'a' ) + 10;
				else if ( tcTC( T, 'A' ) <= c && tcTC( T, 'F' ) >= c )
					*n |= c - tcTC( T, 'A' ) + 10;

			} // end for

			return *n;
		}

	/// 'Fast' number to ascii conversion
    /**
        \param [out] b	-   Destination buffer
        \param [in] ch	-   Character to serialize
		\param [in] fix	-	If greater than zero, pads number to specified size
    */
	template< typename T, typename N >
		void dtoa( T *b, N ch, long fix = 0 )
		{
			T c;

			// For each nibble
			long i;
			for ( i = 0; ch || ( fix && i < fix ); i++ )
			{
				// Write digit
				b[ i++ ] = tcTC( T, '0' ) + ( ch % 10 );

				// Divide by 10
				ch /= 10;

			} // end for

			// Number is in reverse order
			cmn::RevBytes( b, i );
		}

	/// 'Fast' ascii to number conversion
    /**
        \param [out] b	-   Destination buffer
        \param [in] ch	-   Character to serialize
		\param [in] sz	-	Number of characters to process
    */
	template< typename T, typename N >
		long atod( T *b, N *n, long sz, int *pErr = 0 )
		{
			// Initialize to zero
			*n = 0;

			// For each character
			long i;
			for ( i = 0; i < sz; i++ )

				// Is it valid
				if ( tcTC( T, '0' ) <= b[ i ] && tcTC( T, '0' ) >= b[ i ] )
					*n *= 10, *n += b[ i ] - tcTC( T, '0' );

				// Are we just counting errors?
				else if ( pErr )
					(*pErr)++;

				// Punt
				else
					return i;

			return i;
		}

	/// Retuns the offset of ch
    /**
        \param [in] s   -   String in which to search
        \param [in] ln  -   Length of string in s
        \param [in] ch  -   Character to search for
    */
	template< typename T >
		t_size FindCharacter( const T *s, t_size ln, T ch )
		{
			if ( !s )
				return -1;

			t_size i = 0;
			while ( 0 < ln )
			{
				if ( *s == ch )
					return i;

				i++, s++; ln--;

			} // end while

			return -1;
		}

    /// Finds the first character in s2 that is in string s1
    /**
        \param [in] s1  -   String of characters in which to search
        \param [in] ln1 -   Length of the string in s1
        \param [in] s2  -   Character list
        \param [in] ln2 -   Number of characters in s2
    */
	template< typename T >
		t_size FindCharacters( const T *s1, t_size ln1, const T *s2, t_size ln2 )
	    {
			if ( !s1 || !s2 )
				return -1;

		    t_size i = 0;
		    while ( 0 < ln1 )
		    {
			    const T *start = s2;
                t_size ln_start = ln2;
			    while ( ln_start-- )
			    {	if ( *s1 == *start )
					    return i;
				    start++;
			    } // end while

			    i++; s1++; ln1--;

		    } // end while

		    return -1;
	    }

    /// Parses through the string looking for terminating characters
    /**
        \param [in] s       -   String of characters in which to search
        \param [in] ln      -   Length of the string in s
        \param [in] term    -   Terminator character list
        \param [in] ln_term -   Length of string in term
        \param [in] esc     -   Escape characters
        \param [in] ln_esc  -   Length of string in esc
    */
	template< typename T >
		t_size FindTerm( const T *s, t_size ln, const T *term, t_size ln_term, const T *esc = 0, t_size ln_esc = 0 )
	    {
			if ( !s || !term )
				return -1;

		    t_size i = 0, o = 0;
		    while ( 0 < ln )
		    {
			    // Finda terminator
			    i = FindCharacters( s, ln, term, ln_term );
			    if ( 0 > i )
				    return -1;

			    // Is it escaped?
			    if ( !i || !esc || 0 > FindCharacter( esc, ln_esc, s[ i - 1 ] ) )
				    return o + i;

			    // Next
			    s += i + 1;
				o += i + 1;

                ln -= i;

		    } // end while

		    return -1;
	    }

    /// Unquotes a string in place
    /**
        \param [in] s       -   Quoted string
        \param [in] ln      -   Length of the string in s
        \param [in] open    -   List of opening quotes
        \param [in] ln_open -   Length of string in term
        \param [in] close   -   List of closing quotes
        \param [in] ln_close-   Length of string in term
        \param [in] esc     -   Escape characters
        \param [in] ln_esc  -   Length of string in esc

        The opening and closing quote characters must correspond within the string.

        open    = "<{[(";
        close   = ">}])";

		@return The size of the new string

    */
	template< typename T >
		t_size UnquoteInPlace( T* s, t_size ln, const T* open, t_size ln_open, const T *close, t_size ln_close, const T *esc = 0, t_size ln_esc = 0 )
	    {
			if ( !s )
				return 0;

            // What quote character is being used
            t_size q = FindCharacter( open, ln_open, *s );

			// Is it quoted?
		    if ( 0 > q )
				return ln;

			// Destination
			T* d = s;
			t_size o = 0;

			// Skip the quote character
			s++; ln--;

			while ( 0 < ln )
			{
				// Is this an escape character?
				t_size e = FindCharacter( esc, ln_esc, *s );
				if ( 0 <= e )
				{	s++; ln--;
					if ( 0 < ln )
						d[ o++ ] = *s, s++, ln--;
				} // end if

				// Is it the end of the quoted string?
				else if ( *s == close[ q ] )
					ln = 0;

				// Just copy the character
				else
					d[ o++ ] = *s, s++, ln--;

			} // end while

			// Null terminate the string
			d[ o ] = 0;

			return o;
	    }

    /// Returns the offset of the first character in s1 not in s2
    /**
        \param [in] s1  -   String of characters in which to search
        \param [in] ln1 -   Length of the string in s1
        \param [in] s2  -   Character list
        \param [in] ln2 -   Number of characters in s2
    */
	template < typename T >
		static t_size SkipCharacters( T *s1, t_size ln1, const T *s2, t_size ln2 )
	{
		if ( !s1 || !ln1 || !s2 || !ln2 )
			return -1;

		t_size i = 0;
		while ( ln1 )
		{
			const T *start = s2;
			t_size ln_start = ln2;
			while ( start && ln_start-- )
				if ( *s1 == *start )
					start = 0;
				else
					start++;

			if ( start )
				return i;

			i++; s1++; ln1--;

		} // end while

		return i;
	}

    /// Finds the first character in s within the specified range
    /**
        \param [in] s       -   String of characters in which to search
        \param [in] ln      -   Length of the string in s
        \param [in] min     -   Minimum character value
        \param [in] max     -   Maximum character value
    */
	template< class T >
		static t_size FindInRange( const T *s, t_size ln, T min, T max )
	{
		if ( !s )
			return 0;

		t_size i = 0;
		while ( ln )
		{
			if ( *s >= min && *s <= max )
				return i;

			i++; s++; ln--;

		} // end while

		return -1;
	}

    /// Finds the end of a quoted sub string
    /**
        \param [in] s       -   String of characters in which to search
        \param [in] ln      -   Length of the string in s
        \param [in] open    -   List of opening quotes
        \param [in] ln_open -   Length of string in term
        \param [in] close   -   List of closing quotes
        \param [in] ln_close-   Length of string in term
        \param [in] esc     -   Escape characters
        \param [in] ln_esc  -   Length of string in esc

        The opening and closing quote characters must correspond within the string.

        open    = "<{[(";
        close   = ">}])";

    */
	template< typename T >
		t_size ParseWithQuoted( const T* s, t_size ln,
								const T *term, t_size ln_term,
							    const T* open, t_size ln_open,
								const T *close, t_size ln_close,
								const T *esc = 0, t_size ln_esc = 0 )
	{
		if ( !s || !term || !esc )
			return -1;

		t_size q = -1;
		t_size i = 0;
		while ( i < ln )
		{
			// Is this the terminator?
			if ( 0 <= FindCharacter( term, ln_term, *s ) )
				return i;

			// Is it an escape character?
			if ( esc && 0 <= FindCharacter( esc, ln_esc, *s ) )
			{
				i++; s++;

				if ( i >= ln )
					return -1;

			} // end if

			else
			{
				// Is it a 'start quote' character?
				q = FindCharacter( open, ln_open, *s );
				if ( 0 <= q )
				{
					// Skip open quote
					s++, i++;
					if ( i >= ln )
						return -1;

					// Find end quote
					t_size e = FindTerm( s, ln - i, &close[ q ], 1, esc, ln_esc );

					// Adjust if found
					if ( 0 <= e )
						i += e, s += e;

					// iii I'm choosing to ignore an unterminated quote,
					// 	   could also have just grabed the rest of the string

				} // end if

			} // end if

			// Next character
			i++; s++;

		} // end while

		return ln;
	}

	/// Splits a string into an array by cutting on any character in m_pSep, while respecting quotes
	/**
		@param[in,out] 	x_pStr		- String to be split
		@param[in]		x_nSize		- Size of the buffer at x_pStr
		@param[in]		x_pSep		- NULL terminated list of separators
		@param[in]		x_pOpen		- NULL terminated list of open quotes
		@param[in]		x_pClose 	- NULL terminated list of close quotes
		@param[in]		x_pEsc		- NULL terminated list of escape characters
		@param[in]		x_bInPlace	- If not-zero, the buffer is unquoted in place.
									  This destroys the content of x_pStr.

        The opening and closing quote characters must correspond within the string.

        open    = "<{[(";
        close   = ">}])";

		@warning If x_bInPlace is non-zero, this function destroys the contents of x_pStr while parsing.

		+++ Remove the dependency on null terminated strings
	*/
	template < typename T_STR, typename T_LST >
		static T_LST SplitQuoted( typename T_STR::value_type *x_pStr, t_size x_nSize,
								  const typename T_STR::value_type *x_pSep, const typename T_STR::value_type *x_pOpen,
								  const typename T_STR::value_type *x_pClose, const typename T_STR::value_type *x_pEsc,
								  bool x_bInPlace )
	{
		T_LST lst;

		// Sanity check
		if ( !x_pStr || 0 >= x_nSize )
			return lst;

		// Get sep string length
		t_size nSep = x_pSep ? zstr::Length( x_pSep ) : 0;
		if ( !nSep )
			return lst;

		t_size nOpen = x_pOpen ? zstr::Length( x_pOpen ) : 0;
		t_size nClose = x_pClose ? zstr::Length( x_pSep ) : 0;
		t_size nEsc = x_pEsc ? zstr::Length( x_pEsc ) : 0;

		// While we have bytes
		while ( x_nSize )
		{
			// Skip any separators
			t_size nSkip = SkipCharacters( x_pStr, x_nSize, x_pSep, nSep );
			if ( 0 <= nSkip )
				x_pStr += nSkip, x_nSize -= nSkip;

			// Are we done?
			if ( !x_nSize )
				return lst;

			// Find a closing separator
			t_size nPos = ParseWithQuoted( x_pStr, x_nSize, x_pSep, nSep,
										   x_pOpen, nOpen, x_pClose, nClose, x_pEsc, nEsc );

			// Did we find a separator?
			if ( 0 > nPos )
			{
				// Add what's left
				lst.push_back( T_STR( x_pStr, x_nSize ) );

				// This is the end
				return lst;

			} // end if

			else if ( nPos )
			{
				// Unquote the string and add it
				lst.push_back( T_STR( x_pStr, UnquoteInPlace( x_pStr, nPos, x_pOpen, nOpen, x_pClose, nClose, x_pEsc, nEsc ) ) );

				// Skip
				x_nSize -= nPos; x_pStr += nPos;

			} // end if

		} // end while

		return lst;

	}

	/// Attempts to match a string to the specified pattern
	/**
		@param [in] s			- String to search
		@param [in] ln_s		- Length of the string in s
		@param [in] pat			- Pattern to match
		@param [in] ln_pat		- Length of the string in pat
		@param [in] itnore_case	- If not zero, case will be ignored

		@return Returns less than zero if pattern is not matched.

		Examples

		@code

			// "pattern" -> "string"

			"" -> ""
			"*" -> "anything"
			"???" -> "123" "abc" "any"
			"hi*" -> "hi, then anything"
			"hello*world" -> "hello, then anything that ends with world"

		@endcode

	*/
	template< class T >
		t_size MatchPattern( const T *s, t_size ln_s, const T* pat, t_size ln_pat, bool ignore_case )
	{
		if ( !s || !pat )
			return -1;

		// Match empty string
		if ( 0 >= ln_pat )
			return ( 0 >= ln_s ) ? 0 : -1;

		t_size i = 0, p = 0;

		// Skip multiple '*'
		while ( p < ( ln_pat + 1 ) && pat[ p ] == tcTC( T, '*' ) && pat[ p + 1 ] == tcTC( T, '*' ) )
			p++;

		// Check for the 'any' pattern
		if ( pat[ p ] == tcTC( T, '*' ) && p + 1 >= ln_pat )
			return 0;

		// While we're not at the end
		while ( i < ln_s )
		{
			// Are we on a wildcard?
			if ( pat[ p ] == tcTC( T, '*' ) )
			{
				// Are we matching everything?
				if ( p + 1 >= ln_pat )
					return 0;

				// Check for pattern advance
				if ( s[ i ] == pat[ p + 1 ] ||
						( ignore_case &&
							(
								(
									s[ i ] >= tcTC( T, 'a' ) && s[ i ] <= tcTC( T, 'z' ) &&
									( s[ i ] - ( tcTC( T, 'a' ) - tcTC( T, 'A' ) ) ) == pat[ p + 1 ]
								) ||

								(
									s[ i ] >= tcTC( T, 'A' ) && s[ i ] <= tcTC( T, 'Z' ) &&
									( s[ i ] + ( tcTC( T, 'a' ) - tcTC( T, 'A' ) ) ) == pat[ p + 1 ]
								)
							)
						)
					) p += 2;

			} // end if

			// Just accept this character
			else if ( pat[ p ] == tcTC( T, '?' ) )
				p++;

			// Otherwise advance if equal
			else if ( s[ i ] == pat[ p ] )
				p++;

			// Case insensitive
			else if ( ignore_case &&
						(
							(
								s[ i ] >= tcTC( T, 'a' ) && s[ i ] <= tcTC( T, 'z' ) &&
								( s[ i ] - ( tcTC( T, 'a' ) - tcTC( T, 'A' ) ) ) == pat[ p ]
							) ||
							(
								s[ i ] >= tcTC( T, 'A' ) && s[ i ] <= tcTC( T, 'Z' ) &&
								( s[ i ] + ( tcTC( T, 'a' ) - tcTC( T, 'A' ) ) ) == pat[ p ]
							)
						)
					) p++;

			else
			{
				// Attempt to back up in the pattern
				while ( p && pat[ p ] != tcTC( T, '*' ) )
					p--;

				// Did we find the 'any' pattern?
				if ( pat[ p ] != tcTC( T, '*' ) )
					return -1;

			} // end else

			// Next char
			i++;

			// Are we at the end of the pattern?
			if ( p >= ln_pat  )
			{
				// Quit if at the end of the string too
				if ( i >= ln_s )
					return 0;

				// Attempt to back up in the pattern
				while ( p && pat[ p ] != tcTC( T, '*' ) )
					p--;

				// Did we find the 'any' pattern?
				if ( pat[ p ] != tcTC( T, '*' ) )
					return -1;

			} // end if

		} // end while

		// Skip wild cards
		while ( p < ln_pat && pat[ p ] == tcTC( T, '*' ) )
			p++;

		// Did we match?
		return ( p >= ln_pat ) ? 0 : -1;
	}

	/// Appends a size formatted string ( 1.3KB, 44.75GB, etc...)
	template< typename T_STR, typename T_NUM >
		T_STR SizeStr( T_NUM dSize, T_NUM dDiv, long nDigits, const typename T_STR::value_type ** pSuffix = tcNULL )
	{
		typedef typename T_STR::value_type T;

		T_STR s;
		long i = 0;
		static const T *sizes[] =
		{	tcTT( T, "" ), 			//
			tcTT( T, " K" ), 		// Kilo
			tcTT( T, " M" ), 		// Mega
			tcTT( T, " G" ), 		// Giga
			tcTT( T, " T" ), 		// Tera
			tcTT( T, " P" ),		// Peta
			tcTT( T, " E" ),		// Exa
			tcTT( T, " Z" ),		// Zetta
			tcTT( T, " Y" ),		// Yotta
			tcTT( T, " B" ),		// Bronto
			0 						// Geop, but G already taken?
									// Segan, ...
		};

		// Use 1024 as the default divider
		if ( 0 >= dDiv )
			dDiv = T_NUM( 1024 );

		bool bNeg = 0 > dSize;
		if ( bNeg )
			dSize = -dSize;

		// Use default suffixes if non provided
		if ( !pSuffix || !*pSuffix || !**pSuffix )
			pSuffix = sizes;

		// Which size to use?
		while ( dSize > dDiv && pSuffix[ i + 1 ] )
			i++, dSize /= dDiv;

		// Is the number negative?
		if ( bNeg )
			s = tcTT( T, "-" );

		// Special formating?
		if ( 0 > nDigits )
			s += ToString< T_STR >( dSize );
		else if ( !nDigits )
			s += ToString< T_STR >( (long)dSize );
		else
			s += StrFmt< T_STR >( ( T_STR( tcTT( T, "%." ) ) + ToString< T_STR >( nDigits ) + tcTT( T, "f" ) ).c_str(), dSize );

		// Build the string
		s += pSuffix[ i ];

		return s;
	}

	/// Returns the Levenshtein distance between the specified strings
	template < typename T_STR >
		typename T_STR::size_type levstr(const T_STR &s1, const T_STR &s2)
	{
		typename T_STR::size_type l1 = s1.length(), l2 = s2.length();
		std::vector< typename T_STR::size_type > d( ( l1 + 1 ) * ( l2 + 1 ) );

		typename T_STR::size_type i, j;
		for ( i = 0; i <= l1; i++ )
		    d[ i * l2 ] = i;

		for ( i = 0; i <= l2; i++ )
		    d[ i ] = i;

		for ( i = 1; i <= l1; i++ )
		    for ( j = 1; j <= l2; j++ )
		        d[ i * l2 + j ] = cmn::Min( cmn::Min( d[ ( i - 1 ) * l2 + j ] + 1, d[ i * l2 + ( j - 1 ) ] + 1 ),
		                                  	d[ ( i - 1 ) * l2 + ( j - 1 ) ] + ( s1[ i - 1 ] == s2[ j - 1 ] ? 0 : 1 )
			                              );

		return d[ ( l1 * l2 ) + l2 ];
	}

	/// Returns the Levenshtein distance between the specified split paths
	template < typename T_STR, typename T_LST >
		typename T_STR::size_type levpath(const T_LST &lst1, const T_LST &lst2)
	{
		typename T_STR::size_type l1 = lst1.size(), l2 = lst2.size();
		std::vector< typename T_STR::size_type > d( ( l1 + 1 ) * ( l2 + 1 ) );

		typename T_STR::size_type i, j;
		for ( i = 0; i <= l1; i++ )
		    d[ i * l2 ] = i;

		for ( i = 0; i <= l2; i++ )
		    d[ i ] = i;

		for ( i = 1; i <= l1; i++ )
		    for ( j = 1; j <= l2; j++ )
		        d[ i * l2 + j ] = cmn::Min( cmn::Min( d[ ( i - 1 ) * l2 + j ] + 1, d[ i * l2 + ( j - 1 ) ] + 1 ),
											d[ ( i - 1 ) * l2 + ( j - 1 ) ] + levstr( lst1[ i - 1 ], lst2[ j - 1 ] )
										  );

		return d[ ( l1 * l2 ) + l2 ];
	}

	/// Calculates 32 bit CRC
	unsigned int CRC32( void *x_buf, unsigned long x_size, unsigned int x_crc = 0 );

}; // namespace str


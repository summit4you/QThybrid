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

#include "str.h"

// +++ Rewrite all these to use join/split method

namespace parser
{
	template< typename T_LST >
		T_LST Split( const typename T_LST::value_type &s, const typename T_LST::value_type &div )
	{
		typedef typename T_LST::value_type T_STR;

		T_LST lst;

		// Explode?
		if ( !div.length() )
		{
			typename T_STR::size_type i = 0;
			while ( i < s.length() )
				lst.push_back( T_STR( s, i, 1 ) );

		} // end if

		// Split on divider
		else
		{
			typename T_STR::size_type start = 0, end = 0;
			while ( T_STR::npos != end )
			{
				end = s.find_first_of( div, start );
				if ( T_STR::npos != end )
					lst.push_back( T_STR( s, start, end - start ) ), start = end + div.length();
				else
					lst.push_back( T_STR( s, start ) );

			} // end while

		} // end else

		return lst;

	}

//------------------------------------------------------------------
// C++ encode / decode
//------------------------------------------------------------------

	/// Returns non-zero if the character is a valid character
	template< typename T >
		static bool IsCppChar( T x_ch )
	{
		switch( x_ch )
		{	case tcTC( T, '"' ) :
			case tcTC( T, '\\' ) :
				return false;
		} // end switch

		return ( 0 > x_ch || tcTC( T, ' ' ) <= x_ch ) ? true : false;
	}

	template< typename T_STR >
		static T_STR CppEncodeChar( typename T_STR::value_type x_ch )
	{
		typedef typename T_STR::value_type T;

		switch( x_ch )
		{
			case tcTC( T, '"' ) :
				return tcTT( T, "\\\"" );

			case tcTC( T, '\'' ) :
				return tcTT( T, "\\\\" );

			case tcTC( T, '\t' ) :
				return tcTT( T, "\\t" );

			case tcTC( T, '\r' ) :
				return tcTT( T, "\\r" );

			case tcTC( T, '\n' ) :
				return tcTT( T, "\\n" );

		} // end switch

		// Convert to two byte character
		T s[ 16 ] = { '"', ' ', '"', '\\', 'x', 0, 0, '"', ' ', '"', 0 };
		str::htoa< char >( &s[ 5 ], (char)x_ch );

		return T_STR( s, 10 );
	}

	template< typename T_STR >
		static T_STR CppEncode( const typename T_STR::value_type *x_pStr, typename T_STR::size_type x_lSize = 0 )
	{
		typedef typename T_STR::value_type T;

		if ( !x_pStr || !*x_pStr || 0 >= x_lSize )
			return T_STR();

		T_STR ret;
		typename T_STR::size_type nStart = 0, nPos = 0;

		while ( nPos < x_lSize )
		{
			// Must we encode this one?
			if ( !IsCppChar( x_pStr[ nPos ] ) )
			{
				// Copy data that's ok
				if ( nStart < nPos )
					ret.append( &x_pStr[ nStart ], nPos - nStart );

				// Encode this character
				ret.append( CppEncodeChar< T_STR >( x_pStr[ nPos ] ) );

				// Next
				nStart = ++nPos;

			} // end if

			else
				nPos++;

		} // end while

		// Copy remaining data
		if ( nStart < nPos )
			ret.append( &x_pStr[ nStart ], nPos - nStart );

		return ret;
	}

	template< typename T_STR >
		static T_STR CppEncode( const T_STR &x_str )
	{	return CppEncode( x_str.data(), x_str.length() ); }


//------------------------------------------------------------------
// Http Headers encode / decode
//------------------------------------------------------------------

	/// Returns non-zero if the character is a valid url character
	template< typename T >
		static bool IsHttpHeaderChar( T x_ch )
	{	return x_ch != tcTC( T, '\r' ) && x_ch != tcTC( T, '\n' ); }

	/// Encodes the string so it is safe to put into an http header field
	template< typename T_STR >
		static T_STR EncodeHttpHeaderStr( const T_STR &x_str )
	{
		typedef typename T_STR::value_type T;

		const T *p = &x_str[ 0 ];
		typename T_STR::size_type nLen = x_str.length();

		// Sanity check
		if ( !p || 0 >= nLen )
			return T_STR();

		// Need at least this much space
		T_STR ret;
		ret.reserve( nLen );

		while ( 0 < nLen-- )
		{
			// +++ Replace CR & LF with spaces, not sure what else to do here?
			if ( !IsHttpHeaderChar( *p ) )
				ret += tcTC( T, ' ' );
			else
				ret += *p;

			// Next character
			p++;

		} // end while

		return ret;
	}

	/// Decodes the string from an http header field
	template< typename T_STR >
		static T_STR DecodeHttpHeaderStr( const T_STR &x_str )
	{	return str::TrimWsInPlace( x_str ); }

	/// Decodes a single http header name/value pair
	template< typename T_PB >
		static long DecodeHttpHeaderPair( const typename T_PB::t_String &x_sStr, T_PB &x_pb )
	{
		typedef typename T_PB::t_String T_STR;
		typedef typename T_STR::value_type T;

		// Did we get valid data?
		if ( !x_sStr.length() )
			return 0;

		// Find pair separator
		typename T_STR::size_type pos = x_sStr.find_first_of( tcTC( T, ':' ) );
		if ( T_STR::npos == pos )
			x_pb[ str::ToLower( DecodeHttpHeaderStr( x_sStr ) ) ] = tcTT( T, "" );
		else
			x_pb[ str::ToLower( DecodeHttpHeaderStr( T_STR( x_sStr, 0, pos ) ) ) ]
				= DecodeHttpHeaderStr( T_STR( x_sStr, pos + 1 ) );

		return 1;
	}

	/// Decodes a set of http headers
	template< typename T_PB >
		static long DecodeHttpHeaders( const typename T_PB::t_String &x_sStr, T_PB &x_pb, bool x_bMerge = false )
	{
		typedef typename T_PB::t_String T_STR;
		typedef typename T_STR::value_type T;

		if ( !x_bMerge )
			x_pb.clear();

		long i = 0;
		typename T_STR::size_type pos = 0, len = x_sStr.length();
		while ( T_STR::npos != pos && pos < len )
		{
			// Start of pair
			typename T_STR::size_type start = pos;
			typename T_STR::size_type next = pos;

			// Find the end of the pair
			do
			{
				// Potential end
				pos = x_sStr.find_first_of( tcTT( T, "\r\n" ), next );

				// End of data?
				if ( T_STR::npos == pos )
					next = pos;

				// Find the end of this marker
				else
					next = x_sStr.find_first_not_of( tcTT( T, "\r\n" ), pos );

			// If the next line starts with white space, then the variable keeps going
			} while ( T_STR::npos != next && x_sStr[ next ] <= tcTC( T, ' ' ) );

			// Decode the pair
			if ( T_STR::npos == pos )
				i += DecodeHttpHeaderPair( T_STR( x_sStr, start ), x_pb );
			else
				i += DecodeHttpHeaderPair( T_STR( x_sStr, start, pos - start ), x_pb ), pos++;

			// Skip to start of next pair
			pos = next;

		} // end while

		return i;
	}

	/// Decodes the specified http headers into a property bag
	template< typename T_PB >
		static T_PB DecodeHttpHeaders( const typename T_PB::t_String &x_str )
		{	T_PB pb; DecodeHttpHeaders( x_str, pb ); return pb; }

	/// Encodes the property bag into http headers
	template< typename T_PB >
		static typename T_PB::t_String EncodeHttpHeaders( const T_PB &x_pb )
	{
		typedef typename T_PB::t_String T_STR;
		typedef typename T_STR::value_type T;

		T_STR s;
		for ( typename T_PB::const_iterator it = x_pb.begin(); x_pb.end() != it; it++ )
		{
			// Put break between headers
			if ( s.length() )
				s += tcTT( T, "\n" );

			// +++ Can't have a : in key, it would seem http headers encoding is far from perfect
			s += EncodeHttpHeaderStr( str::ReplaceChar( T_STR( it->first ), tcTC( T, ':' ), tcTC( T, '.' ) ) )
				 + tcTT( T, ": " )
				 + EncodeHttpHeaderStr( it->second->str() );

		} // end if

		return s;
	}


//------------------------------------------------------------------
// JSON encode / decode
//------------------------------------------------------------------

	template< typename T_STR >
		static T_STR EncodeJsonStr( const T_STR &x_str )
	{
		typedef typename T_STR::value_type T;

		// Sanity check
		const T *p = &x_str[ 0 ];
		typename T_STR::size_type nLen = x_str.length();
		if ( !p || 0 >= nLen )
			return T_STR();

		// Need at least this much space
		T_STR ret;
		ret.reserve( nLen );

		// Hex string buffer
		T hex[] = { tcTC( T, '\\' ), tcTC( T, 'u' ), 0, 0, 0, 0, 0 };

		while ( 0 < nLen-- )
		{
			switch( *p )
			{
				case tcTC( T, '\"' ) : ret += tcTT( T, "\\\"" ); break;
				case tcTC( T, '\\' ) : ret += tcTT( T, "\\\\" ); break;
				case tcTC( T, '\b' ) : ret += tcTT( T, "\\b" ); break;
				case tcTC( T, '\f' ) : ret += tcTT( T, "\\f" ); break;
				case tcTC( T, '\n' ) : ret += tcTT( T, "\\n" ); break;
				case tcTC( T, '\r' ) : ret += tcTT( T, "\\r" ); break;
				case tcTC( T, '\t' ) : ret += tcTT( T, "\\t" ); break;
				default :
					if ( *p < tcTC( T, ' ' ) || *p > tcTC( T, '~' ) )
						str::htoa( &hex[ 2 ], (unsigned short)*p ), ret.append( hex, 6 );
					else
						ret += *p;
					break;

			} // end switch

			// Next character
			p++;

		} // end while

		return ret;
	}

	template< typename T_PB >
		static typename T_PB::t_String EncodeJson( const T_PB &x_pb, long x_depth = 0 )
	{
		typedef typename T_PB::t_String T_STR;
		typedef typename T_STR::value_type T;

		T_STR sTab, sTab1;
		for ( long t = 0; t < x_depth; t++ )
			sTab += tcTC( T, '\t' );
		sTab1 += tcTC( T, '\t' ), sTab1 += sTab;

		bool bList = x_pb.is_list();
		typename T_STR::size_type i = 0;
		T_STR sStr; sStr += bList ? tcTC( T, '[' ) : tcTC( T, '{' ); sStr += tcTTEXT( T, tcNL8 );

		// stdForeach( typename T_PB::const_iterator, it, x_pb )
		for ( typename T_PB::const_iterator it = x_pb.begin(); it != x_pb.end(); it++ )
		{
			// Add comma if needed
			if ( !i )
				i++;
			else
				sStr += tcTT( T, "," ), sStr += tcTTEXT( T, tcNL8 );

			sStr += sTab1;

			// Add key
			if ( !bList )
				sStr += tcTC( T, '\"' ),
				sStr += EncodeJsonStr( it->first ),
				sStr += tcTT( T, "\": " );

			// Recurse for array
			if ( it->second->size() )
				sStr += EncodeJson( *it->second, x_depth + 1 );

			// Single value
			else if ( it->second->length() )
				sStr += tcTC( T, '\"' ), sStr += EncodeJsonStr( it->second->str() ), sStr += tcTC( T, '\"' );

			// Empty
			else
				sStr += tcTT( T, "\"\"" );

		} // end for

		sStr += tcNL8; sStr += sTab; sStr += bList ? tcTC( T, ']' ) : tcTC( T, '}' );

		return sStr;
	}

	template< typename T_STR >
		static T_STR DecodeJsonStr( const T_STR &x_str )
	{
		typedef typename T_STR::value_type T;

		const T *p = &x_str[ 0 ];
		typename T_STR::size_type nLen = x_str.length();
		if ( !p || 0 >= nLen )
			return T_STR();

		T_STR ret;
		unsigned long n;

		while ( 0 < nLen-- )
		{
			// Escaped?
			if ( tcTC( T, '\\' ) == *p )
			{
				p++, nLen--;
				if ( 0 < nLen )
				{	switch( *p )
					{
						case tcTC( T, '"' ) :	ret += tcTC( T, '"' ); break;
						case tcTC( T, '\'' ) :	ret += tcTC( T, '\'' ); break;
						case tcTC( T, '\\' ) :	ret += tcTC( T, '\\' ); break;
						case tcTC( T, '/' ) :	ret += tcTC( T, '/' ); break;
						case tcTC( T, 'b' ) :	ret += tcTC( T, '\b' ); break;
						case tcTC( T, 'f' ) :	ret += tcTC( T, '\f' ); break;
						case tcTC( T, 'n' ) :	ret += tcTC( T, '\n' ); break;
						case tcTC( T, 'r' ) :	ret += tcTC( T, '\r' ); break;
						case tcTC( T, 't' ) :	ret += tcTC( T, '\t' ); break;
						case tcTC( T, 'v' ) :	ret += tcTC( T, '\v' ); break;
						case tcTC( T, 'u' ) :
							if ( 4 <= nLen )
								p++, ret += (T)str::atoh( p, &n, 4 ), nLen -= 4;
							break;
						default: break;
					} // end switch

					p++;

				} // end if

			} // end if

			else
				ret += *p; p++;

		} // end while

		return ret;
	}

	template< typename T_PB >
		static typename T_PB::t_String::size_type DecodeJson( const typename T_PB::t_String &x_str, T_PB &x_pb, long x_lArrayType = 0, bool x_bMerge = false, long *x_pLast = 0 )
	{
		typedef typename T_PB::t_String T_STR;
		typedef typename T_STR::value_type T;

		const T *p = &x_str[ 0 ];
		typename T_STR::size_type nLen = x_str.length();

		// Lose previous contents
		if ( !x_bMerge )
			x_pb.clear();

		// Sanity check
		if ( !p || 0 >= nLen )
			return 0;

		// Find start of array
		typename T_STR::size_type nPos = 0;
		while ( !x_lArrayType && nPos < nLen )
		{
			// Look for array start character
			switch( *p )
			{	case tcTC( T, '{' ) : x_lArrayType = 1; break;
				case tcTC( T, '[' ) : x_lArrayType = 2; break;
			} // end switch

			// Next character
			p++, nPos++;

		} // end while

		// Unknown type
		if ( !x_lArrayType )
			return nPos;

		// Mark list
		if ( 2 == x_lArrayType )
			x_pb.is_list( true );

		T_STR sKey;
		long lMode = 0, lItems = 0;
		while ( nPos < nLen )
		{
			// Skip whitespace
			typename T_STR::size_type nSkip = str::FindInRange( p, nLen - nPos, tcTC( T, '!' ), tcTC( T, '~' ) );
			if ( 0 > nSkip || nSkip >= ( nLen - nPos ) )
				return nPos;
			p += nSkip, nPos += nSkip;

			// The character we will deal with
			const T ch = *p;

			// Check for array
			if ( tcTC( T, '{' ) == ch || tcTC( T, '[' ) == ch )
			{
				p++;

				if ( !lMode )
					sKey = str::ToString< T_STR >( lItems++ );

//				str::t_size sz = _DecodeJson( p, nLen - nPos, x_pb[ sKey ], tcTC( T, '{' ) == ch ? 1 : 2 );
				typename T_STR::size_type sz = DecodeJson( T_STR( p, nLen - nPos ), x_pb[ sKey ], tcTC( T, '{' ) == ch ? 1 : 2 );
				if ( 0 <= sz )
					p += sz, nPos += sz;

				lMode = 0;

			} // end if

			// Key / Value separator
			else if ( tcTC( T, ':' ) == ch )
				p++, nPos++;

			// Value separator / end array
			else if ( tcTC( T, ',' ) == ch || tcTC( T, '}' ) == ch || tcTC( T, ']' ) == ch )
			{
				if ( lMode )
				{
					if ( 1 == x_lArrayType )
						x_pb[ sKey ] = tcTT( T, "" );
					else
						x_pb[ str::ToString< T_STR >( lItems ) ] = sKey;

					// Count an item
					lItems++;

					// Nothing read yet
					lMode = 0;

				} // end if

				// Skip
				p++, nPos++;

				// End array
				if ( tcTC( T, '}' ) == ch || tcTC( T, ']' ) == ch )
					return nPos;

			} // end else if

			// Parse token
			else if ( tcTC( T, ' ' ) < ch )
			{
				// Find the end of the token
				str::t_size end = ( tcTC( T, '"' ) == ch )
								  ? str::FindTerm( ++p, nLen - ++nPos, tcTT( T, "\"" ), 1, tcTT( T, "\\" ), 1 )
								  : str::FindTerm( p, nLen - nPos, tcTT( T, ",:{}\r\n\t\"" ), 8, tcTT( T, "" ), 0 );

				// Key?
				if ( !lMode )
					lMode = 1,
					sKey = ( 0 < end ) ? DecodeJsonStr( T_STR( p, end) ) : T_STR();

				// Value?
				else if ( lMode )
					lItems++,
					lMode = ( 1 == lMode ) ? 0 : lMode,
					x_pb[ sKey ] = ( 0 < end ) ? DecodeJsonStr( T_STR( p, end ) ) : T_STR();

				// Skip string
				p += end, nPos += end;

				// Skip closing quote
				if ( tcTC( T, '"' ) == *p )
					p++, nPos++;

			} // end if

			// Skip white space
			else
				p++, nPos++;

		} // end while

		return nPos;
	}

	template< typename T_PB >
		static T_PB DecodeJson( const typename T_PB::t_String &x_str, long x_lArrayType = 0, bool x_bMerge = false, long *x_pLast = 0 )
		{	T_PB pb; DecodeJson( x_str, pb, x_lArrayType, x_bMerge, x_pLast ); return pb; }


//------------------------------------------------------------------
// URL encode / decode
//------------------------------------------------------------------

	/// Returns non-zero if the character is a valid url character
	template< typename T >
		static bool IsUrlChar( T x_ch )
	{   return  ( tcTC( T, 'a' ) <= x_ch && tcTC( T, 'z' ) >= x_ch ) ||
				( tcTC( T, 'A' ) <= x_ch && tcTC( T, 'Z' ) >= x_ch ) ||
				( tcTC( T, '0' ) <= x_ch && tcTC( T, '9' ) >= x_ch ) ||
				tcTC( T, '_' ) == x_ch || tcTC( T, '-' ) == x_ch ||
				tcTC( T, '.' ) == x_ch;
	}

	template< typename T_STR >
		static T_STR EncodeUrlStr( const T_STR &x_str )
	{
		typedef typename T_STR::value_type T;

		const T *p = &x_str[ 0 ];
		typename T_STR::size_type nLen = x_str.length();

		// Sanity check
		if ( !p || 0 >= nLen )
			return T_STR();

		// Need at least this much space
		T_STR ret;
		ret.reserve( nLen );

		// Hex string buffer
		T hex[] = { tcTC( T, '%' ), 0, 0, 0 };

		while ( 0 < nLen-- )
		{
			if ( !IsUrlChar( *p ) )
				str::htoa( &hex[ 1 ], (unsigned char)*p ), ret.append( hex, 3 );
			else
				ret += *p;

			// Next character
			p++;

		} // end while

		return ret;
	}

	template< typename T_STR >
		static T_STR DecodeUrlStr( const T_STR &x_str )
	{
		typedef typename T_STR::value_type T;

		const T *p = &x_str[ 0 ];
		typename T_STR::size_type nLen = x_str.length();

		if ( !p || 0 >= nLen )
			return T_STR();

		T ch = 0;
		T_STR ret;
		while ( 0 < nLen-- )
		{
			if ( tcTC( T, '+' ) == *p )
				ret += tcTC( T, ' ' );

			else if ( tcTC( T, '%' ) != *p )
				ret += *p;

			else
				p++, ret += str::atoh( p++, &ch, 2 ), nLen -= 2;

			p++;

		} // end while

		return ret;
	}

	template< typename T_PB >
		static typename T_PB::t_String EncodeUrl( const T_PB &x_pb )
	{
		typedef typename T_PB::t_String T_STR;
		typedef typename T_STR::value_type T;

		T_STR s;
		for ( typename T_PB::const_iterator it = x_pb.begin(); x_pb.end() != it; it++ )
		{
			if ( s.length() )
				s += tcTC( T, '&' );

			s += EncodeUrlStr( it->first )
				 + tcTC( T, '=' )
				 + EncodeUrlStr( it->second->str() );

		} // end if

		return s;
	}

	template< typename T_PB >
		static long DecodeUrlPair( const typename T_PB::t_String &x_sStr, T_PB &x_pb )
	{
		typedef typename T_PB::t_String T_STR;
		typedef typename T_STR::value_type T;

		if ( !x_sStr.length() )
			return 0;
		typename T_STR::size_type pos = x_sStr.find_first_of( tcTC( T, '=' ) );
		if ( T_STR::npos == pos )
			x_pb[ DecodeUrlStr( x_sStr ) ] = tcTT( T, "" );
		else
			x_pb[ DecodeUrlStr( T_STR( x_sStr, 0, pos ) ) ] = DecodeUrlStr( T_STR( x_sStr, pos + 1 ) );
		return 1;
	}

	template< typename T_PB >
		static long DecodeUrl( const typename T_PB::t_String &x_sStr, T_PB &x_pb, bool x_bMerge = false )
	{
		typedef typename T_PB::t_String T_STR;
		typedef typename T_STR::value_type T;

		if ( !x_bMerge )
			x_pb.clear();

		long i = 0;
		typename T_STR::size_type pos = 0, len = x_sStr.length();
		while ( T_STR::npos != pos && pos < len )
		{
			// Find pair sep
			typename T_STR::size_type start = pos;
			pos = x_sStr.find_first_of( tcTC( T, '&' ), pos );
			if ( T_STR::npos == pos )
				i += DecodeUrlPair( T_STR( x_sStr, start ), x_pb );
			else
				i += DecodeUrlPair( T_STR( x_sStr, start, pos - start ), x_pb ), pos++;

		} // end while

		return i;
	}

	template< typename T_PB >
		static T_PB DecodeUrl( const typename T_PB::t_String &x_str )
		{	T_PB pb; DecodeUrl( x_str, pb ); return pb; }

//------------------------------------------------------------------
// URI encode / decode
//------------------------------------------------------------------

	template< typename T_PB >
		static typename T_PB::t_String EncodeUri( T_PB &x_pb )
	{
		typedef typename T_PB::t_String T_STR;
		typedef typename T_STR::value_type T;

		T_STR str;

		// Scheme
		if ( x_pb[ tcTT( T, "scheme" ) ].length() )
			str += x_pb[ tcTT( T, "scheme" ) ].str(), str += "://";

		// Username and password?
		if ( x_pb[ tcTT( T, "username" ) ].length() )
		{
			if ( x_pb[ tcTT( T, "password" ) ].length() )
				str += x_pb[ tcTT( T, "username" ) ].str(),
				str += ":",
				str += x_pb[ tcTT( T, "password" ) ].str(),
				str += "@";
			else
				str += x_pb[ tcTT( T, "username" ) ].str(), str += tcTT( T, "@" );

		} // end if

		// Username and password?
		if ( x_pb[ tcTT( T, "host" ) ].length() )
		{
			if ( x_pb[ tcTT( T, "port" ) ].length() )
				str += x_pb[ tcTT( T, "host" ) ].str(),
				str += ":",
				str += x_pb[ tcTT( T, "port" ) ].str();
			else
				str += x_pb[ tcTT( T, "host" ) ].str();

		} // end if

		// Ensure separator
		if ( '/' != x_pb[ tcTT( T, "path" ) ].str()[ 0 ]
			 && '\\' != x_pb[ tcTT( T, "path" ) ].str()[ 0 ] )
			str += '/';

		// Append the path
		str += x_pb[ tcTT( T, "path" ) ].str();

		// Adding separator
		if ( x_pb[ tcTT( T, "get" ) ].length() )
			str += tcTT( T, "?" ), str += x_pb[ tcTT( T, "get" ) ].str();

		// Adding fragment
		if ( x_pb[ tcTT( T, "fragment" ) ].length() )
			str += tcTT( T, "#" ), str += x_pb[ tcTT( T, "fragment" ) ].str();

		return str;
	}

	template< typename T_PB >
		static long DecodeUri( const typename T_PB::t_String &x_sStr, T_PB &x_pb, int x_bMerge = 0 )
	{
		typedef typename T_PB::t_String T_STR;
		typedef typename T_STR::value_type T;
		typedef typename T_STR::size_type SZ;

		if ( !x_bMerge )
			x_pb.clear();

		// Anything to do?
		if ( !x_sStr.length() )
			return 0;

		// Position
		SZ pos, start = 0;

		// Temp buffer
		T_STR tmp;

		// Read in the scheme
		pos = x_sStr.find( tcTT( T, "://" ), start );
		if ( T_STR::npos != pos )
		{
			// Copy the sceme
			x_pb[ tcTT( T, "scheme" ) ] = T_STR( x_sStr, 0, pos );

			// Skip the scheme
			start = pos + 3;

		} // end if

		// Is there a username / password?
		pos = x_sStr.find_first_of( tcTC( T, '@' ), start );
		if ( T_STR::npos != pos )
		{
			// Copy username:password
			tmp.assign( x_sStr, start, pos - start );

			// Skip u/p
			start = pos + 1;

			// username and password, or just username?
			pos = tmp.find_first_of( tcTC( T, ':' ) );
			if ( T_STR::npos != pos )
				x_pb[ tcTT( T, "username" ) ] = T_STR( tmp, 0, pos ),
				x_pb[ tcTT( T, "password" ) ] = T_STR( tmp, pos + 1, T_STR::npos );
			else
				x_pb[ tcTT( T, "username" ) ] = tmp;

		} // end if

		// Parse the host part
		pos = x_sStr.find_first_of( tcTC( T, '/' ), start );
		if ( T_STR::npos == pos )
			tmp.assign( x_sStr, start, T_STR::npos ), start = T_STR::npos;
		else if ( start < pos )
			tmp.assign( x_sStr, start, pos - start ), start = pos;

		if ( tmp.length() )
		{
			// host:port?
			pos = tmp.find_first_of( tcTC( T, ':' ) );
			if ( T_STR::npos != pos )
				x_pb[ tcTT( T, "host" ) ] = T_STR( tmp, 0, pos ),
				x_pb[ tcTT( T, "port" ) ] = T_STR( tmp, pos + 1, T_STR::npos );
			else
				x_pb[ tcTT( T, "host" ) ] = tmp;

		} // end if

		// Is that all?
		if ( T_STR::npos == start )
			return 1;

		// Parse the path
		pos = x_sStr.find_first_of( tcTC( T, '?' ), start );
		if ( T_STR::npos == pos )
			x_pb[ tcTT( T, "path" ) ] = T_STR( x_sStr, start, T_STR::npos );
		else if ( start < pos )
			x_pb[ tcTT( T, "path" ) ] = T_STR( x_sStr, start, pos - start ), start = pos + 1;

		// Is that all?
		if ( T_STR::npos == pos )
			return 1;

		// Parse the get params
		pos = x_sStr.find_first_of( tcTC( T, '#' ), start );
		if ( T_STR::npos == pos )
			x_pb[ tcTT( T, "get" ) ] = T_STR( x_sStr, start, T_STR::npos );
		else if ( start < pos )
			x_pb[ tcTT( T, "get" ) ] = T_STR( x_sStr, start, pos - start ), start = pos + 1;

		// Is that all?
		if ( T_STR::npos == pos )
			return 1;

		// Whatever is left is the fragment
		x_pb[ tcTT( T, "fragment" ) ] = T_STR( x_sStr, start, T_STR::npos );

		return 1;
	}

	template< typename T_PB >
		static T_PB DecodeUri( const typename T_PB::t_String &x_sStr )
	{	T_PB pb; DecodeUri( x_sStr, pb ); return pb; }

//------------------------------------------------------------------
// URI encode / decode
//------------------------------------------------------------------

	/// Decode MIME variables
	template< typename T_PB >
		static T_PB DecodeMime( const typename T_PB::t_String &x_s, int x_bCaseSensitive = 0 )
	{
		typedef typename T_PB::t_String T_STR;
		typedef typename T_STR::value_type T;

		T_PB pb;
		typename T_STR::size_type s = 0, e = 0;
		while ( T_STR::npos != s )
		{
			T_STR sKey, sVal;

			// Skip leading white space
			s = x_s.find_first_not_of( tcTT( T, "\r\n" ), s );

			// Find the key/value sep
			e = x_s.find_first_of( tcTT( T, ":\r\n" ), s );

			// Grab the key
			if ( T_STR::npos != e )
				sKey = T_STR( x_s, s, e - s );
			else
				sKey = T_STR( x_s, s );

			// Trim white space
			str::TrimWsInPlace( sKey );

			// Change to lower case if not case sensitive
			if ( !x_bCaseSensitive )
				sKey = str::ToLowerInPlace( sKey );

			// End of the string?
			if ( T_STR::npos == e )
				s = T_STR::npos;

			// No value separator found?
			else if ( tcTC( T, ':' ) != x_s[ e ] )
				s = x_s.find_first_not_of( tcTT( T, "\r\n" ), e );

			// Copy the value
			else
			{
				// Skip value sep
				s = e + 1;

				do
				{
					// Find the end of the line
					e = x_s.find_first_of( tcTT( T, "\r\n" ), s );

					// End of string?
					if ( T_STR::npos == e )
						sVal += T_STR( x_s, s ), s = e;

					// See if the value continues on the next line
					else
					{
						// Peel off this part of the value
						sVal += T_STR( x_s, s, e - s );

						// Next non-empty line
						s = x_s.find_first_not_of( tcTT( T, "\r\n" ), e );

						// End of string?
						if ( T_STR::npos == s )
							e = s;

						// If it's not white space, it's a new key/value pair
						else if ( tcTC( T, ' ' ) < x_s[ s ] )
							e = T_STR::npos;

						// Skip leading white space
						else
							while ( s < x_s.length() && tcTC( T, ' ' ) >= x_s[ s ] )
								e = ++s;

					} // end if

				} while ( T_STR::npos != e && e < x_s.length() );

			} // end if

			// Save item
			if ( sKey.length() )
				pb[ sKey ] = str::TrimWsInPlace( sVal );

		} // end while

		return pb;
	}

	/// Encode MIME variables
	template< typename T_PB >
		static typename T_PB::t_String EncodeMime( T_PB &x_pb, int x_bCaseSensitive )
	{
		typedef typename T_PB::t_String T_STR;
		typedef typename T_STR::value_type T;

		T_STR str;

		// Write out each value
		if ( x_bCaseSensitive )
		{	for( typename T_PB::iterator it = x_pb.begin(); x_pb.end() != it; it++ )
				if ( it->first.length() && it->second->str().length() )
					str += it->first, str += tcTT( T, ": " ), str += it->second->str(), str += tcTT( T, "\r\n" );
		} // end if

		else
		{	for( typename T_PB::iterator it = x_pb.begin(); x_pb.end() != it; it++ )
				if ( it->first.length() && it->second->str().length() )
					str += str::ToLower( it->first ), str += tcTT( T, ": " ), str += it->second->str(), str += tcTT( T, "\r\n" );
		} // end else

		return str;
	}


}; // namespace parser

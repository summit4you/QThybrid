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

namespace uid
{
	/// Define UID
	struct UID
	{
		union
		{
			struct
			{
				unsigned int	Data1;
				unsigned short	Data2;
				unsigned short	Data3;
				unsigned char	Data4[ 8 ];
			};

			struct
			{
				unsigned int	u1;
				unsigned int	u2;
				unsigned int	u3;
				unsigned int	u4;
			};

			unsigned char		ucBuf[ 16 ];
		};
	};

	// 64 bit type
	typedef str::tc_int64		t_int64;

    // We've assumed this structure is 16 bytes
//	STATIC_ASSERT( sizeof( UID ) == 16 );

    template< typename T_STR >
        int ahtoui( typename T_STR::size_type *x_puNum, const T_STR *x_pBuffer, typename T_STR::size_type x_uBytes, bool x_bSkipInvalid = false )
    {
	    typename T_STR::size_type num = 0;

	    // For Each ASCII Digit
	    for ( typename T_STR::size_type i = 0; x_uBytes && x_pBuffer[ i ]; i++ )
	    {
			typedef typename T_STR::value_type T;

		    // Convert ASCII Digit Between 0 And 9
		    if ( x_pBuffer[ i ] >= tcTC( T, '0' ) && x_pBuffer[ i ] <= tcTC( T, '9' ) )
			    num = ( num << 4 ) + ( x_pBuffer[ i ] - tcTC( T, '0' ) );

		    // Convert ASCII Digit Between A And F
		    else if ( x_pBuffer[ i ] >= tcTC( T, 'A' ) && x_pBuffer[ i ] <= tcTC( T, 'F' ) )
			    num = ( num << 4 ) + ( x_pBuffer[ i ] - tcTC( T, 'A' ) ) + 10;

		    // Convert ASCII Digit Between a And f
		    else if ( x_pBuffer[ i ] >= tcTC( T, 'a' ) && x_pBuffer[ i ] <= tcTC( T, 'f' ) )
			    num = ( num << 4 ) + ( x_pBuffer[ i ] - tcTC( T, 'a' ) ) + 10;

		    // Do we just skip invalid digits?
		    else if ( !x_bSkipInvalid )
                return false;

			x_uBytes--;

	    } // end for

	    // Save number
	    if ( x_puNum )
            *x_puNum = num;

	    return true;
    }

    template< typename T_STR, typename T_UID >
        const typename T_STR::value_type* ToStr( typename T_STR::value_type *x_pStr,
												 typename T_STR::size_type x_uMax, const T_UID *x_pUid )
    {
		typedef typename T_STR::value_type T;

	    // Sanity checks
	    if ( !x_pStr || 0 >= x_uMax )
		    return 0;

		// NULL terminate
		*x_pStr = 0;

	    // Verify there is enough room for the string
	    if ( 36 > x_uMax )
	    	return x_pStr;

	    // Create new uid if one is not provided
	    T_UID uid;
	    if ( !x_pUid )
	    {	x_pUid = &uid;
            sys::randomize( &uid, sizeof( uid ) );
	    } // end if

	    // Example UID : DD05F574-2D69-4463-95DD-F76C9F7C5E6D
		str::htoa( x_pStr,			x_pUid->Data1, 8 );

		x_pStr[ 8 ] = tcTC( T, '-' );
		str::htoa( &x_pStr[ 9 ],	x_pUid->Data2, 4 );

		x_pStr[ 13 ] = tcTC( T, '-' );
		str::htoa( &x_pStr[ 14 ],	x_pUid->Data3, 4 );

		x_pStr[ 18 ] = tcTC( T, '-' );
		str::htoa( &x_pStr[ 19 ],	x_pUid->Data4[ 0 ], 2 );
		str::htoa( &x_pStr[ 21 ],	x_pUid->Data4[ 1 ], 2 );

		x_pStr[ 23 ] = tcTC( T, '-' );
		str::htoa( &x_pStr[ 24 ],	x_pUid->Data4[ 2 ], 2 );
		str::htoa( &x_pStr[ 26 ],	x_pUid->Data4[ 3 ], 2 );
		str::htoa( &x_pStr[ 28 ],	x_pUid->Data4[ 4 ], 2 );
		str::htoa( &x_pStr[ 30 ],	x_pUid->Data4[ 5 ], 2 );
		str::htoa( &x_pStr[ 32 ],	x_pUid->Data4[ 6 ], 2 );
		str::htoa( &x_pStr[ 34 ],	x_pUid->Data4[ 7 ], 2 );

		// Null terminate
		x_pStr[ 36 ] = 0;

		return x_pStr;
    }

	template< typename T_STR, typename T_UID >
		T_STR ToStr( const T_UID *x_pUid )
	{
		T_STR s; 
		try { s.resize( 36 ); }
		catch( ... ) { return T_STR(); }
		ToStr< T_STR, T_UID >( &s[ 0 ], s.length(), x_pUid );
		return s;
	}

	template< typename T_STR >
		T_STR ToStr()
	{
		T_STR s; 
		try { s.resize( 36 ); }
		catch( ... ) { return T_STR(); }
		ToStr< T_STR, UID >( &s[ 0 ], s.length(), 0 );
		return s;
	}

    template< typename T_STR, typename T_UID >
        T_UID * ToUid( T_UID *x_pUid, const typename T_STR::value_type *x_pString, typename T_STR::size_type x_nMax )
    {
		typedef typename T_STR::value_type T;
		typedef typename T_STR::size_type SZ;

	    if ( !x_pUid )
		    return 0;

        // Does the caller want a new UID?
        if ( !x_pString )
        {	sys::randomize( &x_pUid, sizeof( T_UID ) );
            return x_pUid;
        } // end if

		// Ensure buffer is large enough to hold uid
		if ( 36 > x_nMax )
			return 0;

		// Verify separators
		// Example : DD05F574-2D69-4463-95DD-F76C9F7C5E6D
		//                   ^    ^    ^    ^
		if ( x_pString[ 8 ] != tcTC( T, '-' ) || x_pString[ 13 ] != tcTC( T, '-' ) ||
			 x_pString[ 18 ] != tcTC( T, '-' ) || x_pString[ 23 ] != tcTC( T, '-' ) )
			return 0;

	    // Convert each component
	    unsigned long ul = 0;
		if ( !str::atoh( x_pString, &ul, 8 ) )
            return 0;
	    x_pUid->Data1 = ul;

		if ( !str::atoh( &x_pString[ 9 ], &ul, 4 ) )
            return 0;
	    x_pUid->Data2 = (unsigned short)ul;

		if ( !str::atoh( &x_pString[ 14 ], &ul, 4 ) )
            return 0;
	    x_pUid->Data3 = (unsigned short)ul;

	    SZ i;
	    for ( i = 0; i < 2; i++ )
	    {	if ( !str::atoh( &x_pString[ 19 + ( i << 1 ) ], &ul, 2 ) )
                return 0;
		    x_pUid->Data4[ i ] = (unsigned char)ul;
	    } // end for

	    for ( i = 0; i < 6; i++ )
	    {	if ( !str::atoh( &x_pString[ 24 + ( i << 1 ) ], &ul, 2 ) )
                return 0;
		    x_pUid->Data4[ 2 + i ] = (unsigned char)ul;
	    } // end for

	    return x_pUid;
    }

    template< typename T_UID >
        T_UID * Zero( T_UID *pDst )
    {
		*(t_int64*)pDst = 0;
        ( (t_int64*)pDst )[ 1 ] = 0;
        return pDst;
    }

    template< typename T_UID1, typename T_UID2 >
        T_UID1 * Copy( T_UID1 *pDst, const T_UID2 *pSrc )
    {
		*(t_int64*)pDst = *(t_int64*)pSrc;
		( (t_int64*)pDst )[ 1 ] = ( (t_int64*)pSrc )[ 1 ];
        return pDst;
    }

    template< typename T_UID1, typename T_UID2 >
        int Cmp( const T_UID1 *pUid1, const T_UID2 *pUid2 )
    {
		return ( *(t_int64*)pUid1 == *(t_int64*)pUid2
				 && ( (t_int64*)pUid1 )[ 1 ] == ( (t_int64*)pUid2 )[ 1 ] );
    }

    template< typename T_UID1, typename T_UID2, typename T_MASK >
        int Cmp( const T_UID1 *pUid1, const T_UID2 *pUid2, const T_MASK *pMask )
    {
        int bMatch = 1;
        unsigned char *p1 = (unsigned char*)pUid1;
        unsigned char *p2 = (unsigned char*)pUid2;
        unsigned char *m = (unsigned char*)pMask;

		// Ensure UID sizes match
		if ( sizeof( T_UID1 ) != sizeof( T_UID2 ) || sizeof( T_UID1 ) != sizeof( T_MASK ) )
			return 0;

		// Compare
        for ( unsigned long i = 0; bMatch && i < sizeof( T_UID1 ) / sizeof( unsigned char ); i++ )
            if ( p1[ i ] ^ p2[ i ] & m[ i ] )
                bMatch = 0;

        return bMatch;
    }

    template< typename T_UID1, typename T_UID2 >
        T_UID1 * Xor( T_UID1 *pUid1, const T_UID2 *pUid2 )
    {
		*(t_int64*)pUid1 ^= *(t_int64*)pUid2;
        ( (t_int64*)pUid1 )[ 1 ] ^= ( (t_int64*)pUid2 )[ 1 ];
        return pUid1;
    }

    template< typename T_UID1, typename T_UID2 >
        T_UID1 * And( T_UID1 *pUid1, const T_UID2 *pUid2 )
    {
        *(t_int64*)pUid1 &= *(t_int64*)pUid2;
        ( (t_int64*)pUid1 )[ 1 ] &= ( (t_int64*)pUid2 )[ 1 ];

        return pUid1;
    }

    template< typename T_UID1, typename T_UID2 >
        T_UID1 * Or( T_UID1 *pUid1, const T_UID2 *pUid2 )
    {
        *(t_int64*)pUid1 |= *(t_int64*)pUid2;
        ( (t_int64*)pUid1 )[ 1 ] |= ( (t_int64*)pUid2 )[ 1 ];

        return pUid1;
    }

    template< typename T_UID, typename T >
        T_UID * Set( T_UID *pUid, const T pVal, int uOffset, int uCount )
    {
        // Set UID values
        T *p = &( (T*)pUid )[ uOffset ];
        for ( int i = 0; ( sizeof( T ) * ( uOffset + i + 1 ) ) <= sizeof( T_UID ); i++ )
            p[ i ] = pVal;

        return pUid;
    }

}; // uid

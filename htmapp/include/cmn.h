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

namespace cmn
{

	/// Class factory interface
	class CFactory
	{
	public:

		/// Creates a class
		typedef void* (*PFN_Create)();

		/// Free a class
		typedef void (*PFN_Free)( void *p );

	public:

		/// Returns non-zero if there is valid memory
		virtual int IsValid() { return 0; }

		/// Creates an object instance
		virtual void* Create() = 0;

		/// Frees an object instance
		virtual void Free( void *p ) = 0;

	};

	/// Class factory template
	template< typename T > class TFactory : public CFactory
	{
	public:

		/// Returns non-zero if there is valid memory
		virtual int IsValid() { return 1; }

		/// Creates an object instance
		virtual void* Create()
		{	return (void*)new T(); }

		/// Frees an object instance
		virtual void Free( void *p )
		{	if ( p ) delete (T*)p; }

	};

	/// Returns the number of units required for val
	/**
		Example: How many 8-bit bytes needed to hold 42 bits

		required_bytes = FitTo( 42, 8 );
	*/
    template < typename T >
		T FitTo( T val, T unit )
	{	if ( !unit ) return 0;
		long i = (long)( val / unit );
		return ( ( i * unit ) >= val ) ? i : i + 1;
	}

    /// Returns the largest of a or b
    template < typename T >
        T Max( T a, T b )
    {   return ( a > b ) ? a : b; }

    /// Returns the largest of a, b, or c
    template < typename T >
        T Max( T a, T b, T c )
    {   return a > b ? ( a > c ? a : c ) : ( b > c ? b : c ); }

    /// Returns the largest of a, b, c, or d
    template< typename T >
    	T Max( T a, T b, T c, T d )
    {	return Max( a, Max( b, Max( c, d ) ) ); }

    /// Returns the smallest of a or b
    template < typename T >
        T Min( T a, T b )
    {   return ( a < b ) ? a : b; }

    /// Returns the smallest of a, b, or c
    template < typename T >
        T Min( T a, T b, T c )
    {   return a < b ? ( a < c ? a : c ) : ( b < c ? b : c ); }

    /// Returns the smallest of a, b, c, or d
    template< typename T >
    	T Min( T a, T b, T c, T d )
    {	return Min( a, Min( b, Min( c, d ) ) ); }

    /// Returns val limited to the range of min and max
    template < typename T >
        T Range( T val, T min, T max )
    {   return ( val < min ) ? min : ( val > max ) ? max : val; }

    /// Compares two values ensuring the return type is bool
    template < typename T >
        bool Eq( T a, T b )
    {   return ( a == b ) ? true : false; }

    /// Compares two values ensuring the return type is bool
    template < typename T >
        bool Neq( T a, T b )
    {   return ( a != b ) ? true : false; }

    /// Compares two values ensuring the return type is bool
    template < typename T >
        bool Lt( T a, T b )
    {   return ( a < b ) ? true : false; }

    /// Compares two values ensuring the return type is bool
    template < typename T >
        bool Gt( T a, T b )
    {   return ( a > b ) ? true : false; }

    /// Invokes the NOT (!) operator ensuring the return type is bool
    template < typename T >
        bool Not( T a )
    {   return ( !a ) ? true : false; }

    /// Returns the difference between two values
    template < typename T >
        T Dif( T a, T b )
    {   return a > b ? a - b : b - a; }

    /// Returns the absolute value
    template < typename T >
        T Abs( T v )
    {   return ( v >= 0 ) ? v : -v; }

	/// Returns the modulus
	template < typename T >
		static T Mod( T x, T y )
		{	return ( ( x ) - ( (long)( ( x ) / ( y ) ) ) * ( y ) ); }
	template <>
		inline char Mod< char >( char x, char y )
		{	return x % y; }
	template <>
		inline unsigned char Mod< unsigned char >( unsigned char x, unsigned char y )
		{	return x % y; }
	template <>
		inline short Mod< short >( short x, short y )
		{	return x % y; }
	template <>
		inline unsigned short Mod< unsigned short >( unsigned short x, unsigned short y )
		{	return x % y; }
	template <>
		inline int Mod< int >( int x, int y )
		{	return x % y; }
	template <>
		inline unsigned int Mod< unsigned int >( unsigned int x, unsigned int y )
		{	return x % y; }
	template <>
		inline long Mod< long >( long x, long y )
		{	return x % y; }
	template <>
		inline unsigned long Mod< unsigned long >( unsigned long x, unsigned long y )
		{	return x % y; }

    /// Scales a value v from range( 0, cur ) to range( 0, tgt )
    template < typename T >
        T Scale( T v, T cur, T tgt )
    {   if ( !cur ) return 0; return v * tgt / cur; }

    /// Scales a value v from range( min1, max1 ) to range( min2, max2 )
    template < typename T >
        T Scale( T v, T min1, T max1, T min2, T max2 )
    {   return min2 + Scale( v - min1, max1 - min1, max2 - min2 ); }

    /// Smooths value a to b using  ( a * weight + b ) / ( weight + 1 )
    template < typename T >
        T Smooth( T a, T b, T weight )
    {   return ( a * weight + b ) / ( weight + 1 ); }

    /// Smoothes value a to b using ( ( a << weight ) - a + b ) >> weight
    template < typename T >
        T FastSmooth( T a, T b, T weight )
    {   return ( ( a << weight ) - a + b ) >> weight; }

    /// Swaps value a and b, this will usually be the most efficient Swap function
    template < typename T >
        void Swap( T &a, T &b )
    {   T t = a; a = b; b = t; }

    /// Swaps values a, b, and c.  a = b, b = c, c = a
    template < typename T >
        void Swap( T &a, T &b, T &c )
    {   T t = a; a = b; b = c; c = t; }

    /// Swaps values a, b, c, and d.  a = b, b = c, c = d, d = a
    template < typename T >
        void Swap( T &a, T &b, T &c, T &d )
    {   T t = a; a = b; b = c; c = d; d = t; }

    /// Swaps values a, b, c, d, and e.  a = b, b = c, c = d, d = e, e = a
    template < typename T >
        void Swap( T &a, T &b, T &c, T &d, T &e )
    {   T t = a; a = b; b = c; c = d; d = e; e = t; }

	/// Reverses the byte order
	template < typename T >
		T* RevBytes( T* p, unsigned long uLen )
	{
		unsigned long i = 0;
		if ( !uLen )
			return p;

		uLen--;
		while ( i < uLen )
			Swap( ( (char*)p )[ i++ ], ( (char*)p )[ uLen-- ] );

		return p;
	}

	template < typename T >
		T& RevObj( T& r )
	{
		char*			p = (char*)&r;
		unsigned long	s = sizeof( T ) - 1;
		unsigned long	i = 0;

		while ( i < s )
			Swap( ( p )[ i++ ], ( p )[ s-- ] );

		return r;
	}

    /// Values for pi
    const double c_PI = 3.141592654;
    const double c_PI2 = c_PI * (double)2;

    /// Returns the opposite angle
    template< typename T >
        T OppositeAngle( T a, T m = (T)c_PI2 )
    {   T h = m / 2; return ( a >= h ) ? a - h : a + h; }

    /// Calculates the minimum difference betwen the two angles
    template < typename T >
	    T MinDifAngle( T a, T b, T m = (T)c_PI2 )
	{   T dif = Dif( a, b );
		return ( dif >= ( m / 2 ) ) ? ( m - dif ) : dif;
	}

    /// Averages two angles
    template< typename T >
        T MinAvgAngle( T a, T b, T m = (T)c_PI2 )
    {   if ( a > b ) Swap( a, b );
        T h = m / 2, dif = b - a, avg;
        if ( dif > h )
        {   dif = ( m - dif ) / 2;
            avg = ( dif > a ) ? b + dif : a - dif;
        } // end if
        else avg = a + dif / 2;
        if ( dif == h && avg >= h ) avg -= h;
        return avg;
    }

    /// Aligns value on 16 bit boundry
    template < typename T >
        T Align2( T val )
    {   return ( val + 1 ) & ( ~1 ); }

    /// Returns non-zero if aligned on 16 bit boundry
    template < typename T >
        bool IsAligned2( T val )
    {   return Align2( val ) == val; }

    /// Aligns value on 32 bit boundry
    template < typename T >
        T Align4( T val )
    {   return ( val + 3 ) & ( ~3 ); }

    /// Returns non-zero if aligned on 32 bit boundry
    template < typename T >
        bool IsAligned4( T val )
    {   return Align4( val ) == val; }

    /// Aligns value on 64 bit boundry
    template < typename T >
        T Align8( T val )
    {   return ( val + 7 ) & ( ~7 ); }

    /// Returns non-zero if aligned on 64 bit boundry
    template < typename T >
        bool IsAligned8( T val )
    {   return Align8( val ) == val; }

    /// Aligns value on 128 bit boundry
    template < typename T >
        T Align16( T val )
    {   return ( val + 15 ) & ( ~0x0f ); }

    /// Returns non-zero if aligned on 128 bit boundry
    template < typename T >
        bool IsAligned16( T val )
    {   return Align16( val ) == val; }

    /// Aligns value on 256 bit boundry
    template < typename T >
        T Align32( T val )
    {   return ( val + 31 ) & ( ~0x1f ); }

    /// Returns non-zero if aligned on 256 bit boundry
    template < typename T >
        bool IsAligned32( T val )
    {   return Align32( val ) == val; }

    /// Aligns value on 512 bit boundry
    template < typename T >
        T Align64( T val )
    {   return ( val + 63 ) & ( ~0x3f ); }

    /// Returns non-zero if aligned on 512 bit boundry
    template < typename T >
        bool IsAligned64( T val )
    {   return Align64( val ) == val; }

	/// Aligns value on n byte boundry, where n is a power of 2
	template< typename T >
		T AlignN( T val, T n )
	{	return ( val + ( n - 1 ) ) & ( ~( n - 1 ) ); }

    /// Returns non-zero if aligned on 512 bit boundry
    template < typename T >
        bool IsAlignedN( T val, T n )
    {   return AlignN( val, n ) == val; }

    /// Returns the power of two equal to or greater than v, or v if there isn't one
    template< typename T >
	    T NextPower2( T v )
	{	T n = 2;
		while ( n && n < v ) n <<= 1;
		if ( !n ) return v;
		return n;
	}

    /// Returns the power of two equal to or less than v, or v if there isn't one
    template< typename T >
	    T PrevPower2( T v )
	{	T n = 2;
		while ( n && n <= v ) n <<= 1;
		if ( !n ) return v;
		return n >> 1;
	}

	/// Returns non-zero if the specified value is a power of 2
	template< typename T >
		bool IsPowerOf2( T v )
		{	return v == ( v & ( v - 1 ) ); }

	template< typename T >
		T BitsToValue( T bits )
		{
			if ( !bits )
				return 0;

			return 1 << bits;
		}

	template< typename T >
		T ValueToBits( T v )
		{
			if ( v < 2 )
				return 1;

			v = NextPower2( v );
			T bits = 1;
			while ( v )
				v >>= 1,
				bits++;

			return bits;
		}

	template< typename T >
		T BitsToMask( T bits )
		{
			T mask = 0;

			while ( bits-- )
				mask <<= 1, mask |= 1;

			return mask;
		}

	template< typename T >
		T CountBits( T v )
		{
			T bits = 0;
			while ( v )
			{	if ( v & 1 )
					bits++;
				v >>= 1;
			} // end while

			return bits;
		}

    /// Returns the state of the specified bit in buf
    template < typename T >
        bool GetBit( T *buf, unsigned long bit )
    {   return 0 != ( ( (unsigned char*)buf)[ bit >> 3 ] & ( 1 << ( bit & 7 ) ) ) ? true : false; }

    /// Sets the state of the specified bit in buf
    template < typename T >
        void SetBit( T *buf, unsigned long bit, bool set )
    {   if ( set ) ( (unsigned char*)buf)[ bit >> 3 ] |= ( 1 << ( bit & 7 ) );
        else ( (unsigned char*)buf)[ bit >> 3 ] &= ~( 1 << ( bit & 7 ) );
    }

    /// Copies bits number of bits from src[ soff ] to dst[ doff ]
    /// where doff and soff are offsets in bits.
    /**
        +++ This could probably be made a little more efficient by using
            masks and copying multiple bits at once, but that would make my
            brain hurt at the moment.
    */
    template < typename TD, typename TS >
        void CopyBits( TD *dst, unsigned long doff, TS *src, unsigned long soff, unsigned long bits )
    {
		if ( 0 < bits )
			while ( bits-- )
            	SetBit( dst, doff++, GetBit( src, soff++ ) );
    }

	template < typename T >
		long RW( T *pRect )
	{	return pRect->right - pRect->left; }

	template < typename T >
		long RW( T pRect )
	{	return pRect.right - pRect.left; }

	template < typename T >
		long RH( T *pRect )
	{	return pRect->bottom - pRect->top; }

	template < typename T >
		long RH( T pRect )
	{	return pRect.bottom - pRect.top; }

	template< typename T >
		long bsearch( T *a, long n, T s, long inv = -1 )
		{
			if ( !a || !n )
				return inv;

			// Save begining and ending pointers
			T *o = a, *b = a, *e = a + n - 1;

			while ( b <= e )
			{
				if ( *a < s )
					b = ++a, a += ( e - b ) >> 1;

				else if ( *a > s )
					e = --a, a -= ( e - b ) >> 1;

				else
					return a - o;
			};

			return -1;
		}

#if defined( HTM_BIG_ENDIAN )
	template< typename T > T LE( T v ) { RevBytes( &v, sizeof( v ) ); return v; }
	template< typename T > T setLE( void* p, T v ) { RevBytes( &v, sizeof( v ) ); if ( p ) *(T*)p = v; return v; }
	template< typename T > T BE( T v ) { return v; }
	template< typename T > T setBE( void* p, T v ) { if ( p ) *(T*)p = v; return v; }
#else
	template< typename T > T LE( T v ) { return v; }
	template< typename T > T setLE( void* p, T v ) { if ( p ) *(T*)p = v; return v; }
	template< typename T > T BE( T v ) { RevBytes( &v, sizeof( v ) ); return v; }
	template< typename T > T setBE( void* p, T v ) { RevBytes( &v, sizeof( v ) ); if ( p ) *(T*)p = v; return v; }
#endif

}; // namespace cmn

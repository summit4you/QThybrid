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

#include <map>
#include <string>
#include "str.h"

//==================================================================
/// Implements a multi-dimensional property bag
/**

    This class provides functionality for a multi-dimensional
    property bag and type conversions.

    Typical use

	@code

		TPropertyBag< basic_string< char > > pb;

		pb[ "A" ][ "AA" ] = "Hello World!";
		pb[ "A" ][ "AB" ] = (long)1;
		pb[ "B" ][ "BA" ] = (double)3.14159;
		
		for ( long i = 0; i < 4; i++ )
			pb[ "list" ][ i ] = i * 2;

		// Get long value
		long lVal = pb[ "A" ][ "AB" ].ToLong();

		// Get double
		double dVal = pb[ "B" ][ "BA" ].ToDouble();

		// Get string value
		const char *pString = pb[ "list" ][ 0 ].c_str();

	@endcode

*/
//==================================================================
template < typename T_STR > class TPropertyBag
{
public:

	enum eFlags
	{
		efList = 0x00000001
	};

public:

    //==================================================================
    // CAutoMem
    //
    /// Just a simple auto pointer
    /**
        This class is a simple auto pointer. It has properties that I
        particularly like for this type of job. I'll quit making my
        own when boost comes with VC... 
    */
    //==================================================================
    template < typename T_OBJ > class CAutoMem
    {
        public:

            /// Default constructor
            CAutoMem() { m_p = 0; }

            /// Destructor
            ~CAutoMem() { release(); }

            /// Release allocated object
            void release() { if ( m_p ) { delete m_p; m_p = 0; } }

            /// Returns a pointer to encapsulated object
            T_OBJ& Obj() { if ( !m_p ) m_p = new T_OBJ; return *m_p; }
            const T_OBJ& Obj() const { return ((CAutoMem*)(this))->Obj(); }

            /// Returns a pointer to encapsulated object
            T_OBJ& operator *() { return Obj(); }
            const T_OBJ& operator *() const { return ((CAutoMem*)(this))->Obj(); }

			/// Assignment operator
			T_OBJ& operator = ( const T_OBJ& r ) { return Obj() = r; }

            /// Returns a pointer to the encapsulated object
            operator T_OBJ&() { return Obj(); }

            /// Returns a pointer to the encapsulated object
            T_OBJ* operator ->() { Obj(); return m_p; }
            const T_OBJ* operator ->() const { Obj(); return m_p; }
			
        private:

            /// Contains a pointer to the controlled object
            T_OBJ       *m_p;
    };

    /// Unicode friendly string
    typedef T_STR t_String;
	
	/// Character type
	typedef typename T_STR::value_type T;

    /// Our multi-dimensional string array type
    typedef typename std::map< t_String, CAutoMem< TPropertyBag< T_STR > > > t_map;

public:

	/// Pass on iterator type
	typedef typename t_map::iterator iterator;
	typedef typename t_map::const_iterator const_iterator;

	/// First item
	iterator begin() { return m_lstSub.begin(); }
	const_iterator begin() const { return m_lstSub.begin(); }

	/// Item beyond the last item, yeah, that's what it is
	iterator end() { return m_lstSub.end(); }
	const_iterator end() const { return m_lstSub.end(); }

	/// Expose the list erase function
	void erase( iterator it ) { m_lstSub.erase( it ); }
	typename t_map::size_type erase( const typename t_map::key_type &k ) 
	{	return m_lstSub.erase( k ); }
	

public:

    /// Default constructor
    TPropertyBag() { m_flags = 0; }

    //==============================================================
    // TPropertyBag()
    //==============================================================
    /// Constructos object from string pointer
    /**
        \param [in] pStr    -   string pointer
    */
    TPropertyBag( const T *pStr, typename t_String::size_type len = 0 )
    {
		m_flags = 0;

		// Assign string if needed
		if ( pStr )
		{	if ( 0 >= len )
				len = zstr::Length( pStr );
			m_str.assign( pStr, len );
		} // end if
    }

    //==============================================================
    // TPropertyBag()
    //==============================================================
    /// Constructos object from string reference
    /**
        \param [in] sStr    -   string reference
    */
    TPropertyBag( const t_String &sStr )
    {   m_flags = 0;
    	m_str = sStr;
    }

    //==============================================================
    // TPropertyBag()
    //==============================================================
    /// Constructs object from another property bag
    /**
        \param [in] r    - Reference to property bag to be copied
    */
    TPropertyBag( const TPropertyBag &r )
    {  	merge( r ); 
    }

    /// Construction from int
    TPropertyBag( int n )
    {   m_flags = 0; m_str = str::ToString< t_String >( n ); }

    /// Construction from unsigned int
    TPropertyBag( unsigned int n )
    {   m_flags = 0; m_str = str::ToString< t_String >( n ); }

    /// Construction from long
    TPropertyBag( long n )
    {   m_flags = 0; m_str = str::ToString< t_String >( n ); }

    /// Construction from unsigned long
    TPropertyBag( unsigned long n )
    {   m_flags = 0; m_str = str::ToString< t_String >( n ); }

    /// Construction from long
    TPropertyBag( str::tc_int64 n )
    {   m_flags = 0; m_str = str::ToString< t_String >( n ); }

    /// Construction from unsigned long
    TPropertyBag( str::tc_uint64 n )
    {   m_flags = 0; m_str = str::ToString< t_String >( n ); }

    /// Construction from double
    TPropertyBag( float n )
    {   m_flags = 0; m_str = str::ToString< t_String >( n ); }

    /// Construction from double
    TPropertyBag( double n )
    {   m_flags = 0; m_str = str::ToString< t_String >( n ); }

    //==============================================================
    // operator = ()
    //==============================================================
    /// Copies the contents of another property bag
    /**
        \param [in] r    - Reference to property bag to be copied
    */
    TPropertyBag& operator = ( const TPropertyBag &r ) { clear(); merge( r ); return *this; }

    //==============================================================
    // merge
    //==============================================================
    /// Merges the properties of the specified property bag
    /**
        \param [in] r    - Reference to property bag to be merged
    */
	void merge( const TPropertyBag &r )
	{
		// Copy string value
		m_str = r.m_str;

		// Copy flags
		m_flags = r.m_flags;

		// Copy array
		stdForeach( const_iterator, it, r )
			m_lstSub[ it->first ]->merge( *it->second );
	}

    /// Releases all memory resources and prepares class for reuse.
    void clear() { m_lstSub.clear(); m_str.clear(); m_flags = 0; }

	/// Returns the number of items in the property bag
	long size() const { return m_lstSub.size(); }

    //==============================================================
    // operator []()
    //==============================================================
    /// Indexes into sub array
    /**
        \param [in] pKey    -   Index key
        
        \return Reference to sub class.
    
        \see 
    */
    TPropertyBag& operator []( const T* pKey ) { return m_lstSub[ pKey ]; }

    //==============================================================
    // operator []()
    //==============================================================
    /// Indexes into sub array
    /**
        \param [in] sKey    -   Index key
        
        \return Reference to sub class.
    
        \see 
    */
    TPropertyBag& operator []( const t_String &sKey ) { return m_lstSub[ sKey.c_str() ]; }

    //==============================================================
    // get()
    //==============================================================
    /// Indexes into sub array
    /**
        \param [in] sKey    -   Index key
        
        \return Reference to sub class.
    
        \see 
    */
    TPropertyBag& get( const t_String &sKey ) { return m_lstSub[ sKey.c_str() ]; }
	
    //==============================================================
    // operator []()
    //==============================================================
    /// Indexes into sub array
    /**
        \param [in] n   -   Index key
        
        \return Reference to sub class.
    */
    TPropertyBag& operator []( int n ) 
    {	return m_lstSub[ str::ToString< t_String >( n ) ]; }

    //==============================================================
    // operator []()
    //==============================================================
    /// Indexes into sub array
    /**
        \param [in] n   -   Index key
        
        \return Reference to sub class.
    */
    TPropertyBag& operator []( unsigned int n ) 
    {	return m_lstSub[ str::ToString< t_String >( n ) ]; }

    //==============================================================
    // operator []()
    //==============================================================
    /// Indexes into sub array
    /**
        \param [in] n   -   Index key
        
        \return Reference to sub class.
    */
    TPropertyBag& operator []( long n ) 
    {	return m_lstSub[ str::ToString< t_String >( n ) ]; }

    //==============================================================
    // operator []()
    //==============================================================
    /// Indexes into sub array
    /**
        \param [in] n   -   Index key
    
        \return Reference to sub class.
    */
    TPropertyBag& operator []( unsigned long n ) 
    {	return m_lstSub[ str::ToString< t_String >( n ) ]; }

    //==============================================================
    // operator []()
    //==============================================================
    /// Indexes into sub array
    /**
        \param [in] n   -   Index key
        
        \return Reference to sub class.
    */
    TPropertyBag& operator []( str::tc_int64 n ) 
    {	return m_lstSub[ str::ToString< t_String >( n ) ]; }

    //==============================================================
    // operator []()
    //==============================================================
    /// Indexes into sub array
    /**
        \param [in] n   -   Index key
    
        \return Reference to sub class.
    */
    TPropertyBag& operator []( str::tc_uint64 n ) 
    {	return m_lstSub[ str::ToString< t_String >( n ) ]; }

    //==============================================================
    // operator []()
    //==============================================================
    /// Indexes into sub array
    /**
        \param [in] n   -   Index key
        
        \return Reference to sub class.
    */
    TPropertyBag& operator []( double n ) 
    {	return m_lstSub[ str::ToString< t_String >( n ) ]; }

    //==============================================================
    // operator = ()
    //==============================================================
    /// Conversion from string object
    TPropertyBag& operator = ( t_String sStr ) 
    {   m_str = sStr; return *this; }

    //==============================================================
    // operator = ()
    //==============================================================
    /// Conversion from string
    TPropertyBag& operator = ( const T* pStr ) 
    {   m_str = pStr; return *this; }

    //==============================================================
    // operator = ()
    //==============================================================
    /// Conversion from int
    TPropertyBag& operator = ( int n )
    {   m_str = str::ToString< t_String >( n ); return *this; }

    //==============================================================
    // operator = ()
    //==============================================================
    /// Conversion from unsigned int
    TPropertyBag& operator = ( unsigned int n )
    {   m_str = str::ToString< t_String >( n ); return *this; }

    //==============================================================
    // operator = ()
    //==============================================================
    /// Conversion from long
    TPropertyBag& operator = ( long n )
    {   m_str = str::ToString< t_String >( n ); return *this; }

    //==============================================================
    // operator = ()
    //==============================================================
    /// Conversion from unsigned long
    TPropertyBag& operator = ( unsigned long n )
    {   m_str = str::ToString< t_String >( n ); return *this; }

    //==============================================================
    // operator = ()
    //==============================================================
    /// Conversion from long
    TPropertyBag& operator = ( str::tc_int64 n )
    {   m_str = str::ToString< t_String >( n ); return *this; }

    //==============================================================
    // operator = ()
    //==============================================================
    /// Conversion from unsigned long
    TPropertyBag& operator = ( str::tc_uint64 n )
    {   m_str = str::ToString< t_String >( n ); return *this; }

    //==============================================================
    // operator = ()
    //==============================================================
    /// Conversion from double
    TPropertyBag& operator = ( float n )
    {   m_str = str::ToString< t_String >( n ); return *this; }

    //==============================================================
    // operator = ()
    //==============================================================
    /// Conversion from double
    TPropertyBag& operator = ( double n )
    {   m_str = str::ToString< t_String >( n ); return *this; }

    //==============================================================
    // operator << ()
    //==============================================================
    /// Conversion from string object
    TPropertyBag& operator << ( t_String sStr ) 
    {   m_str += sStr; return *this; }

    //==============================================================
    // operator << ()
    //==============================================================
    /// Conversion from string
    TPropertyBag& operator << ( const T* pStr ) 
    {   m_str += pStr; return *this; }

    //==============================================================
    // operator << ()
    //==============================================================
    /// Conversion from int
    TPropertyBag& operator << ( int n )
    {   m_str += str::ToString< t_String >( n ); return *this; }

    //==============================================================
    // operator << ()
    //==============================================================
    /// Conversion from unsigned int
    TPropertyBag& operator << ( unsigned int n )
    {   m_str += str::ToString< t_String >( n ); return *this; }

    //==============================================================
    // operator << ()
    //==============================================================
    /// Conversion from long
    TPropertyBag& operator << ( long n )
    {   m_str += str::ToString< t_String >( n ); return *this; }

    //==============================================================
    // operator << ()
    //==============================================================
    /// Conversion from unsigned long
    TPropertyBag& operator << ( unsigned long n )
    {   m_str += str::ToString< t_String >( n ); return *this; }

    //==============================================================
    // operator << ()
    //==============================================================
    /// Conversion from long
    TPropertyBag& operator << ( str::tc_int64 n )
    {   m_str += str::ToString< t_String >( n ); return *this; }

    //==============================================================
    // operator << ()
    //==============================================================
    /// Conversion from unsigned long
    TPropertyBag& operator << ( str::tc_uint64 n )
    {   m_str += str::ToString< t_String >( n ); return *this; }

    //==============================================================
    // operator << ()
    //==============================================================
    /// Conversion from double
    TPropertyBag& operator << ( float n )
    {   m_str += str::ToString< t_String >( n ); return *this; }

    //==============================================================
    // operator << ()
    //==============================================================
    /// Conversion from double
    TPropertyBag& operator << ( double n )
    {   m_str += str::ToString< t_String >( n ); return *this; }

    //==============================================================
    // operator + ()
    //==============================================================
    /// Conversion from string object
    TPropertyBag& operator + ( t_String sStr ) 
    {   m_str += sStr; return *this; }

    //==============================================================
    // operator + ()
    //==============================================================
    /// Conversion from string
    TPropertyBag& operator + ( const T* pStr ) 
    {   m_str += pStr; return *this; }

    //==============================================================
    // operator + ()
    //==============================================================
    /// Conversion from int
    TPropertyBag& operator + ( int n )
    {   m_str += str::ToString< t_String >( n ); return *this; }

    //==============================================================
    // operator + ()
    //==============================================================
    /// Conversion from unsigned int
    TPropertyBag& operator + ( unsigned int n )
    {   m_str += str::ToString< t_String >( n ); return *this; }

    //==============================================================
    // operator + ()
    //==============================================================
    /// Conversion from long
    TPropertyBag& operator + ( long n )
    {   m_str += str::ToString< t_String >( n ); return *this; }

    //==============================================================
    // operator + ()
    //==============================================================
    /// Conversion from unsigned long
    TPropertyBag& operator + ( unsigned long n )
    {   m_str += str::ToString< t_String >( n ); return *this; }

    //==============================================================
    // operator + ()
    //==============================================================
    /// Conversion from long
    TPropertyBag& operator + ( str::tc_int64 n )
    {   m_str += str::ToString< t_String >( n ); return *this; }

    //==============================================================
    // operator + ()
    //==============================================================
    /// Conversion from unsigned long
    TPropertyBag& operator + ( str::tc_uint64 n )
    {   m_str += str::ToString< t_String >( n ); return *this; }

    //==============================================================
    // operator + ()
    //==============================================================
    /// Conversion from double
    TPropertyBag& operator + ( float n )
    {   m_str += str::ToString< t_String >( n ); return *this; }

    //==============================================================
    // operator + ()
    //==============================================================
    /// Conversion from double
    TPropertyBag& operator + ( double n )
    {   m_str += str::ToString< t_String >( n ); return *this; }

    //==============================================================
    // setFlags()
    //==============================================================
    /// Sets the flags
    void setFlags( long f ) { m_flags = f; }    

    //==============================================================
    // getFlags()
    //==============================================================
    /// Returns the flags
    long getFlags() const { return m_flags; }    

    //==============================================================
    // is_list()
    //==============================================================
    /// Returns non-zero if this is the list flag is set
    bool is_list() const { return 0 != ( m_flags & efList ); }    

    //==============================================================
    // is_list()
    //==============================================================
    /// Sets the list flag
	void is_list( bool b ) 
	{
		if ( b ) 
			m_flags |= efList;
		else
			m_flags &= ~efList;
	}

    //==============================================================
    // T*()
    //==============================================================
    /// Conversion to const T*
    operator const T*() { return c_str(); }

    //==============================================================
    // c_str()
    //==============================================================
    /// Returns NULL terminated const pointer to string buffer
    const T* c_str() const { return m_str.c_str(); }

    //==============================================================
    // data()
    //==============================================================
    /// Returns pointer to string buffer (may not be NULL terminated)
    const T* data() const { return m_str.c_str(); }
	
    //==============================================================
    // length()
    //==============================================================
    /// Returns length of string
    long length() const { return m_str.length(); }

    //==============================================================
    // ToString()
    //==============================================================
    /// Returns reference to string object
    t_String& str() { return m_str; }
    const t_String& str() const { return m_str; }

    //==============================================================
    // ToString()
    //==============================================================
    /// Returns copy of the string object
    t_String ToString() const { return m_str; }

    //==============================================================
    // ToInt()
    //==============================================================
    /// Converts to int
    int ToInt() { return str::StrToInt( c_str() ); }

    //==============================================================
    // ToUInt()
    //==============================================================
    /// Converts to unsigned int
    unsigned int ToUInt() { return str::StrToUInt( c_str() ); }

    //==============================================================
    // ToInt64()
    //==============================================================
    /// Converts to int64
    str::tc_int64 ToInt64() { return str::StrToInt64( c_str() ); }

    //==============================================================
    // ToUInt64()
    //==============================================================
    /// Converts to unsigned int64
    str::tc_uint64 ToUInt64() { return str::StrToUInt64( c_str() ); }

    //==============================================================
    // ToLong()
    //==============================================================
    /// Converts to long
    long ToLong() { return str::StrToLong( c_str() ); }

    //==============================================================
    // ToULong()
    //==============================================================
    /// Converts to unsigned long
    unsigned long ToULong() { return str::StrToULong( c_str() ); }

    //==============================================================
    // ToFloat()
    //==============================================================
    /// Converts to float
    float ToFloat() { return str::StrToFloat( c_str() ); }

    //==============================================================
    // ToDouble()
    //==============================================================
    /// Converts to double
    double ToDouble() { return str::StrToDouble( c_str() ); }

    //==============================================================
    // IsArray()
    //==============================================================
    /// Returns non-zero if array elements are present
    bool IsArray() { return 0 < m_lstSub.size(); }

    //==============================================================
    // isSet()
    //==============================================================
    /// Returns non-zero if the specified key exists
    bool isSet( const t_String &k ) 
	{	return ( m_lstSub.end() == m_lstSub.find( k ) ) ? false : true; }

public:

	//==============================================================
	// at
	//==============================================================
	/// Parses string path
	/**
		\param [in] x_key	-	Index key

		pb.at( "k1.k2.k3", "." ).ToString()

		is equiv to

		pb[ "k1" ][ "k2" ][ "k3" ].ToString()

		\return Reference to sub class.

		\see
	*/
	TPropertyBag& at( const t_String &k, const t_String &sep )
	{
		// Null length or no sep?
		if ( !k.length() || !sep.length() )
			return *this;

		// If no sub key
		typename t_String::size_type p = k.find_first_of( sep );
		if ( t_String::npos == p )
			return m_lstSub[ k ];

		// Null key is still us
		if ( !p )
			return at( t_String( k, p + sep.length() ), sep );

		// Sub key
		return m_lstSub[ t_String( k, 0, p ) ]->at( t_String( k, p + sep.length() ), sep );
	}

private:

	/// Array flags
	long							m_flags;

    /// Our value
    t_String						m_str;

    /// Array of string maps
    t_map							m_lstSub; 

};

/// Property bag types
/** \see TPropertyBag */
typedef TPropertyBag< str::t_string8 > t_pb8;
#ifndef CII_NO_WCHAR
	typedef TPropertyBag< str::t_stringw > t_pbw;
#endif

#if defined( tcUNICODE )
	typedef TPropertyBag< str::t_stringw > t_pb;
#else
	typedef TPropertyBag< str::t_string8 > t_pb;
#endif



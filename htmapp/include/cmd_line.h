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

#include <list>
#include "str.h"
#include "property_bag.h"

/**
	Command line parser
	
	// Example command line input and parsing
	@code
	
		// Say the command line was
		
		myapp.exe ok -v -xyz 1234 -i "c:\\Program Files" --out 'c:/temp' --p1:hi --p2:"hi 1" --p3:hi\ 2 --p3 hi\ 3 --p4:##1
		
		// It will parse to
		
		[ #0 ] 	= 'myapp.exe'
		[ #1 ] 	= 'ok'
		[ #2 ] 	= '1234'
		[ #3 ] 	= 'c:\Program Files'
		[ #4 ] 	= 'c:/temp'
		[ #5 ] 	= 'hi 3'
		[ i ] 	= 'c:\Program Files'
		[ out ]	= 'c:/temp'
		[ p1 ] 	= 'hi'
		[ p2 ] 	= 'hi 1'
		[ p3 ] 	= 'hi 3'
		[ p4 ] 	= 'ok'
		[ v ] 	= '#2'
		[ x ] 	= '1234'
		[ y ] 	= '1234'
		[ z ] 	= '1234'
		
	@endcode
		
	// Example code
	@code
		
	    TCmdLine< char > cl( argc, argv );
		
		basic_string< char > in = cl.pb()[ "in" ].str();
		
		const char *pOut = cl.pb()[ "out" ].c_str();
		
		long x = cl.pb()[ "x" ].ToLong();
		
		cl.pb()[ "o" ] = cl.pb()[ "out" ];
		
	@endcode

	
	@endcode

*/
template< typename T_STR >
	class TCmdLine
{
public:

	/// Character type
	typedef typename T_STR::value_type t_char;

	/// Size type
	typedef long t_size;

	/// Property bag type
	typedef TPropertyBag< T_STR > t_pb;
	
	/// String type
	typedef typename t_pb::t_String t_String;

	/// List of strings
	typedef std::list< t_String > t_strlist;

	/// List of string pointers
	typedef std::list< t_String* > t_pstrlist;
	
public:

	/// Pass on iterator type
	typedef typename t_pb::iterator iterator;

	/// First item
	iterator begin() { return m_pb.begin(); }

	/// Item beyond the last item, yeah, that's what it is
	iterator end() { return m_pb.end(); }

public:

	// Default constructor
	TCmdLine()
	{
	}

	/// Constructs the object from the specified command line
	/**
		@param[in] argc		- Number of arguments
		@param[in] argv		- Pointer to array of argument pointers
	*/
	TCmdLine( int argc, t_char **argv )
	{
		Parse( argc, argv );
	}
	
	/// Destructor
	~TCmdLine()
	{
		clear();
	}

	/// Releases all resources
	void clear()
	{
		m_pb.clear();
	}

	/// Constructs the object from the specified command line
	/**
		@param[in] x_pStr	- Pointer to command line string
		@param[in] x_lSize	- Size of the string in str
	*/
	TCmdLine( const t_char *x_pStr, t_size x_lSize )
	{
		Parse( x_pStr, x_lSize );
	}

	/// Parses the command line arguments
	/**
		@param[in] argc		- Number of arguments
		@param[in] argv		- Pointer to array of argument pointers
		
		@return Returns the number of arguments found
	*/
	t_size Parse( int argc, t_char **argv )
	{
		// Hope the OS parsed this as we expect
		t_pstrlist last;
		for ( t_size n = 0, i = 0; n < argc; n++ )
			if ( argv[ n ] )
				i = ParseCommandLineItem( i, argv[ n ], zstr::Length( argv[ n ] ), m_pb, &last );

		MapSwitchValues();

		return m_pb.size();
	}

	/// Parses a single command line item
	/**
		@param[in] i		- String position
		@param[in] x_sStr	- String
		@param[in] x_pb		- Reference to property bag in which to add item
	*/
	static t_size ParseCommandLineItem( t_size i, const t_char *x_p, t_size x_l, t_pb &x_pb, t_pstrlist *last )
	{
		t_size n = 0;
		if ( !x_p || 0 >= x_l )
			return i;
	
		// Is it a switch?
		if ( tcTC( t_char, '-' ) != x_p[ n ] )
		{
			// Save the naked value into the array by it's position
			t_String sVal( x_p, x_l );
			x_pb[ t_String( tcTT( t_char, "#" ) ) + str::ToString< t_String >( i ) ] = sVal;
			
			// Update previous switch(s) with it's value
			if ( last && last->size() )
			{	stdForeach( typename t_pstrlist::iterator, it, (*last) )
					if ( *it )
						**it = sVal;
				last->clear();
			} // end if

			return ++i;

		} // end else

		// Dump the last switches
		if ( last )
			last->clear();
		
		// Skip the switch
		if ( ++n >= x_l )
			return i;

		// Single switch item?
		if ( tcTC( t_char, '-' ) != x_p[ n ] )
		{
			// Find string separator
			t_size sep = str::FindCharacter( &x_p[ n ], x_l - n, tcTC( t_char, ':' ) );
			t_String sVal = ( 0 < sep ) 
							? t_String( &x_p[ n + sep + 1 ], x_l - n - sep ) 
							: ( t_String( tcTT( t_char, "#" ) ) + str::ToString< t_String >( i ) );

			// Set all switches
			while( n < x_l && tcTC( t_char, ':' ) != x_p[ n ] )
			{
				t_char key[ 2 ] = { x_p[ n++ ], 0 };
				x_pb[ key ] = sVal;

				// In case we want to update them later with a real value
				if ( 0 >= sep && last )
					last->push_back( &x_pb[ key ].str() );

			} // end while

			return i;

		} // end if

		// Skip the double switch
		if ( ++n >= x_l )
			return i;

		// Find string separator
		t_String sKey;
		t_size sep = str::FindCharacter( &x_p[ n ], x_l - n, tcTC( t_char, ':' ) );
		if ( 0 < sep )
			x_pb[ sKey = t_String( &x_p[ n ], sep ) ] 
				= t_String( &x_p[ n + sep + 1 ], x_l - n - sep );
		else
		{
			x_pb[ sKey = t_String( &x_p[ n ], x_l - n ) ] 
				= t_String( tcTT( t_char, "#" ) ) + str::ToString< t_String >( i );
			if ( last )
				last->push_back( &x_pb[ sKey ].str() );
		} // end else

		return i;
	}

	/// Parses the command line arguments
	/**
		@param[in] argc		- Number of arguments
		@param[in] argv		- Pointer to array of argument pointers
		
		@return Returns the number of arguments found
	*/
	t_size Parse( const t_char *x_pStr, t_size x_lSize )
	{
		// Start over
		clear();

		// Sanity check
		if ( !x_pStr || 0 >= x_lSize )
			return 0;

		// Break the command line into chunks
		t_strlist sl = str::SplitQuoted< t_char, t_String, t_strlist >
								( x_pStr, x_lSize, 
								  tcTT( t_char, " \t" ), tcTT( t_char, "\"'" ), 
								  tcTT( t_char, "\"'" ), tcTT( t_char, "\\" ) );

		// We get anything?
		if ( !sl.size() )
			return 0;

		// Parse each item
		t_size i = 0; t_pstrlist last;
		stdForeach( typename t_strlist::iterator, it, sl )
			i = ParseCommandLineItem( i, it->data(), it->length(), m_pb, &last );

		// Set switch data values
		MapSwitchValues();

		return m_pb.size();
	}

	/// Will map any values with ##<offset> to the corrisponding position keys value
	void MapSwitchValues()
	{
		// @warning Don't use substr(), it has bugs ;)

		// Set switch values to corrisponding data
		stdForeach( typename t_pb::iterator, it, m_pb )
			if ( t_String( it->second->str().c_str(), 0, 2 ) == tcTT( t_char, "##" ) )
			{	t_String sKey = t_String( it->second->str().c_str(), 1, t_String::npos );
				if ( m_pb.isSet( sKey ) )
					it->second = m_pb[ sKey ];
			} // end if
	}

	/// Return a reference to the property bag holding the command line params
	t_pb& pb() { return m_pb; }

private:

	/// Property bag that holds the parsed command line
	t_pb				m_pb;

};


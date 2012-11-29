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

template< typename T_STR >
	int process_binary( const T_STR sIn, const T_STR sOut, const T_STR sPre, const T_STR sLen, const T_STR sEnd )
	{
		typedef typename T_STR::value_type T;
		std::basic_string< char > sInData;

		disk::HFILE hIn = disk::Open( sIn.c_str(), "rb" );
		if ( disk::c_invalid_hfile == hIn )
			return -1;

		disk::HFILE hOut = disk::Open( sOut.c_str(), "wb" );
		if ( disk::c_invalid_hfile == hOut )
		{	disk::Close( hIn );
			return -1;
		} // end if

		// Write out the prefix
		if ( sPre.length() )
			disk::Write( sPre.data(), sizeof( T ), sPre.length(), hOut );

		// Char markers
		T *t = (T*)"0x.., ";
		long tl = zstr::Length( t );
		T *r = (T*)"0x..,\n\t";
		long rl = zstr::Length( r );

		// Buffers
		long bl = 0, lRead = 0, lTotal = 0;
		T in[ 64 * 1024 ], out[ sizeof( in ) ], *s;
		
		// Read in data in chunks
		while ( 0 < ( lRead = disk::Read( in, 1, sizeof( in ), hIn ) ) )
		{
			// Track the total bytes
			lTotal += lRead;

			// Convert each byte and write it out
			for ( long i = 0; i < lRead; i++ )
			{
				// Get a pointer to our spot in the buffer
				s = &out[ bl ];

				// Copy hex prefix
				if ( !( ( i + 1 ) & 0xf ) )
					memcpy( s, r, rl * sizeof( T ) ), bl += rl;
				else
					memcpy( s, t, tl * sizeof( T ) ), bl += tl;

				// Convert byte to ascii
				str::htoa( &s[ 2 * sizeof( T ) ], (T)in[ i ] );

				// Write data out if the buffer is getting full
				if ( ( sizeof( out ) - 128 ) < (unsigned long)bl )
					disk::Write( out, sizeof( T ), bl, hOut ), bl = 0;

			} // end for

			// Write whatever is left
			if ( bl )
				disk::Write( out, sizeof( T ), bl, hOut ), bl = 0;

		} // end while

		if ( sLen.length() )
		{	T_STR w = sLen + str::ToString< T_STR >( lTotal );
			disk::Write( w.data(), sizeof( T ), w.length(), hOut );
		} // end if

		// Write out the prefix
		if ( sEnd.length() )
			disk::Write( sEnd.data(), sizeof( T ), sEnd.length(), hOut );

		// Close the file handles
		disk::Close( hIn );
		disk::Close( hOut );

		// Return total bytes
		return lTotal;
	}

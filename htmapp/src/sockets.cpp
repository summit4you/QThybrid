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

#include "htmapp.h"

#if defined( _WIN32 )
#	include "sockets-windows.hpp"
#else
#	include "sockets-posix.hpp"
#endif

int CIpAddress::SetRawAddress( t_int64 x_llIp, unsigned int x_uPort, unsigned int x_uType )
{
    // Set the ip address
    if ( x_uType == eAddrTypeIpv4 )
        m_uIpv4 = (unsigned int)x_llIp, m_uIpv4Extra = 0;
    else m_llIpv6 = x_llIp;

    // Port number
    m_uPort = x_uPort;

    // Type information
    m_uType = x_uType;

    // Set the crc
    m_uCrc = 0;
	m_uCrc = (unsigned short)str::CRC32( &m_uid, sizeof( m_uid ) );

    return 1;
}

int CIpAddress::ValidateAddress() const
{
	// Temp copy, so this function can be const ;)
	CIpAddress t( *this );

	// Clear the crc while we hash
	t.m_uCrc = 0;

    // Create hash
    unsigned short u = (unsigned short)str::CRC32( &t.m_uid, sizeof( t.m_uid ) );

    // Verify the hash value
    if ( m_uCrc != u )
		return 0; 

    return 1;
}

CIpAddress::t_string CIpAddress::GetHostName()
{
	// Look up the host name
	char szHostName[ 1024 ] = { 0 };
	if( ::gethostname( szHostName, sizeof( szHostName ) ) )
		return t_string();

	return t_string( szHostName );
}

CIpAddress::t_string CIpAddress::GetFullHostName()
{
	t_string sDomain = GetDomainName();
	if ( sDomain.length() )
		return disk::FilePath( sDomain, GetHostName() );
	return GetHostName();
}

CIpAddress& CIpAddress::setUid( const uid::UID *x_pUid )
{
    uid::Copy( &m_uid, x_pUid );
    return *this;
}

int CIpAddress::LookupUri( const t_string &x_sUrl, unsigned int x_uPort, unsigned int x_uType )
{
    // Lose old info
    Destroy();

    // Ensure we have a valid pointer
    if ( !x_sUrl.length() )
        return 0;

    // Crack the url
    t_pb8 pbUri = parser::DecodeUri< t_pb8 >( x_sUrl );
    if ( !pbUri.size() )
        return 0;

    // Did we get a host name?
    if ( !pbUri[ "host" ].length() )
        return 0;

    // Get the port
    if ( !x_uPort )
        x_uPort = pbUri[ "port" ].ToLong();

    // Save the type
    m_uType = x_uType;

    return LookupHost( pbUri[ "host" ].str(), x_uPort );
}

int CIpSocket::Connect( const t_string &x_sAddress, unsigned int x_uPort )
{
	if ( !x_sAddress.length() )
        return 0;

	// Punt if not initialized
	if ( !IsInitialized() )
		return 0;

	CIpAddress addr;

    // Were we passed a URL?
    if ( !x_uPort && addr.LookupUri( x_sAddress ) )
		return Connect( addr );

	// Lookup the host address
    if ( addr.LookupHost( x_sAddress, x_uPort ) )
		return Connect( addr );

	return 0;
}

CIpSocket::t_string CIpSocket::RecvFrom( unsigned long x_uMax, unsigned long x_uFlags )
{
	// Do we have a size limit?
	if ( 0 < x_uMax )
	{
		// Allocate buffer
		t_string sBuf;
		try{ sBuf.resize( x_uMax ); }
		catch( ... ) { return sBuf; }
		if ( sBuf.length() >= x_uMax )
		{
			// Attempt to read data
			unsigned int uRead = RecvFrom( &sBuf[ 0 ], x_uMax, 0, x_uFlags );

			// Accept as the length
			sBuf.resize( uRead );

		} // end if

		return sBuf;

	} // end if

	// Allocate buffer
	t_string sBuf;
	unsigned int uRead = 0, uOffset = 0;

	// Allocate space
	const unsigned int uMin = 1024;
	try{ sBuf.resize( uMin ); }
	catch( ... ) { return sBuf; }
	if ( sBuf.length() < uMin )
		return sBuf;

	// Read all available data
	while ( 0 < ( uRead = RecvFrom( &sBuf[ uOffset ], uMin, 0, x_uFlags ) )
			&& uRead >= (unsigned int)uMin )
	{
		// Allocate more space
		uOffset += uRead;
		try{ sBuf.resize( uOffset + uMin ); }
		catch( ... ) { return sBuf; }
		if ( sBuf.length() < uOffset + uMin )
			return sBuf;

	} // end while

	// Set the length
	sBuf.resize( uOffset + uRead );

	return sBuf;
}

CIpSocket::t_string CIpSocket::Recv( unsigned long x_uMax, unsigned long x_uFlags )
{
	// Do we have a size limit?
	if ( 0 < x_uMax )
	{
		// Allocate buffer
		t_string sBuf;
		try{ sBuf.resize( x_uMax ); }
		catch( ... ) { return sBuf; }
		if ( sBuf.length() >= x_uMax )
		{
			// Attempt to read data
			unsigned int uRead = Recv( &sBuf[ 0 ], x_uMax, 0, x_uFlags );

			// Accept as the length
			sBuf.resize( uRead );

		} // end if

		return sBuf;

	} // end if

	// Allocate buffer
	t_string sBuf;
	unsigned int uRead = 0, uOffset = 0;

	// Allocate space
	const unsigned int uMin = 1024;
	try{ sBuf.resize( uMin ); }
	catch( ... ) { return sBuf; }
	if ( sBuf.length() < uMin )
		return sBuf;

	// Read all available data
	while ( 0 < ( uRead = Recv( &sBuf[ uOffset ], uMin, 0, x_uFlags ) )
			&& uRead >= (unsigned int)uMin )
	{
		// Allocate more space
		uOffset += uRead;
		try{ sBuf.resize( uOffset + uMin ); }
		catch( ... ) { return sBuf; }
		if ( sBuf.length() < uOffset + uMin )
			return sBuf;

	} // end while

	// Set the length
	sBuf.resize( uOffset + uRead );

	return sBuf;
}

unsigned int CIpSocket::hton_l( unsigned int v ) { return ::htonl( v ); }
unsigned int CIpSocket::ntoh_l( unsigned int v ) { return ::ntohl( v ); }
unsigned short CIpSocket::hton_s( unsigned short v ) { return ::htons( v ); }
unsigned short CIpSocket::ntoh_s( unsigned short v ) { return ::ntohs( v ); }

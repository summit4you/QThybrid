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

#include <stdlib.h>

#include <errno.h>
#include <signal.h>
#include <fcntl.h>

#include <sys/socket.h>
#if !defined( HTM_NOEPOLL )
#	include <sys/epoll.h>
#endif
#include <arpa/inet.h>
#include <netdb.h>

#if !defined( HTM_NOUNAME )
#	include <sys/utsname.h>
#endif

// A few verifications
// STATIC_ASSERT( sizeof( CIpSocket::t_SOCKET ) >= sizeof( int ) );

#ifndef EPOLLRDHUP
#	define EPOLLRDHUP	0x2000
#endif

//------------------------------------------------------------------
// Helper functions
//------------------------------------------------------------------

//==============================================================
// CIpSocket_GetAddressInfo()
//==============================================================
/// Copies the specified address
/**
	\param [in] x_pIa	-	Address structure to be filled in.
	\param [in] x_pSai 	-	Address information
*/
static int CIpSocket_GetAddressInfo( CIpAddress *x_pIa, sockaddr_in *x_pSai )
{
    // Verify pointers
    if ( !x_pSai || !x_pIa )
        return 0;

    // Sets the raw address value
    x_pIa->SetRawAddress( ntohl( (unsigned long)x_pSai->sin_addr.s_addr ), ntohs( x_pSai->sin_port ) );

    return 1;
}

//==============================================================
// CIpSocket_GetAddressInfo()
//=================================================e=============
/// Copies the specified address
/**
	\param [in] x_pIa	-	Address structure to be filled in.
	\param [in] x_pSa 	-	Address information
*/
static int CIpSocket_GetAddressInfo( CIpAddress *x_pIa, sockaddr *x_pSa )
{
   return CIpSocket_GetAddressInfo( x_pIa, (sockaddr_in*)x_pSa );
}

//==============================================================
// CIpSocket_SetAddressInfo()
//==============================================================
/// Copies the specified address
/**
	\param [in] x_pIa	-	Address information
	\param [in] x_pSai 	-	Structure to be filled in
*/
static int CIpSocket_SetAddressInfo( CIpAddress *x_pIa, sockaddr_in *x_pSai )
{
    // Verify pointers
    if ( !x_pSai || !x_pIa )
        return 0;

    // Set the ip address
    x_pSai->sin_addr.s_addr = htonl( (unsigned int)x_pIa->getIpv4() );

    // Set the port
    x_pSai->sin_port = htons( (short)x_pIa->getPort() );

    return 1;
}

//==============================================================
// CIpSocket_SetAddressInfo()
//==============================================================
/// Copies the specified address
/**
	\param [in] x_pIa	-	Address information
	\param [in] x_pSai 	-	Structure to be filled in
*/
/*
static int CIpSocket_SetAddressInfo( CIpAddress *x_pIa, sockaddr *x_pSa )
{
   return CIpSocket_GetAddressInfo( x_pIa, (sockaddr_in*)x_pSa );
}
*/

//------------------------------------------------------------------
// CIpAddress
//------------------------------------------------------------------

CIpAddress::t_string CIpAddress::GetDomainName( const t_string &x_sServer )
{
#if defined( HTM_NOUNAME )
	return t_string();
#else
	// Look up the domain name
	struct utsname un; memset( &un, 0, sizeof( un ) );
	if( 0 > ::uname( &un ) || !un.domainname )
		return t_string();

	return t_string( tcMb2Str( un.domainname ) );
#endif
}

// +++ Make IPv6 safe
int CIpAddress::SetDotAddress( const t_string &x_sDotAddress, unsigned int x_uPort, unsigned int x_uType )
{
    if ( !x_sDotAddress.length() )
        return 0;

    // Convert the dot address
    u_long ip = ntohl( inet_addr( x_sDotAddress.c_str() ) );
    if ( INADDR_NONE == ip )
        return 0;

    SetRawAddress( ip, x_uPort, x_uType );

    return 1;
}

// +++ Make IPv6 safe
CIpAddress::t_string CIpAddress::GetDotAddress()
{
    in_addr ia;

    // Put the address in the structure
    ia.s_addr = htonl( (u_long)getIpv4() );

	// Create dot address if needed
	return inet_ntoa( ia );
}

int CIpAddress::LookupHost( const t_string &x_sServer, unsigned int x_uPort, unsigned int x_uType )
{
    // Lose old info
    Destroy();

    // Ensure we have a valid pointer
    if ( !x_sServer.length() )
        return 0;

// +++ Get this working eventually
// #if !defined( HTM_USE_GETHOSTBYNAME )
#if 0

	in_addr ip4;
	if ( inet_pton( PF_INET, x_sServer.c_str(), &ip4 ) )
	{	SetRawAddress( ntohl( *(unsigned long*)&ip4.s_addr ), x_uPort, x_uType );
		return 1;
	} // end if

	struct addrinfo hints, *addr = 0;
	memset( &hints, 0, sizeof( hints ) );
	hints.ai_family = PF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_CANONNAME;

	int rval = 0;
    if ( rval = getaddrinfo( x_sServer.c_str(), 0, &hints, &addr ) )
		return 0;

	int max = 128;
	while ( !addr && max-- )
	{
		switch( addr->ai_family )
		{
			case AF_INET :
			{
				sockaddr_in *psai = (sockaddr_in*)addr->ai_addr;
				in_addr *pia = &psai->sin_addr;
			    SetRawAddress( ntohl( *(unsigned long*)&pia->s_addr ), x_uPort, x_uType );
			    return 1;

			} break;

			case AF_INET6 :
			{
				// +++ Add v6 support
				sockaddr_in6 *psai = (sockaddr_in6*)addr->ai_addr;
				sin6_addr *pia6 = &psai->sin6_addr;

			} break;

		} // end switch

		if ( addr == addr->ai_next )
			addr = 0;
		else
			addr = addr->ai_next;

	} // end while

	return 0;

#else

	// First try to interpret as dot address
	unsigned int uAddr = inet_addr( x_sServer.c_str() );
	if ( INADDR_NONE != uAddr )
	{   SetRawAddress( ntohl( uAddr ), x_uPort, x_uType );
		return 1;
	} // end if

    struct hostent *pHe;
	do { pHe = gethostbyname( x_sServer.c_str() );
	} while ( !pHe && EINTR == h_errno );

    if ( !pHe || !pHe->h_addr_list )
		return 0;

    in_addr *pia = (in_addr*)*pHe->h_addr_list;
    if ( !pia )
		return 0;

    SetRawAddress( ntohl( *(unsigned int*)&pia->s_addr ), x_uPort, x_uType );

    return 1;

#endif

}

//------------------------------------------------------------------
// CIpSocket
//------------------------------------------------------------------

long CIpSocket::m_lInit = -1;

static long g_CIpSocket_lInitCount = 0;

const CIpSocket::t_SOCKET CIpSocket::c_InvalidSocket = (CIpSocket::t_SOCKET)-1;
const CIpSocket::t_SOCKETEVENT CIpSocket::c_InvalidEvent = (CIpSocket::t_SOCKETEVENT)-1;
const CIpSocket::t_SOCKET CIpSocket::c_SocketError = (CIpSocket::t_SOCKET)-1;

unsigned long CIpSocket::FlagWinToNix( unsigned long x_nFlag )
{
#if defined( HTM_NOEPOLL )
	return 0;
#else

	long nRet = 0;

	if ( eReadEvent & x_nFlag )
		nRet |= EPOLLIN | EPOLLPRI;

	if ( eWriteEvent & x_nFlag )
		nRet |= EPOLLOUT;

	if ( eConnectEvent & x_nFlag )
		nRet |= EPOLLIN | EPOLLPRI | EPOLLOUT;

	if ( eCloseEvent & x_nFlag )
		nRet |= EPOLLHUP | EPOLLRDHUP;

//	if ( eCreateEvent & x_nFlag )
//		nRet |= ;

	// +++ I think the accept flag is the same as the read or write ???
	if ( eAcceptEvent & x_nFlag )
		nRet |= EPOLLIN | EPOLLPRI | EPOLLOUT;

	return nRet;
#endif
}

unsigned long CIpSocket::FlagNixToWin( unsigned long x_nFlag )
{
#if defined( HTM_NOEPOLL )
	return 0;
#else

	long nRet = 0;

	if ( ( EPOLLIN | EPOLLPRI ) & x_nFlag )
		nRet |= eReadEvent;

	if ( EPOLLOUT & x_nFlag )
		nRet |= eWriteEvent;

	if ( ( EPOLLIN | EPOLLPRI | EPOLLOUT ) & x_nFlag )
		nRet |= eConnectEvent;

	if ( ( EPOLLHUP | EPOLLRDHUP ) & x_nFlag )
		nRet |= eCloseEvent;

//	if(  & x_nFlag )
//		nRet |= eCreateEvent;

	// +++ I think the accept flag is the same as the read ???
	if ( ( EPOLLIN | EPOLLPRI | EPOLLOUT ) & x_nFlag )
		nRet |= eAcceptEvent;

	return nRet;
#endif
}

void CIpSocket::Construct()
{
	m_hSocket = c_InvalidSocket;

	m_hSocketEvent = c_InvalidEvent;

	m_lActivity = 0;

	m_uLastError = 0;

    m_uConnectState = 0;

    m_uEventState = 0;
    memset( &m_uEventStatus, 0, sizeof( m_uEventStatus ) );
    m_pEventObject = 0;

    m_uSocketFamily = 0;
    m_uSocketType = 0;
    m_uSocketProtocol = 0;

	m_uReads = 0;
	m_uWrites = 0;
	m_uAccepts = 0;
	m_uFlags = 0;

	m_bFree = 1;

	m_bEventsHooked = 0;

	m_lTimeout = 60000;

}

CIpSocket::CIpSocket()
{

	// Construct class
	Construct();
}

CIpSocket::CIpSocket( t_SOCKET hSocket, int x_bFree )
{
	// Construct class
	Construct();

	// Attach to specified socket
	Attach( hSocket, x_bFree );
}

CIpSocket::~CIpSocket()
{
	/// Lose the current socket
	Destroy();
}

long CIpSocket::GetInitCount()
{	return g_CIpSocket_lInitCount;
}

int CIpSocket::IsInitialized()
{
	return m_lInit ? 0 : 1; 
}

int CIpSocket::InitSockets()
{
	// Add ref
	if ( 1 == CThread::Increment( &g_CIpSocket_lInitCount ) )
	{
		// Quit if already initialized
		if ( !m_lInit )
			return 1;

		/// +++ Don't need init in linux?
		m_lInit = 0;

	} // end if

	// Don't cause SIGPIPE
	signal( SIGPIPE, SIG_IGN );

	return IsInitialized();
}

void CIpSocket::UninitSockets()
{
	// Deref
	if ( CThread::Decrement( &g_CIpSocket_lInitCount ) )
		return;

	// Punt if not initialized
	if ( !IsInitialized() )
		return;

	// Not initialized
	m_lInit	= -1;
}

CIpSocket::t_SOCKET CIpSocket::Detach()
{   
	// Save away the socket handle
	t_SOCKET hSocket = m_hSocket;

	// Ditch the event handle
	CloseEventHandle();

	// We won't be freeing the socket
	m_hSocket = c_InvalidSocket;
	
	// Free whatever else is left
	Destroy();

	// It's the callers problem now
	return hSocket;
}

void CIpSocket::Destroy()
{
	// Let everyone know we're closing
	if ( c_InvalidSocket != m_hSocket )
		OnClose();

	// Ditch the event handle
	CloseEventHandle();

	// Save socket pointer to socket
	t_SOCKET hSocket = m_bFree ? m_hSocket : c_InvalidSocket;

	// Ditch member variable
	m_hSocket = c_InvalidSocket;

	// Invalidate member variables
    m_sConnect.clear();
    m_addrPeer.Destroy();
    m_addrLocal.Destroy();

    m_uConnectState = 0;
    m_uEventState = 0;
    memset( &m_uEventStatus, 0, sizeof( m_uEventStatus ) );

    m_uSocketFamily = 0;
    m_uSocketType = 0;
    m_uSocketProtocol = 0;

    m_uConnectState = 0;

	m_uReads = 0;
	m_uWrites = 0;
	m_uAccepts = 0;
	m_uFlags = 0;
	m_lActivity = 0;

	m_bFree = 1;

	// Ensure valid socket handle
	if ( c_InvalidSocket == hSocket )
		return;

	if ( IsInitialized() )
	{
		// Turn off non-blocking
//		int flags = fcntl( tcPtrToULong( hSocket ), F_GETFL, 0 );
//		fcntl( tcPtrToULong( m_hSocket ), F_SETFL, flags & ~O_NONBLOCK );

		struct linger lopt;
		lopt.l_onoff = 1;
		lopt.l_linger = 60;

		if ( -1 == setsockopt( tcPtrToULong( hSocket ), SOL_SOCKET, SO_LINGER, &lopt, sizeof( lopt ) ) )
			m_uLastError = errno;

		// Shutdown the socket
//		if ( -1 == shutdown( tcPtrToULong( hSocket ), SHUT_RDWR ) )
//		{	m_uLastError = errno;
//			if ( ENOTCONN != errno )
//				;
//		} // end if

		// Close the socket
		if ( -1 == close( tcPtrToULong( hSocket ) ) )
			m_uLastError = errno;

	} // end if

}

int CIpSocket::Shutdown()
{
	if ( c_InvalidSocket == m_hSocket )
        return 0;

    // Shut down the socket
    if ( -1 == shutdown( tcPtrToULong( m_hSocket ), SHUT_RDWR ) )
		m_uLastError = errno;

    return 1;
}


int CIpSocket::Create( int x_af, int x_type, int x_protocol, int x_timeout )
{
	// Punt if not initialized
	if ( !IsInitialized() )
		return 0;

	// Close any open socket
	Destroy();

	// Create a scocket
	m_hSocket = (t_SOCKET)tcULongToPtr( socket( (int)x_af, (int)x_type, (int)x_protocol ) );

	if ( c_InvalidSocket == m_hSocket )
    {	m_uLastError = errno;
		return 0;
	} // end if

	// Default timeout?
	if ( 0 >= x_timeout )
		x_timeout = m_lTimeout;
	
	// Setup socket timeout defaults
	struct timeval tv;
	tv.tv_sec = ( x_timeout / 1000 );
	tv.tv_usec = ( x_timeout % 1000 ) * 1000;
	setsockopt( tcPtrToULong( m_hSocket ), SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof( tv ) );
	setsockopt( tcPtrToULong( m_hSocket ), SOL_SOCKET, SO_SNDTIMEO, (const char *)&tv, sizeof( tv ) );

	// Process socket creation
	if ( !OnAttach() )
	{	Destroy();
		return 0;
	} // end if

	m_uLastError = 0;

    // Save socket connect params
    m_uSocketFamily = x_af;
    m_uSocketType = x_type;
    m_uSocketProtocol = x_protocol;

	int set = 1;
	if ( -1 == setsockopt( tcPtrToULong( m_hSocket ), SOL_SOCKET, SO_REUSEADDR, &set, sizeof(set) ) )
//		WARNING( errno, tcT( "socket interface does not support SO_REUSEADDR" ) );
		;

	// Capture all events
	EventSelect();

	return IsSocket();
}

int CIpSocket::Bind( unsigned int x_uPort )
{
	// Punt if not initialized
	if ( !IsInitialized() )
		return 0;

	// Create socket if there is none
	if ( !IsSocket() && !Create() )
	{	Destroy(); return 0; }

	sockaddr_in sai;
	memset( &sai, 0, sizeof( sai ) );
	sai.sin_family = PF_INET;
	sai.sin_port = htons( (unsigned short)x_uPort );

	// Attempt to bind the socket
	int nRet = bind( tcPtrToULong( m_hSocket ), (sockaddr*)&sai, sizeof( sockaddr_in ) );

	if ( -1 == nRet )
		m_uLastError = errno;
	else
	{
		m_uLastError = 0;

	    // Grab the address
    	CIpSocket_GetAddressInfo( &m_addrLocal, &sai );

	} // end if

	return !nRet;
}

int CIpSocket::Listen( unsigned int x_uMaxConnections )
{
	// Punt if not initialized
	if ( !IsInitialized() )
		return 0;

	// Must have socket
	if ( !IsSocket() )
        return 0;

	// Valid number of connections?
	if ( x_uMaxConnections == 0 )
        x_uMaxConnections = SOMAXCONN;

	// Start the socket listening
	int nRet = listen( tcPtrToULong( m_hSocket ), (int)x_uMaxConnections );

	if ( -1 == nRet )
    {	m_uLastError = errno;
		return 0;
	} // end if

	m_uLastError = 0;

	// We're trying to connect
	m_lActivity++;
	m_uConnectState |= eCsConnecting;

    // Return the result
	return !nRet;
}

int CIpSocket::Connect( const CIpAddress &x_rIpAddress )
{
	// Punt if not initialized
	if ( !IsInitialized() )
		return 0;

	// Ensure we were passed a valid address
	if ( !x_rIpAddress.ValidateAddress() )
	{	Destroy(); return 0; }

	// Create socket if there is none
	if ( !IsSocket() && !Create() )
	{	m_uConnectState |= eCsError;
		Destroy();
		return 0;
	} // end if

	// Save the address
	m_addrPeer = x_rIpAddress;

    sockaddr_in si;
    memset( &si, 0, sizeof( si ) );
    si.sin_family = m_uSocketFamily;
    CIpSocket_SetAddressInfo( &m_addrPeer, &si );

    // Attempt to connect
    int nRet = connect( tcPtrToULong( m_hSocket ), (sockaddr*)&si, sizeof( si ) );

	m_uLastError = errno;

	// Check result
	if ( -1 == nRet && EINPROGRESS != m_uLastError )
	{	m_uConnectState |= eCsError;
    	m_uLastError = errno;
		return 0;
	} // end if

	m_uLastError = 0;

	// We're trying to connect
	m_lActivity++;
	m_uConnectState |= eCsConnecting;

	return 1;
}


int CIpSocket::Attach( t_SOCKET x_hSocket, int x_bFree )
{
	// Lose the old socket
    Destroy();

	// Save socket handle
    m_hSocket = x_hSocket;

	// Should we free the socket?
	m_bFree = x_bFree;

	// Call on attach
	if ( !OnAttach() )
		Destroy();

	// How'd it go?
    return IsSocket();
}

int CIpSocket::Accept( CIpSocket &x_is )
{
    // Punt if no socket
	if ( !IsSocket() )
		return 0;

    // Lose any current connection there might be
    x_is.Destroy();

	// Accept the connection
	sockaddr saAddr;
	socklen_t iAddr = sizeof( saAddr );

	// Accept and encapsulate the socket
	int bSuccess = x_is.Attach( (t_SOCKET)tcULongToPtr( accept( tcPtrToULong( m_hSocket ), &saAddr, &iAddr ) ) );

	if ( !bSuccess )
	{
		m_uLastError = errno;
		
		if ( EAGAIN != m_uLastError )
			m_uEventState |= eCloseEvent;

		else 
			m_uEventState = 0;

		return 0;

	} // end if
		
	else
		m_uLastError = 0;

    // Grab the address
    CIpSocket_GetAddressInfo( &m_addrPeer, &saAddr );

	// Capture all events
	x_is.EventSelect();

	// Accepted
	m_uAccepts++;
	m_lActivity++;

	// Child is connecting
	x_is.m_uConnectState |= eCsConnecting;

    return 1;
}

int CIpSocket::CreateEventHandle()
{
#if defined( HTM_NOEPOLL )
	return 0;
#else

    // Check for event handle
    if ( IsEventHandle() )
        return 1;

	int id = epoll_create( eMaxEvents );
	if ( -1 == id )
	{	m_uLastError = errno;
		return 0;
	} // end if
	else
		m_uLastError = 0;
		
	// Create an event handle
	m_hSocketEvent = (void*)id;
		
	// Create event object array and initialize
	m_pEventObject = calloc( eMaxEvents, sizeof( epoll_event ) );
	
    return 1;
#endif
}

void CIpSocket::CloseEventHandle()
{
#if defined( HTM_NOEPOLL )
	return;
#else

	if ( IsInitialized() )
		if ( c_InvalidEvent != m_hSocketEvent )
		{
			// Unhook events
			if ( m_bEventsHooked )
			{
				m_bEventsHooked = 0;

				epoll_event ev; 
				memset( &ev, 0, sizeof( ev ) );
				ev.data.fd = tcPtrToULong( m_hSocket );
				ev.events = 0;

				// Set the event masks
				int nRes = epoll_ctl( tcPtrToULong( m_hSocketEvent ), EPOLL_CTL_DEL, tcPtrToULong( m_hSocket ), &ev );

				if ( -1 == nRes )
					m_uLastError = errno;

			} // end if

			// Close event handle
			if ( -1 == close( tcPtrToULong( m_hSocketEvent ) ) )
				m_uLastError = errno;

			else
				m_uLastError = 0;

		} // end if

	// Give up on socket event handle
	m_hSocketEvent = c_InvalidEvent;

	// Release event object
	if ( m_pEventObject )
		free( (epoll_event*)m_pEventObject );

	m_pEventObject = 0;
	m_bEventsHooked = 0;

#endif
}

int CIpSocket::EventSelect( long x_lEvents )
{
#if defined( HTM_NOEPOLL )
	return 0;
#else
    // Punt if no socket
	if ( !IsSocket() )
		return 0;

	// Enable non-blocking mode
	int flags = fcntl( tcPtrToULong( m_hSocket ), F_GETFL, 0 );
	fcntl( tcPtrToULong( m_hSocket ), F_SETFL, flags | O_NONBLOCK );

    // Must have event handle
    if ( !IsEventHandle() || !m_pEventObject )
        CreateEventHandle();

	// Clear events
	m_uEventState = 0;

	epoll_event ev;
	memset( &ev, 0, sizeof( ev ) );
	ev.data.fd = tcPtrToULong( m_hSocket );
	ev.events = EPOLLERR | FlagWinToNix( x_lEvents );

	// Set the event masks
	int nRes = epoll_ctl( tcPtrToULong( m_hSocketEvent ),
						  m_bEventsHooked ? EPOLL_CTL_MOD : EPOLL_CTL_ADD,
						  tcPtrToULong( m_hSocket ), &ev );

	if ( -1 == nRes )
    {	m_uLastError = errno;
		return 0;
	} // end if

	m_bEventsHooked = 1;

	return 1;
#endif
}

long CIpSocket::WaitEvent( long x_lEventId, long x_lTimeout )
{
#if defined( HTM_NOEPOLL )
	return 0;
#else

	// Must have a socket handle
	if ( !IsSocket() )
        return 0;

    // Must have event handle
    if ( !IsEventHandle() || !m_pEventObject )
    {
    	if ( !CreateEventHandle() )
    		return 0;

    	if ( !EventSelect() )
            return 0;

    } // end if

	// +++ Ensure our event is being waited on?

	// Grab pointer to event object
	epoll_event *pev = (epoll_event*)m_pEventObject;

	// Check for default timeout
	if ( 0 > x_lTimeout )
		x_lTimeout = (long)m_lTimeout;

	// Save start time
	unsigned int uEnd = time( 0 ) + x_lTimeout / 1000;
	for( ; ; ) // forever
	{
        // What's the event state
        if ( 0 == ( m_uEventState & x_lEventId ) )
        {
			// Wait only for our specific event
			EventSelect( x_lEventId );

			// Wait for events
			int nRes;
			do { 
				nRes = epoll_wait( tcPtrToULong( m_hSocketEvent ), pev, eMaxEvents, x_lTimeout );

			} while ( -1 == nRes && EINTR == errno );

			if ( -1 == nRes )
			{
				// Log error
				m_uLastError = errno;

				// Disconnected?
				m_uConnectState |= eCsError;

				// Just ditch if they aren't waiting for the close event
				m_uEventState |= eCloseEvent;
				if ( !( x_lEventId & eCloseEvent ) )
					return 0;

			} // end if

			// Process all events
			if ( 0 < nRes )
				for ( long i = 0; i < nRes; i++ )
				{
					// Convert to windows flags
					unsigned long uFlags = FlagNixToWin( pev[ i ].events );

					// Save the status of all events
					if ( pev[ i ].data.fd == tcPtrToULong( m_hSocket ) )
					{	pev[ i ].events = 0;
						for ( unsigned long uMask = 1; uMask < eAllEvents; uMask <<= 1 )
							if ( uFlags & uMask )
							{
								// Get bit offset
								unsigned long uOffset = GetEventBit( uMask );

								// Save the event info
								m_uEventState |= uMask;
								m_uEventStatus[ uOffset ] = 0;

								// Attempt to detect connect error
								if ( 0 != ( uMask & eConnectEvent ) )
								{
/* +++ Nope, doesn't always work
									// use getpeername() to check for errors
									sockaddr_in sai; 
									memset( &sai, 0, sizeof( sai ) );
									socklen_t len = sizeof( sai );
									if ( -1 == getpeername( tcPtrToULong( m_hSocket ), (sockaddr*)&sai, &len ) )
									{	m_uLastError = errno;
										m_uEventStatus[ uOffset ] = errno;
										m_uConnectState |= eCsError;
									} // end if
*/
/* +++ gives error : Resource temporarily unavailable
									char buf[ 1 ];
									if ( -1 == recv( tcPtrToULong( m_hSocket ), buf, 0, 0 ) )
									{	m_uLastError = errno;
										m_uEventStatus[ uOffset ] = errno;
										m_uConnectState |= eCsError;
									} // end if
*/
								}

								// Handle close event
								if ( 0 != ( uMask & eCloseEvent ) )
									m_uConnectState &= ~eCsConnected;
								else
									m_uConnectState |= eCsActivity;

								// +++ Signal when we get a connect message
//								if ( 0 != ( ( EPOLLIN | EPOLLOUT ) & uMask ) )
//									m_uConnectState |= 2;

							} // end if

					} // end if

				} // end for

            // !!!  Kludge around missing connect message
            if ( !( m_uConnectState & eCsConnected ) && ( m_uConnectState & eCsActivity ) )
            {   m_uConnectState |= eCsConnected;
//				m_uConnectState &= ~eCsError;
                m_uEventState |= eConnectEvent;
                m_uEventStatus[ eConnectBit ] = 0;
            } // end if

        } // end if

		// Did our event go off?
		if ( 0 != ( m_uEventState & x_lEventId ) )
		{
            // Get the first event
            unsigned long uBit = GetEventBit( x_lEventId & m_uEventState );
            unsigned long uMask = 1 << uBit;

            // Acknowledge this event
            // m_uEventState &= ~uMask;

			// Save the error code
			if ( m_uEventStatus[ uBit ] )
				m_uLastError = m_uEventStatus[ uBit ];

			// Something is going on
			m_lActivity++;

			// We received the event
			return uMask;

		} // end if

		// Have we timed out?
		unsigned long uTick = time( 0 );
		if ( uEnd <= uTick )
            return 0;

		// Adjust timeout
		x_lTimeout = ( uEnd - uTick ) * 1000;

	} // end if

	// Can't get here...
	return 0;
#endif
}

long CIpSocket::GetEventBit( long x_lEventMask )
{
    // !!!  Events will be returned by WaitEvent() in the order
    //      they appear below.

	if ( 0 != ( eConnectEvent & x_lEventMask ) )
        return eConnectBit;

	if ( 0 != ( eAcceptEvent & x_lEventMask ) )
        return eAcceptBit;

	if ( 0 != ( eWriteEvent & x_lEventMask ) )
        return eWriteBit;

	if ( 0 != ( eReadEvent & x_lEventMask ) )
        return eReadBit;

	if ( 0 != ( eOobEvent & x_lEventMask ) )
        return eOobBit;

	if ( 0 != ( eQosEvent & x_lEventMask ) )
        return eQosBit;

	if ( 0 != ( eGroupQosEvent & x_lEventMask ) )
        return eGroupQosBit;

	if ( 0 != ( eRoutingInterfaceChangeEvent & x_lEventMask ) )
        return eRoutingInterfaceChangeBit;

	if ( 0 != ( eCloseEvent & x_lEventMask ) )
        return eCloseBit;

	return 0;
}

CIpSocket::t_string CIpSocket::GetErrorMsg( unsigned long x_uErr )
{
	m_sError = str::StrFmt< t_string >( tcT( "0x%X (%d) : %s" ), x_uErr, x_uErr, strerror( x_uErr ) );
	return m_sError;
}
/*
	switch( x_uErr )
	{
		case EPERM:
			ptr = tcT( "Operation not permitted" );
			break;
		case ENOENT:
			ptr = tcT( "No such file or directory" );
			break;
		case ESRCH:
			ptr = tcT( "No such process" );
			break;
		case EINTR:
			ptr = tcT( "Interrupted system call" );
			break;
		case EIO:
			ptr = tcT( "I/O error" );
			break;
		case ENXIO:
			ptr = tcT( "No such device or address" );
			break;
		case E2BIG:
			ptr = tcT( "Arg list too long" );
			break;
		case ENOEXEC:
			ptr =tcT( "Exec format error" );
			break;
		case EBADF:
			ptr =tcT( "Bad file number" );
			break;
		case ECHILD:
			ptr =tcT( "No child processes" );
			break;
		case EAGAIN:
			ptr =tcT( "Try again" );
			break;
		case ENOMEM:
			ptr =tcT( "Out of memory" );
			break;

	} // end switch
*/

int CIpSocket::v_recvfrom( int socket, void *buffer, int length, int flags )
{
    sockaddr_in si;
    socklen_t nSize = sizeof( si );
    memset( &si, 0, sizeof( si ) );
    si.sin_family = m_uSocketFamily;

	// Receive data from socket
	int res = recvfrom( socket, buffer, length, flags, (sockaddr*)&si, &nSize );

    // Save the address
    CIpSocket_GetAddressInfo( &m_addrPeer, &si );

	return res;
}

unsigned long CIpSocket::RecvFrom( void *x_pData, unsigned long x_uSize, unsigned long *x_puRead, unsigned long x_uFlags )
{
	// Initialize bytes read
	if ( x_puRead )
        *x_puRead = 0;

	// Must have a socket handle
	if ( !IsSocket() )
        return 0;

	// Receive data from socket
	x_uFlags |= MSG_NOSIGNAL;
	int nRes = v_recvfrom( tcPtrToULong( m_hSocket ), x_pData, (int)x_uSize, (int)x_uFlags );

	m_uLastError = errno;

	m_uReads++;
	m_lActivity++;

	if ( -1 == nRes && m_uLastError != EAGAIN )
    {	m_uEventState |= eCloseEvent;
		m_uConnectState |= eCsError;
		if ( x_puRead )
            *x_puRead = 0;
		return 0;
	} // end if

	m_uLastError = 0;

	// Check for closed socket
	if ( !nRes && m_uLastError != EAGAIN )
	{	m_uEventState |= eCloseEvent;
        return -1;
	} // end if

	// Check for socket error
	if ( -1 == nRes || x_uSize < (unsigned long)nRes  || 0 > nRes )
	{
		if ( EAGAIN == m_uLastError )
			m_uEventState &= ~EPOLLIN;
	
		// Nothing read
		if ( x_puRead )
            *x_puRead = 0;

		return 0;

	} // end if

	// Save the number of bytes read
	if ( x_puRead )
        *x_puRead = nRes;

	return nRes;
}

int CIpSocket::v_recv( int socket, void *buffer, int length, int flags )
{
	return recv( socket, buffer, length, flags );
}

unsigned long CIpSocket::Recv( void *x_pData, unsigned long x_uSize, unsigned long *x_puRead, unsigned long x_uFlags )
{
	// Initialize bytes read
	if ( x_puRead )
        *x_puRead = 0;

	// Must have a socket handle
	if ( !IsSocket() )
        return 0;

	// Receive data from socket
	x_uFlags |= MSG_NOSIGNAL;
	int nRes = v_recv( tcPtrToULong( m_hSocket ), x_pData, (int)x_uSize, (int)x_uFlags );

	m_uLastError = errno;

	m_uReads++;
	m_lActivity++;

	if ( -1 == nRes && m_uLastError != EAGAIN )
    {	m_uEventState |= eCloseEvent;
		m_uConnectState |= eCsError;
		if ( x_puRead )
            *x_puRead = 0;
		return -1;
	} // end if

	// Check for closed socket
	if ( !nRes && m_uLastError != EAGAIN )
	{	m_uEventState |= eCloseEvent;
        return -1;
	} // end if

	// Check for socket error
	if ( x_uSize < (unsigned long)nRes  || 0 >= nRes )
	{
		if ( EAGAIN == m_uLastError )
			m_uEventState &= ~EPOLLIN;

		// Nothing read
		if ( x_puRead )
            *x_puRead = 0;

		return 0;

	} // end if

	m_uLastError = 0;

	// Save the number of bytes read
	if ( x_puRead )
        *x_puRead = (unsigned long)nRes;

	return nRes;
}

int CIpSocket::v_sendto(int socket, const void *message, int length, int flags )
{
    // Use the peer address
    sockaddr_in si;
    memset( &si, 0, sizeof( si ) );
    si.sin_family = m_uSocketFamily;
    CIpSocket_SetAddressInfo( &m_addrPeer, &si );

	// Send the data
	return sendto( socket, message, length, flags, (sockaddr*)&si, sizeof( si ) );
}

unsigned long CIpSocket::SendTo( const void *x_pData, unsigned long x_uSize, unsigned long *x_puSent, unsigned long x_uFlags )
{
	// Initialize bytes sent
	if ( x_puSent )
        *x_puSent = 0;

	// Must have a socket handle
	if ( !IsSocket() )
        return 0;

    // Send the data
	x_uFlags |= MSG_NOSIGNAL;
    int nRes = v_sendto( tcPtrToULong( m_hSocket ), x_pData, (int)x_uSize, (int)x_uFlags );

	m_uLastError = errno;

	m_uWrites++;
	m_lActivity++;

	// Check for error
	if ( -1 == nRes && m_uLastError != EAGAIN )
	{
		m_uEventState |= eCloseEvent;
		m_uConnectState |= eCsError;

		// Number of bytes sent
		if ( x_puSent )
            *x_puSent = 0;

		// Error
		return 0;

	} // end if

	m_uLastError = 0;

	// Check for socket error
	if ( x_uSize < (unsigned long)nRes  || 0 > nRes )
	{
		if ( EAGAIN == m_uLastError )
			m_uEventState &= ~EPOLLOUT;

		// Nothing written
		if ( x_puSent )
            *x_puSent = 0;

		return 0;

	} // end if
	
	// Save the number of bytes sent
	if ( x_puSent )
        *x_puSent = nRes;

	return nRes;
}

int CIpSocket::v_send( int socket, const void *buffer, int length, int flags )
{
	return send( socket, buffer, length, flags );
}

unsigned long CIpSocket::Send( const void *x_pData, unsigned long x_uSize, unsigned long *x_puSent, unsigned long x_uFlags )
{
	// Initialize bytes sent
	if ( x_puSent )
        *x_puSent = 0;

	// Must have a socket handle
	if ( !IsSocket() )
        return 0;

	// Attempt to send the data
	x_uFlags |= MSG_NOSIGNAL;
	int nRes = v_send( tcPtrToULong( m_hSocket ), x_pData, (int)x_uSize, (int)x_uFlags );

	m_uLastError = errno;

	m_uWrites++;
	m_lActivity++;

	// Check for error
	if ( -1 == nRes && m_uLastError != EAGAIN )
	{
		m_uEventState |= eCloseEvent;
		m_uConnectState |= eCsError;

		// Number of bytes sent
		if ( x_puSent )
            *x_puSent = 0;

		// Error
		return 0;

	} // end if

	m_uLastError = 0;

	// Check for socket error
	if ( x_uSize < (unsigned long)nRes  || 0 > nRes )
	{
		if ( EAGAIN == m_uLastError )
			m_uEventState &= ~EPOLLOUT;

		// Nothing written
		if ( x_puSent )
            *x_puSent = 0;

		return 0;

	} // end if
	
	// Save the number of bytes sent
	if ( x_puSent )
        *x_puSent = nRes;

	return nRes;
}

int CIpSocket::GetPeerAddress( t_SOCKET x_hSocket, CIpAddress *x_pIa )
{
    if ( !x_pIa )
        return 0;

    // Reset address information
    x_pIa->Destroy();

    // Ensure socket
    if ( c_InvalidSocket == x_hSocket )
        return 0;

    // Init socket structure
	sockaddr_in sai;
    memset( &sai, 0, sizeof( sai ) );
	socklen_t len = sizeof( sai );

	// Get the socket info
	if ( -1 == getpeername( tcPtrToULong( x_hSocket ), (sockaddr*)&sai, &len ) )
		if ( ENOTCONN != errno )
			return 0;

    // Format the info
    return CIpSocket_GetAddressInfo( x_pIa, &sai );

}

int CIpSocket::GetLocalAddress( t_SOCKET x_hSocket, CIpAddress *x_pIa )
{
    if ( !x_pIa )
        return 0;

    // Reset address information
    x_pIa->Destroy();

    // Ensure socket
    if ( c_InvalidSocket == x_hSocket )
        return 0;

    // Init socket structure
	sockaddr_in sai;
    memset( &sai, 0, sizeof( sai ) );
	socklen_t len = sizeof( sai );

	// Get the socket info
	if ( -1 == getsockname( tcPtrToULong( x_hSocket ), (sockaddr*)&sai, &len ) )
		return 0;

    // Format the info
    return CIpSocket_GetAddressInfo( x_pIa, &sai );
}

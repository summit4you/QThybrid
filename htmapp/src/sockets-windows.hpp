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
#include <winsock2.h>

#include "windows.h"

// A few verifications
// STATIC_ASSERT( sizeof( CIpSocket::t_SOCKET ) == sizeof( SOCKET ) );

#if !defined( HTM_NOSOCKET2 )
 //STATIC_ASSERT( sizeof( CIpSocket::t_SOCKETEVENT ) == sizeof( WSAEVENT ) );
#endif

// Socket version we will use
const WORD c_MinSocketVersion = MAKEWORD( 2, 2 );

//////////////////////////////////////////////////////////////////////
// Helper functions
//////////////////////////////////////////////////////////////////////

//==============================================================
// CIpSocket_GetAddressInfo()
//==============================================================
/// Copies the specified address
/**
	\param [in] x_pIa	-	Address structure to be filled in.
	\param [in] x_pSai 	-	Address information
*/
static int CIpSocket_GetAddressInfo( CIpAddress *x_pIa, SOCKADDR_IN *x_pSai )
{
	// Verify pointers
	if ( !x_pSai || !x_pIa )
		return 0;

	// Sets the raw address value
	x_pIa->SetRawAddress( ntohl( *(DWORD*)&x_pSai->sin_addr.S_un.S_addr ), ntohs( x_pSai->sin_port ) );

	return 1;
}

//==============================================================
// CIpSocket_GetAddressInfo()
//==============================================================
/// Copies the specified address
/**
	\param [in] x_pIa	-	Address structure to be filled in.
	\param [in] x_pSa 	-	Address information
*/
static int CIpSocket_GetAddressInfo( CIpAddress *x_pIa, SOCKADDR *x_pSa )
{
	return CIpSocket_GetAddressInfo( x_pIa, (SOCKADDR_IN*)x_pSa );
}

//==============================================================
// CIpSocket_SetAddressInfo()
//==============================================================
/// Copies the specified address
/**
	\param [in] x_pIa	-	Address information
	\param [in] x_pSai 	-	Structure to be filled in
*/
static int CIpSocket_SetAddressInfo( CIpAddress *x_pIa, SOCKADDR_IN *x_pSai )
{
	// Verify pointers
	if ( !x_pSai || !x_pIa )
		return 0;

	// Set the ip address
	*(DWORD*)&x_pSai->sin_addr.S_un.S_addr = htonl( (DWORD)x_pIa->getIpv4() );

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
* /
/ *
static int CIpSocket_SetAddressInfo( CIpAddress *x_pIa, SOCKADDR *x_pSa )
{
	return CIpSocket_GetAddressInfo( x_pIa, (SOCKADDR_IN*)x_pSa );
}
*/

#define NET_API_FUNCTION __stdcall
typedef WCHAR* LMSTR;
typedef DWORD NET_API_STATUS;
typedef struct _WKSTA_INFO_100
{	DWORD wki100_platform_id;
	LMSTR wki100_computername;
	LMSTR wki100_langroup;
	DWORD wki100_ver_major;
	DWORD wki100_ver_minor;
} WKSTA_INFO_100, *PWKSTA_INFO_100, *LPWKSTA_INFO_100;

typedef NET_API_STATUS (NET_API_FUNCTION *pfn_NetWkstaGetInfo)( LPWSTR servername, DWORD level, LPBYTE *bufptr );
typedef NET_API_STATUS (NET_API_FUNCTION *pfn_NetApiBufferFree)( LPVOID Buffer );

CIpAddress::t_string CIpAddress::GetDomainName( const t_string &x_sServer )
{
	CIpAddress::t_string sRet;

	// Load netapi32.dll
	HMODULE hLib = LoadLibrary( tcT( "netapi32.dll" ) );
	if ( !hLib )
		return sRet;

	// Get function pointers
	pfn_NetApiBufferFree pNetApiBufferFree = (pfn_NetApiBufferFree)GetProcAddress( hLib, tcT( "NetApiBufferFree" ) );
	pfn_NetWkstaGetInfo pNetWkstaGetInfo = (pfn_NetWkstaGetInfo)GetProcAddress( hLib, tcT( "NetWkstaGetInfo" ) );

	// Attempt to read the domain name
	WKSTA_INFO_100 *pwi100 = 0;
	if ( pNetWkstaGetInfo
		 && !pNetWkstaGetInfo( x_sServer.length() ? (LPWSTR)tcStr2Wc( x_sServer ).c_str() : 0, 100, (LPBYTE*)&pwi100 ) )
		if ( pwi100 && pwi100->wki100_langroup )
			sRet = tcWc2Str( pwi100->wki100_langroup );

	// Free buffer
	if ( pNetApiBufferFree && pwi100 )
		pNetApiBufferFree( pwi100 );

	// Free library
	FreeLibrary( hLib );

	// Send the domain name along
	return sRet;
}

int CIpAddress::SetDotAddress( const t_string &x_sDotAddress, unsigned int x_uPort, unsigned int x_uType )
{
#if defined( HTM_NOSOCKET2 )
	return 0;
#else
    if ( !x_sDotAddress.length() )
        return 0;

    // Convert the dot address
    u_long ip = ntohl( inet_addr( x_sDotAddress.c_str() ) );
    if ( INADDR_NONE == ip )
        return 0;

    SetRawAddress( ip, x_uPort, x_uType );

    return 1;
#endif
}

CIpAddress::t_string CIpAddress::GetDotAddress()
{
#if defined( HTM_NOSOCKET2 )
	return 0;
#else
    in_addr ia;

    // Put the address in the structure
    ia.S_un.S_addr = htonl( (u_long)getIpv4() );

	// Create dot address if needed
	return inet_ntoa( ia );
#endif
}

int CIpAddress::LookupHost( const t_string &x_sServer, unsigned int x_uPort, unsigned int x_uType )
{
#if defined( HTM_NOSOCKET2 )
	return 0;
#else
    // Lose old info
    Destroy();

    // Ensure we have a valid pointer
    if ( !x_sServer.length() )
        return 0;

	// First try to interpret as dot address
	unsigned long uAddr = inet_addr( x_sServer.c_str() );
	if ( INADDR_NONE == uAddr )
    {
        LPHOSTENT pHe = gethostbyname( x_sServer.c_str() );

        if ( !pHe )
            return 0;

        LPIN_ADDR pia = (LPIN_ADDR)*pHe->h_addr_list;
        if ( !pia )
            return 0;

        // Grab the address
        uAddr = *(DWORD*)&pia->S_un.S_addr;

    } // end if

    SetRawAddress( ntohl( uAddr ), x_uPort, x_uType );

    return 1;
#endif
}

long CIpSocket::m_lInit = -1;
static long g_CIpSocket_lInitCount = 0;

const CIpSocket::t_SOCKET CIpSocket::c_InvalidSocket = (CIpSocket::t_SOCKET)INVALID_SOCKET;
#if !defined( HTM_NOSOCKET2 )
const CIpSocket::t_SOCKETEVENT CIpSocket::c_InvalidEvent = (CIpSocket::t_SOCKETEVENT)WSA_INVALID_EVENT;
#else
const CIpSocket::t_SOCKETEVENT CIpSocket::c_InvalidEvent = (CIpSocket::t_SOCKETEVENT)SOCKET_ERROR;
#endif
const CIpSocket::t_SOCKET CIpSocket::c_SocketError = (CIpSocket::t_SOCKET)SOCKET_ERROR;


void CIpSocket::Construct()
{
	// Initialize socket library
	InitSockets();

	m_hSocket = c_InvalidSocket;

	m_hSocketEvent = c_InvalidEvent;

	m_uLastError = 0;

	m_uConnectState = 0;

	m_uEventState = 0;
	memset( &m_uEventStatus, 0, sizeof( m_uEventStatus ) );

	m_uSocketFamily = 0;
	m_uSocketType = 0;
	m_uSocketProtocol = 0;

	m_uReads = 0;
	m_uWrites = 0;
	m_uAccepts = 0;

	m_uFlags = 0;

	m_bFree = 0;

	m_lActivity = 0;

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

	// Lose the current socket
	Destroy();

	// Uninitialize socket library
	UninitSockets();
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
		if ( m_lInit == 0 )
			return 1;

		WSADATA wd;

		// Attempt to initialize the Socket library
		m_lInit = WSAStartup( c_MinSocketVersion, &wd );

	} // end if

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

#if !defined( HTM_NOSOCKET2 )
	// Clean up socket lib
	WSACleanup();
#endif
}

long CIpSocket::GetInitCount()
{
	return 0;
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
	// Punt if not initialized
	if ( !IsInitialized() )
		return;

	// Let everyone know we're closing
	if ( c_InvalidSocket != m_hSocket )
		OnClose();

	// Ditch the event handle
	CloseEventHandle();

	// Save socket pointer to socket
	SOCKET hSocket = m_bFree ? (SOCKET)m_hSocket : INVALID_SOCKET;

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

	m_uReads = 0;
	m_uWrites = 0;
	m_uAccepts = 0;
	m_uFlags = 0;

	m_bFree = 1;

	// Reset activity timeout
	m_lActivity = 0;

	// Ensure valid socket handle
	if ( INVALID_SOCKET == hSocket )
		return;

	if ( IsInitialized() )
	{
		struct linger lopt;
		lopt.l_onoff = 0; lopt.l_linger = 0;
		if ( SOCKET_ERROR == setsockopt( hSocket, SOL_SOCKET, SO_LINGER, (const char*)&lopt, sizeof( lopt ) ) )
			m_uLastError = WSAGetLastError();

		// Close the socket
		if ( SOCKET_ERROR == closesocket( hSocket ) )
			m_uLastError = WSAGetLastError();

	} // end if
}

int CIpSocket::Shutdown()
{
	if ( c_InvalidSocket == m_hSocket )
		return 0;

	// Shut down the socket
	shutdown( (SOCKET)m_hSocket, SD_SEND );

	return 1;
}


int CIpSocket::Create( int x_af, int x_type, int x_protocol, int x_timeout )
{
	// Punt if not initialized
	if ( !IsInitialized() )
		return 0;

	// Close any open socket
	Destroy();

	// Clear errors
	m_uLastError = 0;

	// Create a scocket
	m_hSocket = (t_SOCKET)socket( (int)x_af, (int)x_type, (int)x_protocol );

	// Save the last error code
	m_uLastError = WSAGetLastError();
	if ( WSAEWOULDBLOCK == m_uLastError )
		m_uLastError = 0;

	// Was there an error?
	if ( c_InvalidSocket == m_hSocket )
	{	m_uConnectState |= eCsError;
		return 0;
	} // end if

	// Set default timeout
	if ( 0 >= x_timeout )
		x_timeout = m_lTimeout;

	// Setup socket timeout defaults
	struct timeval tv;
	tv.tv_sec = ( x_timeout / 1000 );
	tv.tv_usec = ( x_timeout % 1000 ) * 1000;
	setsockopt( (SOCKET)m_hSocket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof( tv ) );
	setsockopt( (SOCKET)m_hSocket, SOL_SOCKET, SO_SNDTIMEO, (const char *)&tv, sizeof( tv ) );

	// Process socket creation
	if ( !OnAttach() )
	{	Destroy();
		return 0;
	} // end if

	// Save socket connect params
	m_uSocketFamily = x_af;
	m_uSocketType = x_type;
	m_uSocketProtocol = x_protocol;

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
	{	Destroy();
		m_uConnectState |= eCsError;
		return 0;
	} // end if

	sockaddr_in sai;
	ZeroMemory( &sai, sizeof( sai ) );
	sai.sin_family = PF_INET;
	sai.sin_port = htons( (WORD)x_uPort );

	// Attempt to bind the socket
	int nRet = bind( (SOCKET)m_hSocket, (sockaddr*)&sai, sizeof( sockaddr_in ) );

	// Save the last error code
	m_uLastError = WSAGetLastError();
	if ( WSAEWOULDBLOCK == m_uLastError )
		m_uLastError = 0, nRet = 0;

	// Grab the address
	CIpSocket_GetAddressInfo( &m_addrLocal, &sai );

	if ( nRet )
	{	m_uConnectState |= eCsError;
		return 0;
	} // end if

	return 1;
}

int CIpSocket::Listen( unsigned int x_uMaxConnections )
{
	// Punt if not initialized
	if ( !IsInitialized() )
		return 0;

	// Must have socket
	if ( !IsSocket() )
	{	m_uConnectState |= eCsError;
		return 0;
	} // end if

	// Valid number of connections?
	if ( x_uMaxConnections == 0 )
		x_uMaxConnections = 16;
//		x_uMaxConnections = SOMAXCONN;

	// Start the socket listening
	int nRet = listen( (SOCKET)m_hSocket, (int)( x_uMaxConnections ? x_uMaxConnections : SOMAXCONN ) );

	// Save the last error code
	m_uLastError = WSAGetLastError();
	if ( WSAEWOULDBLOCK == m_uLastError )
		m_uLastError = 0, nRet = 0;

	// Error?
	if ( c_SocketError == (t_SOCKET)nRet )
	{	m_uConnectState |= eCsError;
		return 0;
	} // end if

	// We're trying to connect
	m_lActivity++;
	m_uConnectState |= eCsConnecting;

	// Return the result
	return 1;
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

	SOCKADDR_IN si;
	memset( &si, 0, sizeof( si ) );
	si.sin_family = m_uSocketFamily;
	CIpSocket_SetAddressInfo( &m_addrPeer, &si );

	// Attempt to connect
	int nRet = connect( (SOCKET)m_hSocket, (PSOCKADDR)&si, sizeof( si ) );

	// Save the last error code
	m_uLastError = WSAGetLastError();
	if ( WSAEWOULDBLOCK == m_uLastError )
		m_uLastError = 0, nRet = 0;

	// Check for error
	if ( nRet )
	{	m_uConnectState |= eCsError;
		return 0;
	} // end if

	// We're trying to connect
	m_lActivity++;
	m_uConnectState |= eCsConnecting;

	// Return the result
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
		m_hSocket = c_InvalidSocket;

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
	SOCKADDR saAddr; int iAddr = sizeof( saAddr );

	// Accept and encapsulate the socket
	BOOL bSuccess = x_is.Attach( (t_SOCKET)accept( (SOCKET)m_hSocket, &saAddr, &iAddr ) );

	// Check for error
	if ( !bSuccess )
	{   m_uLastError = WSAGetLastError();
		return 0;
	} // end if

	// Grab the address
	CIpSocket_GetAddressInfo( &m_addrPeer, &saAddr );

	// Capture all events
	x_is.EventSelect();

	m_uAccepts++;
	m_lActivity++;

	// Child is connecting
	x_is.m_lActivity++;
	x_is.m_uConnectState |= eCsConnecting;

	return 1;
}

int CIpSocket::CreateEventHandle()
{
#if defined( HTM_NOSOCKET2 )
	return 0;
#else
	// Check for event handle
	if ( IsEventHandle() )
		return 1;

	// Create socket event
	m_hSocketEvent = WSACreateEvent();

	// How did it turn out?
	return ( c_InvalidEvent != m_hSocketEvent ) ? 1 : 0;
#endif
}

void CIpSocket::CloseEventHandle()
{
#if defined( HTM_NOSOCKET2 )
	return;
#else

	// Do we have a valid handle?
	if ( c_InvalidEvent != m_hSocketEvent )
	{
		// Blocking socket
		if ( c_InvalidSocket != m_hSocket )
		{
			WSAEventSelect( (SOCKET)m_hSocket, (WSAEVENT)m_hSocketEvent, 0 );
			WSAEventSelect( (SOCKET)m_hSocket, (WSAEVENT)0, 0 );
			unsigned long l = 0; ioctlsocket( (SOCKET)m_hSocket, FIONBIO, &l );

			// Restore socket timeout defaults
			struct timeval tv;
			tv.tv_sec = ( m_lTimeout / 1000 );
			tv.tv_usec = ( m_lTimeout % 1000 ) * 1000;
			setsockopt( (SOCKET)m_hSocket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof( tv ) );
			setsockopt( (SOCKET)m_hSocket, SOL_SOCKET, SO_SNDTIMEO, (const char *)&tv, sizeof( tv ) );

		} // end if

		// Close the event handle
		WSACloseEvent( m_hSocketEvent );

		// Invalid handle value
		m_hSocketEvent = c_InvalidEvent;

	} // end if
#endif
}

int CIpSocket::EventSelect( long x_lEvents )
{
#if defined( HTM_NOSOCKET2 )
	return 0;
#else
	// Punt if no socket
	if ( !IsSocket() )
		return 0;

	// Must have event handle
	if ( !IsEventHandle() )
		CreateEventHandle();

	return ( WSAEventSelect( (SOCKET)m_hSocket, (WSAEVENT)m_hSocketEvent, x_lEvents ) == 0 );
#endif
}

long CIpSocket::WaitEvent( long x_lEventId, long x_uTimeout )
{
#if defined( HTM_NOSOCKET2 )
	return 0;
#else
	// Must have a socket handle
	if ( !IsSocket() )
		return 0;

	// Must have event handle
	if ( !IsEventHandle() )
	{   if ( !CreateEventHandle() || !EventSelect() )
			return 0;
	} // end if

	if ( 0 > x_uTimeout )
		x_uTimeout = m_lTimeout;

	// Save start time
	UINT uEnd = GetTickCount() + x_uTimeout;
	for( ; ; )
	{
		// What's the event state
		if ( 0 == ( m_uEventState & x_lEventId ) )
		{
			if ( x_uTimeout )
			{
				// Wait for event
//				UINT uRet =
					WaitForSingleObject( m_hSocketEvent, x_uTimeout );

				// Check for timeout or error
//		        if ( uRet != WAIT_OBJECT_0 )
//                   return 0;

			} // end if

			// Reset the network event
			ResetEvent( m_hSocketEvent );

			// Get network events
			WSANETWORKEVENTS wne; memset( &wne, 0, sizeof( wne ) );
			if ( SOCKET_ERROR == WSAEnumNetworkEvents( (SOCKET)m_hSocket, m_hSocketEvent, &wne ) )
			{	m_uLastError = WSAGetLastError();
				return 0;
			} // end if

			// Save the status of all events
			for ( unsigned int uMask = 1; uMask && uMask <= eAllEvents; uMask <<= 1 )
				if ( wne.lNetworkEvents & uMask )
				{
					// Get bit offset
					unsigned int uOffset = GetEventBit( uMask );

					// Save the event info
					m_uEventState |= uMask;
					m_uEventStatus[ uOffset ] = wne.iErrorCode[ uOffset ];

					// Signal activity
					m_uConnectState |= eCsActivity;

					// Signal when we get a connect message
					if ( 0 != ( FD_CONNECT & wne.lNetworkEvents ) )
					{
						// Set connected status
						if ( !m_uEventStatus[ FD_CONNECT_BIT ] )
							m_uConnectState |= eCsConnected;

						// Handle connect error
						else
							m_uLastError = m_uEventStatus[ FD_CONNECT_BIT ],
							m_uConnectState &= ~( eCsConnected | eCsActivity );

						// Connecting process is over
						m_uConnectState &= ~eCsConnecting;

					} // end if

					// Check for socket close
					if ( 0 != ( FD_CLOSE & wne.lNetworkEvents ) )
						m_uConnectState &= ~( eCsConnected | eCsActivity | eCsConnecting );

				} // end if

			// !!!  Kludge around missing connect message
			//      If we're accepting a connection, we don't
			//      get a connect message.  ( At least on Windows )
			if ( !( m_uConnectState & 2 ) && ( m_uConnectState & 1 ) )
			{	m_uConnectState |= eCsConnected;
				m_uConnectState &= ~eCsConnecting;
				wne.lNetworkEvents |= FD_CONNECT;
				m_uEventState |= FD_CONNECT;
				m_uEventStatus[ FD_CONNECT_BIT ] = 0;
			} // end if

		} // end if

		// Did our event go off?
		if ( 0 != ( m_uEventState & x_lEventId ) )
		{
			// Get the first event
			unsigned int uBit = GetEventBit( x_lEventId & m_uEventState );
			unsigned int uMask = 1 << uBit;

			// Acknowledge this event
			m_uEventState &= ~uMask;

			// Save the error code
			m_uLastError = m_uEventStatus[ uBit ];

			// Something is going on
			m_lActivity++;

			// We received the event
			return uMask;

		} // end if

		// Have we timed out?
		unsigned int uTick = GetTickCount();
		if ( uEnd <= uTick )
			return 0;

		// Adjust timeout
		x_uTimeout = uEnd - uTick;

	} // end if

	// Can't get here...
	return 0;
#endif
}

long CIpSocket::GetEventBit( long x_lEventMask )
{
#if defined( HTM_NOSOCKET2 )
	return 0;
#else
	// !!!  Events will be returned by WaitEvent() in the order
	//      they appear below.

	if ( 0 != ( FD_CONNECT & x_lEventMask ) )
		return FD_CONNECT_BIT;

	if ( 0 != ( FD_ACCEPT & x_lEventMask ) )
		return FD_ACCEPT_BIT;

	if ( 0 != ( FD_WRITE & x_lEventMask ) )
		return FD_WRITE_BIT;

	if ( 0 != ( FD_READ & x_lEventMask ) )
		return FD_READ_BIT;

	if ( 0 != ( FD_OOB & x_lEventMask ) )
		return FD_OOB_BIT;

	if ( 0 != ( FD_QOS & x_lEventMask ) )
		return FD_QOS_BIT;

	if ( 0 != ( FD_GROUP_QOS & x_lEventMask ) )
		return FD_GROUP_QOS_BIT;

	if ( 0 != ( FD_ROUTING_INTERFACE_CHANGE & x_lEventMask ) )
		return FD_ROUTING_INTERFACE_CHANGE_BIT;

	if ( 0 != ( FD_CLOSE & x_lEventMask ) )
		return FD_CLOSE_BIT;

	return 0;
#endif
}

CIpSocket::t_string CIpSocket::GetErrorMsg( unsigned long x_uErr )
{
	t_char *ptr = tcTT( t_char, "Unknown Winsock error" );

	switch( x_uErr )
	{
		case WSAEACCES:
			ptr = tcTT( t_char, "Access Denied" );
			break;
		case WSAEADDRINUSE:
			ptr = tcTT( t_char, "Address already in use" );
			break;
		case WSAEADDRNOTAVAIL:
			ptr = tcTT( t_char, "Cannot assign requested address" );
			break;
		case WSAEAFNOSUPPORT:
			ptr = tcTT( t_char, "Address family not supported by protocol family" );
			break;
		case WSAEALREADY:
			ptr = tcTT( t_char, "Operation already in progress" );
			break;
		case WSAECONNABORTED:
			ptr = tcTT( t_char, "Software caused connection abort" );
			break;
		case WSAECONNREFUSED:
			ptr = tcTT( t_char, "Connection refused" );
			break;
		case WSAECONNRESET:
			ptr =tcTT( t_char, "Connection reset by peer" );
			break;
		case WSAEDESTADDRREQ:
			ptr =tcTT( t_char, "Destination addres required" );
			break;
		case WSAEFAULT:
			ptr =tcTT( t_char, "Bad Address" );
			break;
		case WSAEHOSTDOWN:
			ptr =tcTT( t_char, "Host is down" );
			break;
		case WSAEHOSTUNREACH:
			ptr =tcTT( t_char, "Host is unreachable" );
			break;
		case WSAEINPROGRESS:
			ptr =tcTT( t_char, "Operation is now in progress" );
			break;
		case WSAEINTR:
			ptr =tcTT( t_char, "Interrupted function call" );
			break;
		case WSAEINVAL:
			ptr =tcTT( t_char, "Invalid argument" );
			break;
		case WSAEISCONN:
			ptr =tcTT( t_char, "Socket is already connected" );
			break;
		case WSAEMFILE:
			ptr =tcTT( t_char, "Too many open files" );
			break;
		case WSAEMSGSIZE:
			ptr =tcTT( t_char, "Message is too long" );
			break;
		case WSAENETDOWN:
			ptr =tcTT( t_char, "Network is down" );
			break;
		case WSAENETRESET:
			ptr =tcTT( t_char, "Network dropped connection on reset" );
			break;
		case WSAENETUNREACH:
			ptr =tcTT( t_char, "Network is unreachable" );
			break;
		case WSAENOBUFS:
			ptr =tcTT( t_char, "Insufficient buffer space is available" );
			break;
		case WSAENOPROTOOPT:
			ptr =tcTT( t_char, "Bad protocol option" );
			break;
		case WSAENOTCONN:
			ptr =tcTT( t_char, "Socket is not connected" );
			break;
		case WSAENOTSOCK:
			ptr =tcTT( t_char, "Socket operation on non-socket" );
			break;
		case WSAEOPNOTSUPP:
			ptr =tcTT( t_char, "Operation not supported" );
			break;
		case WSAEPFNOSUPPORT:
			ptr =tcTT( t_char, "Protocol family not supported" );
			break;
		case WSAEPROCLIM:
			ptr =tcTT( t_char, "Too many processes" );
			break;
		case WSAEPROTONOSUPPORT:
			ptr =tcTT( t_char, "Protocol not supported" );
			break;
		case WSAEPROTOTYPE:
			ptr =tcTT( t_char, "Protocol wrong type for socket" );
			break;
		case WSAESHUTDOWN:
			ptr =tcTT( t_char, "Cannot send after socket shutdown" );
			break;
		case WSAESOCKTNOSUPPORT:
			ptr =tcTT( t_char, "Socket type not supported" );
			break;
		case WSAETIMEDOUT:
			ptr =tcTT( t_char, "Connection timed out" );
			break;
		case WSATYPE_NOT_FOUND:
			ptr =tcTT( t_char, "Class type not found" );
			break;
		case WSAEWOULDBLOCK:
			ptr =tcTT( t_char, "Resource temporarily unavailable (Would block)" );
			break;
		case WSAHOST_NOT_FOUND:
			ptr =tcTT( t_char, "Host not found" );
			break;

		case WSANOTINITIALISED:
			ptr =tcTT( t_char, "Successful WSAStartup not yet performed" );
			break;
		case WSANO_DATA:
			ptr =tcTT( t_char, "Valid name, no data record of requested type" );
			break;
		case WSANO_RECOVERY:
			ptr =tcTT( t_char, "Non-recoverable error has occured" );
			break;
//		case WSAPROVIDERFAILEDINIT:
//			ptr =tcTT( t_char, "Unable to initialize a service provider" );
//			break;
		case WSASYSCALLFAILURE:
			ptr =tcTT( t_char, "System call failure" );
			break;
		case WSASYSNOTREADY:
			ptr =tcTT( t_char, "Network subsystem is unavailable" );
			break;
		case WSATRY_AGAIN:
			ptr =tcTT( t_char, "Non-authoritative host not found" );
			break;
		case WSAVERNOTSUPPORTED:
			ptr =tcTT( t_char, "WINSOCK.DLL version not supported" );
			break;
		case WSAEDISCON:
			ptr =tcTT( t_char, "Graceful shutdown in progress" );
			break;

#if !defined( HTM_NOSOCKET2 )
		case WSA_INVALID_HANDLE:
			ptr =tcTT( t_char, "Specified event object handle is invalid" );
			break;
		case WSA_INVALID_PARAMETER:
			ptr =tcTT( t_char, "One or mor parameters are invalid" );
			break;
//		case WSAINVALIDPROCTABLE;
//			ptr =tcTT( t_char, "Invalid procedure table from service provider" );
//			break;
//		case WSAINVALIDPROVIDER:
//			ptr =tcTT( t_char, "Invalid service provider version number" );
//			break;
		case WSA_IO_INCOMPLETE:
			ptr =tcTT( t_char, "Overlapped I/O event object not in signaled state" );
			break;
		case WSA_IO_PENDING:
			ptr =tcTT( t_char, "Overlapped I/O operations will complete later" );
			break;
		case WSA_NOT_ENOUGH_MEMORY:
			ptr =tcTT( t_char, "Insufficient memory available" );
			break;
		case WSA_OPERATION_ABORTED:
			ptr =tcTT( t_char, "Overlapped I/O operation has been aborted" );
			break;
#endif

	} // end switch

	return ptr;
}

int CIpSocket::v_recvfrom( int socket, void *buffer, int length, int flags )
{
	SOCKADDR_IN si;
	int nSize = sizeof( si );
	memset( &si, 0, sizeof( si ) );
	si.sin_family = m_uSocketFamily;

	// Receive data from socket
	int res = recvfrom( socket, (char*)buffer, length, flags, (LPSOCKADDR)&si, &nSize );

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
	int nRet = v_recvfrom( (SOCKET)m_hSocket, x_pData, (int)x_uSize, (int)x_uFlags );

	// Grab the last error code
	m_uLastError = WSAGetLastError();
	if ( WSAEWOULDBLOCK == m_uLastError )
		m_uLastError = 0;

	m_uReads++;
	m_lActivity++;

	// Check for closed socket
	if ( !nRet )
	{	m_uConnectState |= eCsError;
		return 0;
	} // end if

	// Check for socket error
	if ( SOCKET_ERROR == nRet || x_uSize < (unsigned int)nRet  || 0 > nRet )
	{
		// Is the socket blocking?
		if ( !getWouldBlock() && m_uLastError )
		{	m_uConnectState |= eCsError;
			return 0;
		} // end if

		// Nothing read
		if ( x_puRead )
			*x_puRead = 0;

		return 0;

	} // end if

	// Save the number of bytes read
	if ( x_puRead )
		*x_puRead = nRet;

	return nRet;
}

int CIpSocket::v_recv( int socket, void *buffer, int length, int flags )
{
	return recv( socket, (char*)buffer, length, flags );
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
	int nRet = v_recv( (SOCKET)m_hSocket, x_pData, (int)x_uSize, (int)x_uFlags );

	// Grab the last error code
	m_uLastError = WSAGetLastError();
	if ( WSAEWOULDBLOCK == m_uLastError )
		m_uLastError = 0;

	m_uReads++;
	m_lActivity++;

	// Check for closed socket
	if ( !nRet )
	{	m_uConnectState |= eCsError;
		return 0;
	} // end if

	// Check for socket error
	if ( SOCKET_ERROR == nRet || x_uSize < (unsigned int)nRet  || 0 > nRet )
	{
		// Is the socket blocking?
		if ( !getWouldBlock() && m_uLastError )
		{	m_uConnectState |= eCsError;
			return 0;
		} // end if

		// Nothing read
		if ( x_puRead )
			*x_puRead = 0;

		return 0;

	} // end if

	// Save the number of bytes read
	if ( x_puRead )
		*x_puRead = nRet;

	return nRet;
}


int CIpSocket::v_sendto(int socket, const void *message, int length, int flags )
{
	// Use the peer address
	SOCKADDR_IN si;
	memset( &si, 0, sizeof( si ) );
	si.sin_family = m_uSocketFamily;
	CIpSocket_SetAddressInfo( &m_addrPeer, &si );

	// Send the data
	return sendto( socket, (const char*)message, length, flags, (LPSOCKADDR)&si, sizeof( si ) );
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
	int nRet = v_sendto( (SOCKET)m_hSocket, x_pData, (int)x_uSize, (int)x_uFlags );

	// Get the last error code
	m_uLastError = WSAGetLastError();
	if ( WSAEWOULDBLOCK == m_uLastError )
		m_uLastError = 0;

	m_uWrites++;
	m_lActivity++;

	// Check for error
	if ( SOCKET_ERROR == nRet )
	{
		// Is the socket blocking?
		if ( !getWouldBlock() && m_uLastError )
		{	m_uConnectState |= eCsError;
			return 0;
		} // end if

		// Number of bytes sent
		if ( x_puSent )
			*x_puSent = 0;

		// Error
		return 0;

	} // end if

	// Save the number of bytes sent
	if ( x_puSent )
		*x_puSent = nRet;

	return nRet;
}

int CIpSocket::v_send( int socket, const void *buffer, int length, int flags )
{
	return send( socket, (const char*)buffer, length, flags );
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
	setWouldBlock( 0 );
	int nRet = v_send( (SOCKET)m_hSocket, x_pData, (int)x_uSize, (int)x_uFlags );

	// Get the last error code
	m_uLastError = WSAGetLastError();
	if ( WSAEWOULDBLOCK == m_uLastError )
		m_uLastError = 0;

	m_uWrites++;
	m_lActivity++;

	// Check for error
	if ( SOCKET_ERROR == nRet || 0 > nRet )
	{
		// Is the socket blocking?
		if ( !getWouldBlock() && m_uLastError )
		{	m_uConnectState |= eCsError;
			return 0;
		} // end if

		// Not an error
		m_uConnectState &= ~eCsError;

		// Number of bytes sent
		if ( x_puSent )
			*x_puSent = 0;

		// no bytes sent
		return 0;

	} // end if

	// Save the number of bytes sent
	if ( x_puSent )
		*x_puSent = nRet;

	return nRet;
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
	SOCKADDR_IN sai;
	memset( &sai, 0, sizeof( sai ) );
	int len = sizeof( sai );

	// Get the socket info
	if ( getpeername( (SOCKET)x_hSocket, (sockaddr*)&sai, &len ) )
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
	SOCKADDR_IN sai;
	memset( &sai, 0, sizeof( sai ) );
	int len = sizeof( sai );

	// Get the socket info
	if ( getsockname( (SOCKET)x_hSocket, (sockaddr*)&sai, &len ) )
		return 0;

	// Format the info
	return CIpSocket_GetAddressInfo( x_pIa, &sai );
}

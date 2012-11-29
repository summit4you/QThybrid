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

//==================================================================
// CIpAddress
//
/// Socket address wrapper
/**
	@see CIpSocket
*/
//==================================================================
class CIpAddress
{
public:

    enum
    {
        // IPV4 address type
        eAddrTypeIpv4           = 0,

        // IPV6 address type
        eAddrTypeIpv6           = 1

    };

	/// String type
	typedef str::t_string8 t_string;

	/// Character type
	typedef t_string::value_type t_char;

	/// 64 bit integer
	typedef str::tc_int64 t_int64;

public:

    /// Default constructor
    CIpAddress()
    {   m_uType = 0; m_uCrc = 0; m_uPort = 0; m_llIpv6 = 0;
    }

    /// Copy constructor
    CIpAddress( const CIpAddress &r )
    {   Copy( r );
    }

    /// Copy constructor
    CIpAddress( const t_string &r )
    {   m_uType = 0; m_uCrc = 0; m_uPort = 0; m_llIpv6 = 0; LookupUri( r );
    }

    /// Resets address information
    void Destroy()
    {   m_uType = 0; m_uCrc = 0; m_uPort = 0; m_llIpv6 = 0; 
    }

    /// Returns the UID
    uid::UID& getUid() { return m_uid; }

    /// Sets the address from an ID
    CIpAddress& setUid( const uid::UID *x_pUid );

	/// Returns the current host name
	static t_string GetHostName();

	/// Returns the current host name
	static t_string GetFullHostName();

	/// Returns the current computers domain name
	static t_string GetDomainName( const t_string &x_sServer = "" );

	/// Gets address by host name
	int GetHostByName( const t_string &x_sHost )
	{	return LookupHost( x_sHost, 0 ); }

    /// Returns the dot address
    t_string GetDotAddress();

    /// Fills in the address using dot address
    int SetDotAddress( const t_string &x_sDotAddress, unsigned int x_uPort, unsigned int x_uType = eAddrTypeIpv4 );

    /// Sets the raw address values
    int SetRawAddress( t_int64 x_llIp, unsigned int x_uPort, unsigned int x_uType = eAddrTypeIpv4 );

    /// Validates the address check sum
    int ValidateAddress() const;

    //==============================================================
    // LookupUri()
    //==============================================================
    /// Gets uri server address information from DNS server
    /**
	    \param [in] x_sUrl	-	Url to lookup
        \param [in] x_uPort -   Optional port to set
		\param [in] x_uType		- Network type

	    \return Returns non-zero if success.
    */
    int LookupUri( const t_string &x_sUrl, unsigned int x_uPort = 0, unsigned int x_uType = eAddrTypeIpv4 );

    //==============================================================
    // LookupHost()
    //==============================================================
    /// Gets host address information from DNS server
    /**
	    \param [in] x_sServer	- Host address to lookup
        \param [in] x_uPort     - Optional port to set
		\param [in] x_uType		- Network type

	    \return Returns non-zero if success.
    */
    int LookupHost( const t_string &x_sServer, unsigned int x_uPort, unsigned int x_uType = eAddrTypeIpv4 );

    /// Returns the binary ip address value
    t_int64 getIpv6()
    {   return m_llIpv6; }

    /// Returns the lo part of the ip address
    int getIpv4()
    {   return m_uIpv4; }

    /// Returns the port number
    int getPort()
    {   return m_uPort; }

    /// Returns the port number
    int getType()
    {   return m_uType; }

    /// Returns the address check sum
    int getCrc()
    {   return m_uCrc; }

    /// uid structure
    operator uid::UID*()
    {   return &m_uid; }

    /// Copy another address
    CIpAddress& Copy( const CIpAddress &rIa )
    {   uid::Copy( &m_uid, &rIa.m_uid );
		return *this;
    }

    /// Copy operator
    CIpAddress& operator = ( const CIpAddress &rIa )
    {   return Copy( rIa ); }

    /// Compare addresses
    int Cmp( CIpAddress &rIa )
    {   return uid::Cmp( &m_uid, &rIa.m_uid ); }

    /// EQ operator
    int operator == ( CIpAddress &rIa )
    {   return Cmp( rIa ); }

    /// NEQ operator
    int operator != ( CIpAddress &rIa )
    {   return !Cmp( rIa ); }

private:

// No padding
#ifndef HTM_NOPACK
#	pragma pack( push, 1 )
#endif

    union
    {
        struct
        {
            /// Connection Port
	        unsigned int		    m_uPort;

	        /// CRC
	        unsigned short		    m_uCrc;

	        /// Address type
	        unsigned short		    m_uType;

            union
            {
    	        /// IPV6 Address
	            t_int64			    m_llIpv6;

                struct
                {
                    /// Filler
                    unsigned int	m_uIpv4Extra;

                    /// IPV4 address
                    unsigned int	m_uIpv4;

                };
            };
        };

        /// Unique id
        uid::UID m_uid;
    };

#ifndef HTM_NOPACK
#	pragma pack( pop )
#endif

};

//==================================================================
// CIpSocket
//
/// Socket API wrapper class
/**
	Provides wrapper for os specific socket support

	Example:
	\code

    if ( !CIpSocket::InitSockets() )
        return -1;

    CIpSocket is;

    if ( !is.Connect( "google.com", 80 )
         || !is.WaitEvent( CIpSocket::eConnectEvent ) )
    {   TRACE( "CIpSocket::Connect() : %s\n", is.GetLastErrorMsg().c_str() );
        return -2;
    } // end if

    if ( !is.Send( "GET / HTTP/1.0\r\n\r\n" ) )
    {   TRACE( "CIpSocket::Send() : %s\n", is.GetLastErrorMsg().c_str() );
        return -3;
    } // end if

	// Wait for data
    if ( is.WaitEvent( CIpSocket::eReadEvent ) )
        TRACE( "%s\n", is.Recv().c_str() );

    TRACE( "Peer: %s\n", is.PeerAddress().GetDotAddress().c_str() );
    TRACE( "Local: %s\n", is.LocalAddress().GetDotAddress().c_str() );

    CIpSocket::UninitSockets();

	\endcode

	\see CIpAddress
*/
//==================================================================
class CIpSocket
{
public:

	/// Address Family
	enum
	{
		/// Unspecified
		eAfUnspec		= 0,

		/// Unix sockets
		eAfUnix			= 1,

		/// Internet IP sockets
		eAfInet			= 2
	};

	/// Socket Type
	enum
	{
		/// TCP / Stream
		eTypeStream		= 1,

		/// UDP / Datagram
		eTypeDgram		= 2,

        /// Raw interface
        eTypeRaw        = 3,

        /// Reliably delivered message
        eTypeRdm        = 4,

        /// Sequenced packet stream
        eTypeSeqPacket  = 5

	};

    /// Protocol type
    enum
    {
        /// IP
        eProtoIp                = 0,
        eProtoHopOpts           = 0,
        eProtoIcmp              = 1,
        eProtoIgmp              = 2,
        eProtoGgp               = 3,
        eProtoIpv4              = 4,
        eProtoTcp               = 6,
        eProtoPup               = 12,
        eProtoUdp               = 17,
        eProtoIdp               = 22,
        eProtoIpv6              = 41,
        eProtoRouting           = 43,
        eProtoFragment          = 44,
        eProtoEsp               = 50,
        eProtoAh                = 51,
        eProtoIcmpv6            = 58,
        eProtoIpv6None          = 59,
        eProtoIpv6DstOpts       = 60,
        eProtoNetDisk           = 61,

        eProtoRaw               = 255,
        eProtoMax               = 256

    };

	/// Socket 2 values
	enum
	{
		eReadBit						= 0,
		eReadEvent						= ( 1 << eReadBit ),

		eWriteBit						= 1,
		eWriteEvent						= ( 1 << eWriteBit ),

		eOobBit							= 2,
		eOobEvent						= ( 1 << eOobBit ),

		eAcceptBit						= 3,
		eAcceptEvent					= ( 1 << eAcceptBit ),

		eConnectBit						= 4,
		eConnectEvent					= ( 1 << eConnectBit ),

		eCloseBit						= 5,
		eCloseEvent						= ( 1 << eCloseBit ),

		eQosBit							= 6,
		eQosEvent						= ( 1 << eQosBit ),

		eGroupQosBit					= 7,
		eGroupQosEvent					= ( 1 << eGroupQosBit ),

		eRoutingInterfaceChangeBit		= 8,
		eRoutingInterfaceChangeEvent	= ( 1 << eRoutingInterfaceChangeBit ),

		eAddressListChangeBit			= 9,
		eAddressListChangeEvent			= ( 1 << eAddressListChangeBit ),

		eNumEvents						= 10,
		eAllEvents						= ( ( 1 << eNumEvents ) - 1 )
	};

	enum
	{
		/// Maximum number of events handled on a single call to
		/// epoll_wait() (linux only)
		eMaxEvents						= 64
	};

	enum
	{
		/// Activity
		eCsActivity						= 0x00000001,

		/// Socket has received connected signal
		eCsConnected					= 0x00000002,

		/// Socket is connecting
		eCsConnecting					= 0x00000004,

		/// Error on socket
		eCsError						= 0x00010000
	};

	enum
	{
		/// Operation would have blocked
		eFlagWouldBlock					= 0x00000001
	};

    /// Socket handle type
    typedef void* t_SOCKET;

    /// Socket event type
    typedef void* t_SOCKETEVENT;

	/// Invalid socket value
	static const t_SOCKET c_InvalidSocket;

	/// Failure value
	static const t_SOCKET c_SocketError;

	/// Invalid socket event value
	static const t_SOCKETEVENT c_InvalidEvent;

	/// String type
	typedef str::t_string8 t_string;

	/// Character type
	typedef t_string::value_type t_char;

	/// 64 bit integer
	typedef str::tc_int64 t_int64;

protected:

	/// Constructs the class
	virtual void Construct();

public:

	/// Default Constructor
	CIpSocket();

	/// Construct from socket handle
	CIpSocket( t_SOCKET hSocket, int x_bFree = 1 );

	/// Destructor
	virtual ~CIpSocket();

public:

	//==============================================================
	// Destroy()
	//==============================================================
	/// Closes the socket and releases related resources
	void Destroy();

	//==============================================================
	// OnClose()
	//==============================================================
	/// Called when closing a socket
	/**
		\return Returns non-zero if success
	*/
	virtual int OnClose() { return 1; }

	//==============================================================
	// InitSockets()
	//==============================================================
	/// Initializes the Socket API
	/**
		\warning	You must call this function before any other
					socket functions!  Best if called from your
					application startup code.  Call UninitSockets()
					before your application shuts down and after
					all instances of this class have been closed.

		\return Returns non-zero if success.
	*/
	static int InitSockets();

	//==============================================================
	// UninitSockets()
	//==============================================================
	/// Uninitializes the Windows Socket API
	static void UninitSockets();

	//==============================================================
	// GetInitCount()
	//==============================================================
	/// Returns the number of outstanding calls to InitSockets()
	static long GetInitCount();

	//==============================================================
	// IsInitialized()
	//==============================================================
	/// Returns non-zero if the Socket API was successfully initialized.
	static int IsInitialized();

public:

	/// Override to provide custom read
	virtual int v_recv( int socket, void *buffer, int length, int flags );
	virtual int v_recvfrom( int socket, void *buffer, int length, int flags );

	/// Override to provide custom write
	virtual int v_send( int socket, const void *buffer, int length, int flags );
	virtual int v_sendto(int socket, const void *message, int length, int flags );

	/// Transport type
	virtual t_string v_get_transport_type() { return tcTT( t_char, "raw" ); }
	virtual t_string v_get_transport_name() { return tcTT( t_char, "socket" ); }
	virtual t_string v_get_transport_properties() { return t_string(); }

public:

	//==============================================================
	// Attach()
	//==============================================================
	/// Attaches to existing socket handle
	/**
		\param [in] x_hSocket		-	Existing socket handle
		\param [in] x_bFree			-	Non-zero if this class should free the socket

		\return Returns non-zero if success
	*/
	int Attach( t_SOCKET x_hSocket, int x_bFree = 1 );

	//==============================================================
	// OnAttach()
	//==============================================================
	/// Called when attaching to a new socket
	/**

		\return Returns non-zero if success
	*/
	virtual int OnAttach() { return 1; }

	//==============================================================
	// Detach()
	//==============================================================
	/// Detaches from existing socket handle without releasing it.
	t_SOCKET Detach();

	//==============================================================
	// IsSocket()
	//==============================================================
	/// Returns non-zero if the class contains a valid socket handle
	int IsSocket()
	{   return ( c_InvalidSocket != m_hSocket ); }

	//==============================================================
	// GetConnectState()
	//==============================================================
	/// Returns value representing connect status
	/**
			bit 1	-	non zero if activity
			bit 2	-	non zero if connected
			bit 3	-	non zero if connecting
	*/
	int GetConnectState()
	{	return m_uConnectState; }

	//==============================================================
	// IsConnected()
	//==============================================================
	/// Returns non-zero if th socket is connected
	int IsConnected()
	{
		// If errors, we're closed
		if ( m_uLastError )
			return 0;

		// Do we think we're connected?
		if ( m_uConnectState & ( eCsActivity | eCsConnected ) )

			// Check for close event
			return ( WaitEvent( eCloseEvent, 0 ) || m_uLastError ) ? 0 : 1;

		// See if we're just now connecting
		if ( !WaitEvent( eConnectEvent, 0 ) )
			return 0;

		return m_uLastError ? 0 : 1;
	}

	//==============================================================
	// IsConnecting()
	//==============================================================
	/// Returns non-zero if the socket is in the process of connecting
	int IsConnecting()
	{	WaitEvent( eConnectEvent, 0 );
		return ( m_uConnectState & eCsConnecting ) ? 1 : 0;
	}

	//==============================================================
	// IsError()
	//==============================================================
	/// Returns
	int IsError()
	{	return ( m_uConnectState & eCsError ) ? 1 : 0;
	}

	//==============================================================
	// GetActivity()
	//==============================================================
	/// Returns
	long GetActivity()
	{	return m_lActivity;
	}

	//==============================================================
	// IsRunning()
	//==============================================================
	/// Returns non-zero if the socket is alive
	int IsRunning()
	{
		// Is there a socket?
		if ( !IsSocket() )
			return 0;

		// Has it erred out?
		if ( IsError() )
			return 0;

		// Ensure it's connected or connecting
		if ( !IsConnected() && !IsConnecting() )
			return 0;

		return 1;
	}

	//==============================================================
	// GetSocketHandle()
	//==============================================================
	/// Returns a handle to the socket
	t_SOCKET GetSocketHandle()
	{	return m_hSocket; }

	//==============================================================
	// getWouldBlock()
	//==============================================================
	/// Returns non-zero if the socket is in the process of connecting
	int getWouldBlock()
	{	return ( m_uFlags & eFlagWouldBlock ) ? 1 : 0;
	}

	//==============================================================
	// setWouldBlock()
	//==============================================================
	/// Returns non-zero if the socket is in the process of connecting
	void setWouldBlock( int b )
	{	m_uFlags = b ? ( m_uFlags | eFlagWouldBlock ) : ( m_uFlags & ~eFlagWouldBlock );
	}

	//==============================================================
	// Create()
	//==============================================================
	/// Creates a new socket handle.
	/**
		\param [in] x_af		-	Address family specification.
		\param [in] x_type		-	The type specification.
		\param [in] x_protocol	-	The protocol to be used with the socket.
		\param [in] x_timeout	-	Default timeout in micro-seconds

		\return Returns non-zero if success.

		\see Bind(), Listen(), Connect()
	*/
	virtual int Create( int x_af = eAfInet, int x_type = eTypeStream, int x_protocol = 0, int x_timeout = 60000000 );

	//==============================================================
	// GetLastError()
	//==============================================================
	/// Returns the most recent error code
	unsigned long GetLastError()
    {   return m_uLastError; }

	//==============================================================
	// GetErrorMsg()
	//==============================================================
	/// Returns the human readable error description
    t_string GetErrorMsg( unsigned long x_uErr );

	//==============================================================
	// GetErrorMsg()
	//==============================================================
	/// Returns a human readable description of the last error
    t_string GetLastErrorMsg()
    {   return GetErrorMsg( GetLastError() ); }

	//==============================================================
	// Accept()
	//==============================================================
	/// Accepts an incomming connection on the specified socket
    /*
        \param [in] x_is    -   Socket that accepts the incomming
                                connection.
    */
    int Accept( CIpSocket &x_is );

	//==============================================================
	// GetPeerInfo()
	//==============================================================
	/// Returns address information about the connected peer
    /*
        \param [in] x_hSocket   -   Socket handle
        \param [in] x_pIa        -   Receives address information.
    */
    static int GetPeerAddress( t_SOCKET x_hSocket, CIpAddress *x_pIa );

	//==============================================================
	// GetLocalAddress()
	//==============================================================
	/// Returns address information about the local connection
    /*
        \param [in] x_hSocket   -   Socket handle
        \param [in] x_pIa        -   Receives address information.
    */
    static int GetLocalAddress( t_SOCKET x_hSocket, CIpAddress *x_pIa );

    /// Returns peer address information
    CIpAddress& PeerAddress()
    {   if ( !m_addrPeer.getIpv4() )
            GetPeerAddress( m_hSocket, &m_addrPeer );
        return m_addrPeer;
    }

    /// Returns local address information
    CIpAddress& LocalAddress()
    {   if ( !m_addrLocal.getIpv4() )
            GetLocalAddress( m_hSocket, &m_addrLocal );
        return m_addrLocal;
    }

public:

	//==============================================================
	// Bind()
	//==============================================================
	/// Binds the open socket to the specified Port
	/**
		\param [in] x_uPort	-	Port to bind to.

		\return Returns non-zero if success.

		\see Create(), Listen(), Connect()
	*/
	int Bind( unsigned int x_uPort );

	//==============================================================
	// Listen()
	//==============================================================
	/// Creates a socket listening on the bound port.
	/**
		\param [in] uMaxConnections	-	The maximum number of connections allowed.

		\return Returns non-zero if success, otherwise zero.

		\see Create(), Bind(), Connect()
	*/
	int Listen( unsigned int x_uMaxConnections = 0 );

	//==============================================================
	// Connect()
	//==============================================================
	/// Address of remote peer.
	/**
		\param [in] x_sAddress	-	URL formed address of remote peer.
		\param [in] x_uPort		-	Port of remote peer.

		\return Returns non-zero if success.
	*/
	int Connect( const t_string &x_sAddress, unsigned int x_uPort = 0 );

	//==============================================================
	// Connect()
	//==============================================================
	/// Address of remote peer.
	/**
		\param [in] x_rIpAddress -	Address of remote peer

		\return Returns non-zero if success.
	*/
	int Connect( const CIpAddress &x_rIpAddress );

public:

	//==============================================================
	// EventSelect()
	//==============================================================
	/// Selects which events will generate callbacks
	/**
		\param [in] x_lEvents	-	The events to hook.

		\return Returns non-zero if success.
	*/
	int EventSelect( long x_lEvents =	eReadEvent | eWriteEvent
						    			| eAcceptEvent | eConnectEvent
							    		| eCloseEvent );

	//==============================================================
	// GetEventHandle()
	//==============================================================
	/// Retuns the current event handle
	t_SOCKETEVENT GetEventHandle()
    {   return m_hSocketEvent; }

	//==============================================================
	// IsEventHandle()
	//==============================================================
	/// Returns non-zero if there is a valid event handle
    int IsEventHandle()
    {   return vInvalidEvent() != GetEventHandle() ? 1 : 0; }

	//==============================================================
	// CreateEventHandle()
	//==============================================================
	/// Creates a network event handle
	int CreateEventHandle();

	//==============================================================
	// CloseEventHandle()
	//==============================================================
	/// Closes the event handle
	void CloseEventHandle();

	//==============================================================
	// WaitEvent()
	//==============================================================
	/// Waits for a socket event to occur
	/**
		\param [in] x_lEventId	-	Mask identifying event(s) to wait
									for.
		\param [in] x_lTimeout	-	Maximum time to wait in milli-
									seconds.  Negative for default
									timeout value.

		\return The mask of the event that triggered the return. Zero
                if timed out waiting for event.

		\see
	*/
	long WaitEvent( long x_lEventId = ~0, long x_lTimeout = -1 );

	//==============================================================
	// GetEventBit()
	//==============================================================
	/// Returns the bit offset for the specified event
	/**
		\param [in] x_lEventMask	-	Event mask

		\return Bit offset for specified event

		\see
	*/
	static long GetEventBit( long x_lEventMask );

public:

	//==============================================================
	// RecvFrom()
	//==============================================================
	/// Reads data from the socket
	/**
		\param [in] x_pData		-	Receives the socket data
		\param [in] x_uSize		-	Size of buffer in pData
		\param [in] x_puRead	-	Receives the number of bytes read
		\param [in] x_uFlags	-	Socket receive flags

		\return Number of bytes read or c_InvalidSocket if failure.

		\see
	*/
	unsigned long RecvFrom( void *x_pData, unsigned long x_uSize, unsigned long *x_puRead = 0, unsigned long x_uFlags = 0 );


	//==============================================================
	// RecvFrom()
	//==============================================================
	/// Reads data from the socket and returns a CStr object
	/**
		\param [in] x_uMax		-   Maximum amount of data to return
		\param [in] x_uFlags	-	Socket receive flags

		\return CStr containing data

		\see
	*/
    t_string RecvFrom( unsigned long x_uMax = 0, unsigned long x_uFlags = 0 );

	//==============================================================
	// Recv()
	//==============================================================
	/// Reads data from the socket
	/**
		\param [in] x_pData		-	Receives the socket data
		\param [in] x_uSize		-	Size of buffer in pData
		\param [in] x_puRead	-	Receives the number of bytes read
		\param [in] x_uFlags	-	Socket receive flags

		\return Number of bytes read or c_InvalidSocket if failure.

		\see
	*/
	unsigned long Recv( void *x_pData, unsigned long x_uSize, unsigned long *x_puRead = 0, unsigned long x_uFlags = 0 );


	//==============================================================
	// Recv()
	//==============================================================
	/// Reads data from the socket and returns a CStr object
	/**
		\param [in] x_uMax		-   Maximum amount of data to return
		\param [in] x_uFlags	-	Socket receive flags

		\return CStr containing data

		\see
	*/
    t_string Recv( unsigned long x_uMax = 0, unsigned long x_uFlags = 0 );

	//==============================================================
	// Read()
	//==============================================================
	/// Reads data from the socket and returns a CStr object
	/**
		\param [in] x_uMax		-   Maximum amount of data to return
		\param [in] x_uFlags	-	Socket receive flags

		\return CStr containing data

		\see
	*/
    t_string Read( unsigned long x_uMax = 0, unsigned long x_uFlags = 0 )
	{	return Recv( x_uMax, x_uFlags ); }

	//==============================================================
	// SendTo()
	//==============================================================
	/// Writes data to the socket
	/**
		\param [in] x_pData		-	Buffer containing write data
		\param [in] x_uSize		-	Size of the buffer in pData
		\param [in] x_puSent	-	Receives the number of bytes written
		\param [in] x_uFlags	-	Socket write flags

		\return Number of bytes sent or c_InvalidSocket if failure.

		\see
	*/
	unsigned long SendTo( const void *x_pData, unsigned long x_uSize, unsigned long *x_puSent = 0, unsigned long x_uFlags = 0 );

	//==============================================================
	// SendTo()
	//==============================================================
	/// Writes a string to the socket
	/**
		\param [in] x_sStr		-	String to be sent
		\param [in] x_uMax		-	Maximum number of bytes to send
		\param [in] x_puSent	-	Number of bytes sent
		\param [in] x_uFlags	-	Socket write flags

		\return Number of bytes sent or c_InvalidSocket if failure.

		\see
	*/
	unsigned long SendTo( const t_string &x_sStr, unsigned long x_uMax = 0, unsigned long *x_puSent = 0, unsigned long x_uFlags = 0 )
    {	if ( !x_uMax || x_uMax > x_sStr.length() ) x_uMax = x_sStr.length();
		return SendTo( (const void*)x_sStr.c_str(), x_uMax, x_puSent, x_uFlags );
	}

	//==============================================================
	// Send()
	//==============================================================
	/// Writes data to the socket
	/**
		\param [in] x_pData		-	Buffer containing write data
		\param [in] x_uSize		-	Size of the buffer in pData
		\param [in] x_puSent	-	Receives the number of bytes written
		\param [in] x_uFlags	-	Socket write flags

		\return Number of bytes sent or c_InvalidSocket if failure.

		\see
	*/
	unsigned long Send( const void *x_pData, unsigned long x_uSize, unsigned long *x_puSent = 0, unsigned long x_uFlags = 0 );

	//==============================================================
	// Send()
	//==============================================================
	/// Writes a string to the socket
	/**
		\param [in] x_sStr		-	String to be sent
		\param [in] x_puSent	-	Number of bytes sent
		\param [in] x_uFlags	-	Socket write flags

		\return Number of bytes sent or c_InvalidSocket if failure.

		\see
	*/
	unsigned long Send( const t_string &x_sStr, unsigned long *x_puSent = 0, unsigned long x_uFlags = 0 )
    {	return Send( (const void*)x_sStr.c_str(), x_sStr.length(), x_puSent, x_uFlags ); }

	//==============================================================
	// Write()
	//==============================================================
	/// Writes a string to the socket
	/**
		\param [in] x_sStr		-	String to be sent
		\param [in] x_puSent	-	Number of bytes sent
		\param [in] x_uFlags	-	Socket write flags

		\return Number of bytes sent or c_InvalidSocket if failure.

		\see
	*/
	unsigned long Write( const t_string &x_sStr, unsigned long *x_puSent = 0, unsigned long x_uFlags = 0 )
    {	return Send( (const void*)x_sStr.c_str(), x_sStr.length(), x_puSent, x_uFlags ); }

public:

	//==============================================================
	// GetPeerName()
	//==============================================================
	/// Gets the remote socket information
	/**
		\param [out] x_pName	-	Receives the remote address of the connected socket.
		\param [out] x_puPort	-	Receives the remote TCP port of the connected socket.

		\return Returns non-zero if success.
	*/
	int GetPeerName( t_string &x_sName, unsigned long *x_puPort = 0 ) { return 0; }

	//==============================================================
	// GetSocketName()
	//==============================================================
	/// Gets the local socket information
	/**
		\param [out] x_pName	-	Receives the local address of the connected socket.
		\param [out] x_pdwPort	-	Receives the local TCP port of the connected socket.

		\return Returns non-zero if success.
	*/
	int GetSocketName( t_string &x_pName, unsigned long *x_puPort = 0 ) { return 0; }

    /// Shuts down the socket
    int Shutdown();

	/// Byte conversions
	static unsigned int hton_l( unsigned int v );
	static unsigned int ntoh_l( unsigned int v );
	static unsigned short hton_s( unsigned short v );
	static unsigned short ntoh_s( unsigned short v );

public:

	/// Invalid socket handle value
    static const t_SOCKET vInvalidSocket()
    {   return c_InvalidSocket; }

    /// Invalid socket event value
    static const t_SOCKETEVENT vInvalidEvent()
    {   return c_InvalidEvent; }

    /// Invalid socket event value
    static const t_SOCKET vSocketError()
    {   return c_SocketError; }

	/// Converts windows event flags to unix event flags
	static unsigned long FlagWinToNix( unsigned long x_nFlag );

	/// Converts unix event flags to windows event flags
	static unsigned long FlagNixToWin( unsigned long x_nFlag );

	/// Returns the number of reads on the socket
	unsigned long getNumReads() { return m_uReads; }

	/// Returns the number of writes on the socket
	unsigned long getNumWrites() { return m_uWrites; }

	/// Returns the number of accepts on the socket
	unsigned long getNumAccepts() { return m_uAccepts; }

	/// Returns the default timeout in microseconds
	long getTimeout() { return m_lTimeout; }

	/// Sets the default timeout in microseconds
	void setTimeout( long ul ) { m_lTimeout = ul; }

private:

	/// Socket API initialization return code
	static long	m_lInit;

	/// Socket handle
	t_SOCKET				m_hSocket;

	/// Non-zero if this class should free the socket on destruction
	int						m_bFree;

	/// Last error code
	unsigned long			m_uLastError;

	/// Temporarily holds error value for GetErrorMsg()
    t_string				m_sError;

    /// String used to connect
    t_string                m_sConnect;

    /// Peer address structure
    CIpAddress              m_addrPeer;

    /// Peer address structure
    CIpAddress              m_addrLocal;

	/// Socket event handle
    t_SOCKETEVENT			m_hSocketEvent;

    /// Connect state
    unsigned long           m_uConnectState;

    /// Event state flags
    unsigned long           m_uEventState;

    /// Event status flags
    unsigned long           m_uEventStatus[ eNumEvents ];

	/// Pointer to event object (UNIX only)
	void 					*m_pEventObject;

    /// Socket family
    unsigned long           m_uSocketFamily;

    /// Socket type
    unsigned long           m_uSocketType;

    /// Socket protocol
    unsigned long           m_uSocketProtocol;

	/// Number of reads on the socket
	unsigned long			m_uReads;

	/// Number of writes on the socket
	unsigned long			m_uWrites;

	/// Number of accepts
	unsigned long			m_uAccepts;

	/// Activity counter
	unsigned long			m_lActivity;

	/// Non-zero if events have been hooked
	int						m_bEventsHooked;

	/// Flags
	unsigned long			m_uFlags;

	/// Default timeout in milliseconds
	long					m_lTimeout;
};



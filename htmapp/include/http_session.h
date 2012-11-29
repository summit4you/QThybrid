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
// CHttpSession
//
/// Provides basic HTTP functionality
/**

  @code


  @endcode

*/
//==================================================================

template < typename T_PORT > class THttpSession
{
public:

	enum
	{ 	HTTP_OK = 200,
		HTTP_CREATED = 201,
		HTTP_ACCEPTED = 202,
		HTTP_NON_AUTHORITATIVE_INFORMATION = 203,
		HTTP_NO_CONTENT = 204,
		HTTP_RESET_CONTENT = 205,
		HTTP_PARTIAL_CONTENT = 206,

		HTTP_MULTIPLE_CHOICES = 300,
		HTTP_MOVED_PERMANENTLY = 301,
		HTTP_FOUND = 302,
		HTTP_SEE_OTHER = 303,
		HTTP_NOT_MODIFIED = 304,
		HTTP_USE_PROXY = 305,
		HTTP_TEMPORARY_REDIRECT = 307,

		HTTP_BAD_REQUEST = 400,
		HTTP_AUTHORIZATION_REQUIRED = 401,
		HTTP_PAYMENT_REQUIRED = 402,
		HTTP_FORBIDDEN = 403,
		HTTP_NOT_FOUND = 404,
		HTTP_METHOD_NOT_ALLOWED = 405,
		HTTP_NOT_ACCEPTABLE = 406,
		HTTP_PROXY_AUTHENTICATION_REQUIRED = 407,
		HTTP_REQUEST_TIMEOUT = 408,
		HTTP_CONFLICT = 409,
		HTTP_GONE = 410,
		HTTP_LENGTH_REQUIRED = 411,
		HTTP_PRECONDITION_FAILED = 412,
		HTTP_REQUEST_ENTITY_TOO_LARGE = 413,
		HTTP_REQUEST_URI_TOO_LONG = 414,
		HTTP_UNSUPPORTED_MEDIA_TYPE = 415,
		HTTP_REQUESTED_RANGE_NOT_SATISFIABLE = 416,
		HTTP_EXPECTED_FAILURE = 417,

		HTTP_SERVER_ERROR = 500,
		HTTP_NOT_IMPLEMENTED = 501,
		HTTP_BAD_GATEWAY = 502,
		HTTP_SERVICE_UNAVAILABLE = 503,
		HTTP_GATEWAY_TIMEOUT = 504,
		HTTP_VERSION_NOT_SUPPORTED = 505,
	};

	enum
	{
		eAuthRequest = 0,

		eAuthMappedFolder = 1,
	};

	// +++ This also exists in THttpServer, should probably be in once place
	enum
	{
		// Don't allow clients to linger idle longer than one minute
		eDefaultKeepAlive = 60,

		// Don't process requests bigger than this
		eMaxRecvBytes = 1 * 1024 * 1024
	};

	/// String type
	typedef typename str::t_string8 t_string;

	/// Character type
	typedef typename t_string::value_type t_char;

	/// Server callback
	typedef int (*PFN_Callback)( void *x_pData, THttpSession< T_PORT > *x_pSession );

	/// Authentication callback
	typedef int (*PFN_Authenticate)( void *x_pData, THttpSession< T_PORT > *x_pSession, long lType, t_pb8 &sData );

	/// Server callback
	typedef int (*PFN_Close)( void *x_pData, THttpSession< T_PORT > *x_pSession );

	/// Buffer type
	typedef t_string						t_buffer;

	/// Byte type
	typedef typename t_buffer::value_type	t_byte;

public:

    /// Default constructor
    THttpSession()
    {
		m_pPort = 0;
        m_nErrorCode = HTTP_OK;
        m_nTransactions = 0;
        m_bHeaderReceived = 0;
#ifdef HTM_ENABLE_ZIP
		m_bEnableCompression = 1;
#else
		m_bEnableCompression = 0;
#endif
		m_fnCallback = 0;
		m_pData = 0;
		m_fnAuthenticate = 0;
		m_pAuthData = 0;
		m_fnClose = 0;
		m_pCloseData = 0;
		m_bNewSession = 1;
		m_uSessionTimeout = 60 * 60;
		m_ppbSession = 0;
		m_plockSession = 0;
		m_pMappedFolders = 0;
		m_pMappedFoldersLock = 0;
		m_uKeepAlive = 0;
		m_uMaxKeepAlive = eDefaultKeepAlive;
		m_uMaxRequestBytes = eMaxRecvBytes;
    }

    THttpSession( T_PORT *x_pPort )
    {
		m_pPort = x_pPort;
        m_nErrorCode = HTTP_OK;
        m_nTransactions = 0;
        m_bHeaderReceived = 0;
#ifdef HTM_ENABLE_ZIP
		m_bEnableCompression = 1;
#else
		m_bEnableCompression = 0;
#endif
		m_fnCallback = 0;
		m_pData = 0;
		m_fnAuthenticate = 0;
		m_pAuthData = 0;
		m_fnClose = 0;
		m_pCloseData = 0;
		m_bNewSession = 1;
		m_uSessionTimeout = 60 * 60;
		m_ppbSession = 0;
		m_plockSession = 0;
		m_pMappedFolders = 0;
		m_pMappedFoldersLock = 0;
		m_uMaxKeepAlive = eDefaultKeepAlive;
		m_uMaxRequestBytes = eMaxRecvBytes;
    }

    /// Destructor
    virtual ~THttpSession()
    {	m_pPort = 0;
		if ( m_fnClose )
			m_fnClose( m_pCloseData, this );
    }

	/// Initialize connection
	void Init()
	{
		// Initialize the keep alive timeout
		m_uKeepAlive = sys::get_gmt_timestamp() + m_uMaxKeepAlive;
	}

    void SetPort( T_PORT *x_pPort )
    {	m_pPort = x_pPort;
    }

    T_PORT* GetPort()
    {	return m_pPort;
    }

	//==============================================================
	// OnProcessRequest()
	//==============================================================
	/// Called when an HTTP request is made
    /**
        Over-ride this and provide custom HTTP processing
    */
    virtual int OnProcessRequest()
    {
    	// Call the callback function if provided
		if ( m_fnCallback )
			if ( !m_fnCallback( m_pData, this ) )
				return 1;

    	return 0;
    }

	//==============================================================
	// KeepAlive()
	//==============================================================
	/// Returns zero if connection should be terminated
    int KeepAlive()
	{	return (long)m_uKeepAlive > sys::get_gmt_timestamp();
	}

	//==============================================================
	// ResetConnection()
	//==============================================================
	/// Readys for a new transaction
	void Reset()
	{
		// Get ready for more requests
        m_nErrorCode = HTTP_OK;
		m_bHeaderReceived = 0;

		// Reset transaction data
		m_pb.clear();
		m_sFile.clear();
		m_sContent.clear();
	}

	//==============================================================
	// OnRead()
	//==============================================================
	/// Called when new data arrives
	/**
		\param [in] x_nErr	-	Error code

		\return Less than zero on failure

		\see
	*/
	int OnRead( int x_nErr )
	{
		if ( !m_pPort )
			return -1;

		// Check for data overflow
		if ( m_rx.length() >= m_uMaxRequestBytes )
			return -1;

		// Reset keep alive counter
		m_uKeepAlive = sys::get_gmt_timestamp() + m_uMaxKeepAlive;

		// Read in data if port is available
		unsigned long uMax = m_uMaxRequestBytes - m_rx.length();
		t_string sData = m_pPort->Read( uMax );
		
		// Wait for more data
		if ( !sData.length() )
			return 0;
		
		// Too much data
		if ( sData.length() >= uMax )
			return -1;

		// Add the data to the queue
		m_rx.append( sData );

		// Grab headers if needed
		if ( !m_bHeaderReceived )
		{
			int nErr = ReadHeaders();

			// Waiting for data?
			if ( !nErr )
				return 0;

			// Error?
			if ( HTTP_OK != nErr )
				return SendErrorMsg( nErr, "Bad HTTP request headers." ) ? 1 : -1;

		} // end if

		// Do we have all the data?
		long lContentLength = m_pb[ "RX_HEADERS" ][ "content-length" ].ToLong();
		if ( 0 < lContentLength && m_rx.length() < (unsigned long)lContentLength  )
			return 0;

		// Set default headers
		DefaultHeaders();

		// Are there any post variables?
		if ( !m_pb.isSet( "POST" ) && m_pb[ "REQUEST" ][ "REQUEST_METHOD" ].str() == "POST" )
		{
			// IE and Netscape append CRLF, 
			// so be sure and use the content length if available
			if ( lContentLength )
				m_pb[ "POST" ] = parser::DecodeUrl< t_pb8 >( t_string( m_rx, 0, lContentLength ) );

			else
				m_pb[ "POST" ] = parser::DecodeUrl< t_pb8 >( m_rx );

		} // end if

		// Dump whatever remains in the rx buffer
		// Some browsers I won't mention stick garbage in here
		m_rx.clear();

		// Process the request
		ProcessRequest();

		// Count a transaction
		m_nTransactions++;

		// Reset the connection
		Reset();

		return 1;
	}

	int ProcessRequest()
	{
		// Check for mapped foders first
		if ( !ProcessMappedFolders() )
		{
			// Call authentication function if any
			if ( m_fnAuthenticate )
				if ( 0 > m_fnAuthenticate( m_pAuthData, this, eAuthRequest, m_pb ) )
					return SendErrorMsg( HTTP_FORBIDDEN, "Access denied" );

			// Do the processing
			if ( OnProcessRequest() )
				SendReply();

			// No hablo
			else
				return SendErrorMsg( HTTP_NOT_FOUND, "File not found." );

		} // end if

		return 1;
	}

	int SendMappedFile( t_pb8 &pbFolder, const t_string &sPath )
	{
		// Search locations for the resource
		for ( t_pb8::iterator it = pbFolder.begin(); pbFolder.end() != it; it++ )
		{
			t_string sMapped = it->first;

			// Is it a resource?
			if ( '#' == sMapped[ 0 ] )
			{
				CHmResources res;

				// Are there any resources?
				if ( res.IsValid() )
				{
					// Drop the '#'
					sMapped.erase( 0, 1 );

					// Build full path
					sMapped = str::LTrim< t_string >( disk::WebPath( sMapped, sPath ), "/" );

					// Find a resource by this name
					HMRES hRes = res.FindResource( 0, sMapped.c_str() );

					// Process resource
					if ( hRes ) switch ( res.Type( hRes ) )
					{
						default :
							break;

						case 1 :
						{
							// Set MIME type
							SetContentType( sMapped );

							// Get data pointers
							const void *ptr = res.Ptr( hRes );
							unsigned long sz = res.Size( hRes );

							// Verify data
							if ( !ptr || 0 >= sz )
								SendErrorMsg( HTTP_SERVER_ERROR, "NULL file" );

							// Send binary data
							SendBinary( ptr, sz );

							return 1;

						} break;

						case 2 :
						{
							// Get function pointer
							CHmResources::t_fn pFn = res.Fn( hRes );
							if ( pFn )
							{
								t_pb8 out;

								// Execute the page
								pFn( m_pb, out );

								// Set MIME type
								SetContentMimeType( out[ "TX_HEADERS" ].isSet( "CONTENT_TYPE" )
													? out[ "TX_HEADERS" ][ "CONTENT_TYPE" ].str()
													: "text/html" );

								// Send the data
								SendBinary( out.data(), out.length() );

								return 1;

							} // end if

						} break;

					} // end switch

				} // end if

			} // end if

			else
			{
				// Build full file path
				t_string sFile = disk::FilePath( sMapped, sPath );

				// Attempt to send it if it exists
				if ( disk::exists( sFile.c_str() ) )
				{
					// Send the file
					if ( !SendFile( sFile ) )
						SendErrorMsg( HTTP_SERVER_ERROR, "File error" );

					return 1;

				} // end if

			} // end else

		} // end for

		return 0;
	}

	int ProcessMappedFolders()
	{
		// Mapped folders?
		if ( !m_pMappedFolders || !m_pMappedFoldersLock )
			return 0;

		CScopeLock ll( m_pMappedFoldersLock );
		if ( !ll.isLocked() )
			return 0;

		// What is being requested?
		t_string sPath = str::LTrim< t_string >( m_pb[ "REQUEST" ][ "uri" ][ "path" ].str(), "/" );
		if ( !sPath.length() && m_sDefaultPath.length() )
			sPath = m_sDefaultPath;

		// Get a name to try and match to a mapped folder
		t_string sName = disk::GetRoot( sPath );

		// Root?
		if ( !sName.length() )
			sName = "/";

		if ( !m_pMappedFolders->isSet( sName ) || !(*m_pMappedFolders)[ sName ].size() )
		{
			// Was it the root?
			if ( sName == "/" )
				return 0;

			// We must check the root folder if there is one
			sName = "/";
			if ( !m_pMappedFolders->isSet( sName ) || !(*m_pMappedFolders)[ sName ].size() )
				return 0;

		} // end if

		if ( m_fnAuthenticate )
			if ( 0 > m_fnAuthenticate( m_pAuthData, this, eAuthMappedFolder, m_pb ) )
			{	SendErrorMsg( HTTP_FORBIDDEN, "Access denied" );
				return 1;
			} // end if

		// See if there is a matching folder
		if ( SendMappedFile( (*m_pMappedFolders)[ sName ], sPath ) )
			return 1;

		return 0;
	}

	//==============================================================
	// OnClose()
	//==============================================================
	/// Called when socket connection has been closed or aborted.
	/**
		\param [in] nErr	-	Zero if no error, otherwise socket error value.

		\return Return non-zero if handled
	*/
	int OnClose( int x_nErr )
    {
        return 1;
    }

	t_string CreateCookie( t_string params )
	{	t_pb8 pb;
		pb[ m_sCookieId ] = params;
		return parser::EncodeUrl( pb ) += "; path=/";
	}

	t_string DecodeCookie( t_string cookie )
	{	std::list< t_string > items 
			= parser::Split< std::list< t_string > >( m_pb[ "RX_HEADERS" ][ "cookie" ].ToString(), "; " );
		for ( std::list< t_string >::iterator it = items.begin(); items.end() != it; it++ )
		{	t_pb8 data = parser::DecodeJson< t_pb8 >( *it );
			if ( data.isSet( m_sCookieId ) && data[ m_sCookieId ].str().length() )
				return data[ m_sCookieId ].str();
		} // end for
		return t_string();
	}

	void RestoreSession()
	{
		// Do we have session objects?
		if ( !m_ppbSession || !m_plockSession )
			return;

		// Lock the session data object
		CScopeLock ll( m_plockSession );
		if ( !ll.isLocked() )
			return;

		t_string id;

		// Is there a cookie?
		if ( m_pb[ "RX_HEADERS" ].isSet( "cookie" ) )
		{
			// Get our session id
			id = DecodeCookie( m_pb[ "RX_HEADERS" ][ "cookie" ].ToString() );

			// Attempt to recover our data
			if ( id.length() && m_ppbSession->isSet( id ) )
				m_pb[ "SESSION" ] = (*m_ppbSession)[ id ];

		} // end if

		// Get remote ip address
		t_string ip = m_pPort->PeerAddress().GetDotAddress();
		long ts = sys::get_gmt_timestamp();

		// Do we need to create a new session
		// Ensure session ip and port match current connection
		// This helps stop cookie spoofing
		if ( !id.length()
			 || !m_pb[ "SESSION" ].isSet( "_id" ) || m_pb[ "SESSION" ][ "_id" ].str() != id
			 || !m_pb[ "SESSION" ].isSet( "_ip" ) || m_pb[ "SESSION" ][ "_ip" ].str() != ip
			 || !m_pb[ "SESSION" ].isSet( "_ts" )
			 || ( m_pb[ "SESSION" ][ "_ts" ].ToLong() + (long)m_uSessionTimeout ) < ts
			 )
		{
			// Save new connection information
			m_pb[ "SESSION" ].clear();
			m_pb[ "SESSION" ][ "_id" ] = id.length() ? id : uid::ToStr< t_string >();
			m_pb[ "SESSION" ][ "_ip" ] = ip;

		} // end if

		// Existing session restored
		else
			m_bNewSession = 0;

		// Update timestamp
		m_pb[ "SESSION" ][ "_ts" ] = ts;

	}

	void SaveSession()
	{
		// Do we have session objects?
		if ( !m_ppbSession || !m_plockSession )
			return;

		// Lock the session data object
		CScopeLock ll( m_plockSession );
		if ( !ll.isLocked() )
			return;

		// Ensure session id
		if ( !m_pb[ "SESSION" ].isSet( "_id" ) || m_pb[ "TX_HEADERS" ].isSet( "Set-Cookie" ) )
			return;

		// Grab session id
		t_string id = m_pb[ "SESSION" ][ "_id" ].str();

		// Erase data if session has been marked invalid
		if ( !m_pb[ "SESSION" ][ "_ts" ].ToULong() )
			m_ppbSession->erase( id );

		// Save the new session data
		else
			(*m_ppbSession)[ id ] = m_pb[ "SESSION" ];

		// Add id to headers if new session
		if ( m_bNewSession )
			m_pb[ "TX_HEADERS" ][ "Set-Cookie" ] = CreateCookie( id );
	}

	void GrabConnectionInfo()
	{
		m_pb[ "REQUEST" ][ "REQUEST_TIME" ] = sys::get_gmt_timestamp();

		// Punt if no port
		if ( !m_pPort )
			return;

		t_string sLocal = m_pPort->LocalAddress().GetDotAddress();
		t_string sRemote = m_pPort->PeerAddress().GetDotAddress();

		t_pb8 &rq = m_pb[ "REQUEST" ];
		rq[ "SERVER_ADDR" ] = sLocal;
		rq[ "SERVER_PORT" ] = m_pPort->LocalAddress().getPort();
		rq[ "REMOTE_ADDR" ] = sRemote;
		rq[ "REMOTE_PORT" ] = m_pPort->PeerAddress().getPort();
		rq[ "IS_REMOTE" ] = ( sLocal != sRemote && sRemote != "127.0.0.1" ) ? "1" : "";
		rq[ "TRANSPORT_TYPE" ] = m_pPort->v_get_transport_type();
		rq[ "TRANSPORT_NAME" ] = m_pPort->v_get_transport_name();
		rq[ "TRANSACTION" ] = GetTransactions();
		rq[ "TRANSACTION_ID" ] = GetTransactionId();
	}

	/// Reads in the http headers
	int ReadHeaders()
	{
		// Ditch whatever we had 
		m_pb.erase( "REQUEST" );
		m_pb.erase( "RX_HEADERS" );
		m_bHeaderReceived = 0;

		// Ensure we have complete headers
		t_string::size_type eh = m_rx.find( "\r\n\r\n" );
		if ( t_string::npos == eh )
			return 0;

		// Parse off the headers from the buffer
		t_string rx( m_rx, 0, eh );
		m_rx.erase( 0, eh + 4 );

		t_string::size_type s = 0, e = 0;

		// The first line is the request string
		e = rx.find_first_of( "\r\n", s );

		// Parse off the request string
		t_string req( rx, s, e );

		// Are there Headers?
		if ( t_string::npos != e )
			m_pb[ "RX_HEADERS" ] = parser::DecodeMime< t_pb8 >( t_string( rx, e ) );

		// Save the request string
		m_pb[ "REQUEST" ][ "REQUEST_STRING" ] = req;

		// Request type
		t_string method = str::Token( req );
		if ( !method.length() )
			return HTTP_BAD_REQUEST;
		req.erase( 0, method.length() + 1 );

		// Grab the path
		t_string path = str::Token( req );
		if ( !path.length() )
			return HTTP_BAD_REQUEST;
		req.erase( 0, path.length() + 1 );

		// Request method
		t_string type = str::Token< t_string >( req, "/" );
		if ( type != "HTTP" )
			return HTTP_BAD_REQUEST;
		req.erase( 0, type.length() + 1 );

		// Whatever is left should be the version
		m_pb[ "REQUEST" ][ "ver" ] = req;

		// Save the request method
		m_pb[ "REQUEST" ][ "REQUEST_METHOD" ] = method;

		// The client should have provided us with a host name
		// This is how the client 'sees' us
		t_string host = m_pb[ "RX_HEADERS" ][ "host" ].str();
		if ( !host.length() )
			host = m_pPort->PeerAddress().GetDotAddress();

		// Decode URI information
		parser::DecodeUri( disk::WebPath< t_string >( t_string( "http://" ) + host, path ), 
						   m_pb[ "REQUEST" ][ "uri" ], true );

		// Map get parameters
		if ( m_pb[ "REQUEST" ][ "uri" ][ "get" ].length() )
			m_pb[ "GET" ] = parser::DecodeUrl< t_pb8 >( m_pb[ "REQUEST" ][ "uri" ][ "get" ].str() );

		// Add connection information
		GrabConnectionInfo();

		// Attempt to restore session information
		RestoreSession();

		// Headers received
		m_bHeaderReceived = 1;

		return HTTP_OK;

	}

    /// Sets the default header values
    int DefaultHeaders()
	{
		// Lose old header values
		m_pb.erase( "TX_HEADERS" );

		// Set the server name
		m_pb[ "TX_HEADERS" ][ "Server" ] = "";

		// Add timestamp
		m_pb[ "TX_HEADERS" ][ "Date" ] = sys::format_gmt_time< t_string >( "%w, %d %b %Y %g:%m:%s GMT" );

		// Last modified
		m_pb[ "TX_HEADERS" ][ "Last-modified" ] = "";

		// Let the client know we're supporting persistent connections
		m_pb[ "TX_HEADERS" ][ "Connection" ] = "Keep-Alive";

		// Let the client know we're supporting persistent connections
		m_pb[ "TX_HEADERS" ][ "X-Transaction" ] = m_nTransactions;

		return 1;
	}

    /// Returns the content object
    t_string& Content() { return m_sContent; }

    /// Sets the content type
    int SetContentType( const t_string &x_sFile )
	{
		// Set content type from extension / file
		m_pb[ "TX_HEADERS" ][ "Content-type" ] = disk::GetMimeType( x_sFile );

		return 1;
	}

    /// Sets the content type
    int SetContentMimeType( const t_string &x_sMime )
	{
		// Set content type from extension / file
		m_pb[ "TX_HEADERS" ][ "Content-type" ] = x_sMime;

		return 1;
	}

    /// Sends the specified error message back to the client
    int SendErrorMsg( int nErrorCode, const t_string &sMsg )
	{
		m_nErrorCode = nErrorCode;

		Content() += "<html><body><h1>";
		Content() += sMsg;
		Content() += "</h1></body></html>";

		// Send it
		SendReply();

		return 1;
	}


    /// Returns the human readable string for the specified error code
    t_string GetErrorString( long x_nErr )
	{
		switch( x_nErr )
		{
			case HTTP_OK :
				return "HTTP/1.1 200 OK\r\n";

			case HTTP_NO_CONTENT :
				return "HTTP/1.1 204 No Content\r\n";

			case HTTP_PARTIAL_CONTENT :
				return "HTTP/1.1 206 Partial Content\r\n";

			case HTTP_BAD_REQUEST :
				return "HTTP/1.1 400 Bad Request\r\n";

			case HTTP_AUTHORIZATION_REQUIRED :
				return "HTTP/1.1 401 Authorization Required\r\n";

			case HTTP_FORBIDDEN :
				return "HTTP/1.1 403 Forbidden\r\n";

			case HTTP_NOT_FOUND :
				return "HTTP/1.1 404 Document Not Found\r\n";

			case HTTP_REQUEST_TIMEOUT :
				return "HTTP/1.1 408 Request timed out\r\n";

			case HTTP_NOT_IMPLEMENTED :
				return "HTTP/1.1 501 Not Implemented\r\n";

			case HTTP_SERVER_ERROR :
			default :
				return "HTTP/1.1 500 Server Error\r\n";
		}

		return "HTTP/1.1 500 Server Error\r\n";
	}

	t_string GetReply()
	{
		t_string sReply;

		// How big is the data?
		m_pb[ "TX_HEADERS" ][ "Content-length" ] = m_sContent.length();

		// Send the header
		sReply += GetErrorString( m_nErrorCode );

		// Send the headers
		sReply += parser::EncodeMime( m_pb[ "TX_HEADERS" ], 1 ); 
		sReply += "\r\n";

		// Send the content
		sReply += m_sContent;

		return sReply;
	}

	int SendHeaders( long lLength, long lErrorCode = -1 )
	{
		// Log the request
		Log();

		// Save session data
		SaveSession();

		// Correct error code
		if ( 0 > lErrorCode )
			lErrorCode = m_nErrorCode;

		// How big is the data?
		if ( 0 <= lLength )
			m_pb[ "TX_HEADERS" ][ "Content-length" ] = lLength;

		// Send response string
		WritePort( GetErrorString( lErrorCode ) );

		// Send the headers
		WritePort( parser::EncodeMime( m_pb[ "TX_HEADERS" ], 1 ) + "\r\n" );

		return 1;
	}

    /// Sends a reply to the client
    int SendReply()
	{
		if ( !m_pPort )
			return 0;

		// Do we need to send a file?
		if ( !m_sContent.length() && m_sFile.length() )
			return SendFile( m_sFile, m_sFileType );

		// For compression support
		t_string *pSend = &m_sContent;

#ifdef HTM_ENABLE_ZIP

		t_string sCompressed;

		// Is compression enabled, and does the client support it?
		if ( m_bEnableCompression )
		{
			// Currently only supporting zlib/deflate
			if ( 0 <= str::Match( m_pbRxHeaders[ "accept-encoding" ].str(), "deflate" ) )
			{	m_pb[ "TX_HEADERS" ][ "content-encoding" ] = "deflate";
				sCompressed = util::compress( m_sContent );
				pSend = &sCompressed;
			} // end if

		} // end if

#endif

		// Send the headers
		SendHeaders( pSend->length() );

		// Send the content
		WritePort( *pSend );

		return 1;

	}

	int WritePort( const t_string &sStr )
	{	return WritePort( sStr.c_str(), sStr.length() );
	}

	int WritePort( const void *pBuf, long uSize = 0 )
	{
// tcS( str::t_string( (const char*)pBuf, uSize ) );

		// +++ Hmmm...
//		if ( !uSize )
//			uSize = zstr::Length( (const t_byte*)pBuf );

		if ( !uSize )
			return 0;

		// Send the data
		long uSent = 0;
		while ( uSent < uSize )
		{
			// Send out as much data as we can
			long uRet = m_pPort->Send( &((const t_byte*)pBuf)[ uSent ], uSize - uSent );
			if ( !uRet && m_pPort->IsError() )
				return 0;

			// Track bytes sent
			uSent += uRet;

			// Wait if buffer is full
			if ( uSent < uSize )
				if ( !m_pPort->WaitEvent( CIpSocket::eWriteEvent ) )
					return 0;

		} // end while

		return 1;
	}

    int SendBinary( const void *x_p, long x_l, const t_string &x_sType = "" )
    {
		if ( !x_p || 0 >= x_l )
			return 0;

		// Set type
		if ( x_sType.length() )
			SetContentType( x_sType );

		// Send the headers
		SendHeaders( x_l );

		// Write the data
		return WritePort( x_p, x_l );

	}

    int SendBinary( t_buffer *x_pBuffer, const t_string &x_sType = "" )
    {
		if ( !x_pBuffer )
			return 0;

		return SendBinary( x_pBuffer->data(), x_pBuffer->length(), x_sType );
	}

    int SendFile( const t_string &x_sFile, const t_string &x_sType = "" )
	{
		if ( !x_sFile.length() || !disk::exists( x_sFile.c_str() ) )
			return 0;

		// Set content type
		SetContentType( x_sType.length() ? x_sType : x_sFile );

		disk::HFILE hFile = disk::Open( x_sFile.c_str(), "rb" );
		if ( !hFile )
			return 0;

		// Send the headers
		SendHeaders( disk::Size( hFile ) );

		// Write out the file
		// Do this in chunks in case the file is huge
		unsigned char buf[ 1024 ];
		disk::t_size read = 0;
		while ( 0 < ( read = disk::Read( buf, 1, sizeof( buf ), hFile ) ) )
			if ( !WritePort( buf, read ) )
				return 0;

		return 1;
	}

    /// Sets the HTTP reply code
    void SetHTTPReplyCode( long nErrorCode ) { m_nErrorCode = nErrorCode; }

	/// Returns a reference to the internal receive buffer
	t_string& Rx()
	{	return m_rx; }

	/// Returns the number of transactions processes by this class
	long GetTransactions()
	{	return m_nTransactions; }

	/// Returns the property bag reference
	t_pb8& pb() 
	{	return m_pb; }

	/// Sets a callback function
	void SetCallback( PFN_Callback x_fnCallback, void *x_pData )
	{	m_fnCallback = x_fnCallback; m_pData = x_pData; }

	/// +++ Added this one to get things compiling, please change to above function
	void SetCallback( void *x_pCallback, void *x_pData )
	{	m_fnCallback = (PFN_Callback)x_pCallback; m_pData = x_pData; }

	/// Sets a authentication function
	void SetAuthCallback( PFN_Authenticate x_fnAuthenticate, void *x_pData )
	{	m_fnAuthenticate = x_fnAuthenticate; m_pAuthData = x_pData; }

	/// +++ Sets a authentication function
	void SetAuthCallback( void *x_fnAuthenticate, void *x_pData )
	{	m_fnAuthenticate = (PFN_Authenticate)x_fnAuthenticate; m_pAuthData = x_pData; }

	/// Sets a close function
	void SetCloseCallback( PFN_Close x_fnClose, void *x_pData )
	{	m_fnClose = x_fnClose; m_pCloseData = x_pData; }

	/// +++ Sets a close function
	void SetCloseCallback( void *x_fnClose, void *x_pData )
	{	m_fnClose = (PFN_Close)x_fnClose; m_pCloseData = x_pData; }

	/// Sets the log file name
	int SetLogFile( const t_string &x_sLog )
	{	m_sLog = x_sLog; return 1; }

	/// Sets the path to a file to send as a reply
	void SetFileName( const t_string &sFile, const t_string &sType = "" )
	{	m_sFile = sFile; m_sFileType = sType; }

	/// Returns the name of the file to be sent as a reply
	t_string GetFileName() { return m_sFile; }

	/// Reply file type
	t_string GetFileType() { return m_sFileType; }

	/// Enables / disables compression
	void EnableCompression( int b )
	{
#ifdef HTM_ENABLE_ZIP
		m_bEnableCompression = b;
#else
		m_bEnableCompression = 0;
#endif
	}

	/// Returns non-zero if compression is enabled
	int IsCompressionEnabled()
	{	return m_bEnableCompression; }

	t_string CommonLog()
	{
		t_string s;

		s += m_pb[ "REQUEST" ][ "REMOTE_ADDR" ].str();
		s += " -"; // rfc931
		s += " -"; // username
		s += " "; s += sys::format_gmt_time< t_string >( "[%d/%b/%Y:%g:%m:%s %Zs%Zh%Zm]" );
		s += " \""; s += m_pb[ "REQUEST" ][ "REQUEST_STRING" ].str(); s += "\"";
		s += " "; s += m_nErrorCode;
		s += " "; s += m_sContent.length();

		// Add referer if specified
		if ( m_pb[ "RX_HEADERS" ][ "referer" ].length() )
			s += " \"", s += m_pb[ "RX_HEADERS" ][ "referer" ].str(), s += "\"";
		else
			s += " -";

		// Add user agent if specified
		if ( m_pb[ "RX_HEADERS" ][ "user-agent" ].length() )
			s += " \"", s += m_pb[ "RX_HEADERS" ][ "user-agent" ].str(), s += "\"";
		else
			s += " -";

		s += "\r\n";

		return s;
	}

	// Writes the transaction log to a file
	long Log()
	{
		if ( !m_sLog.length() )
			return 0;

		return disk::AppendFile( m_sLog, CommonLog() );
	}

	/// Set session object
	void SetServerId( const t_string &sSid )
	{	m_sServerId = sSid; }

	/// Returns the server id
	t_string GetServerId()
	{	return m_sServerId; }

	/// Set session object
	void SetSessionObject( t_pb8 *pPb, CLock *pLock )
	{	m_sCookieId = "SID_"; m_sCookieId += m_sServerId; m_ppbSession = pPb; m_plockSession = pLock; }

	/// Returns non-zero if this is a new session
	int IsNewSession()
	{	return m_bNewSession; }

	/// Sets the length of time that session data is to be valid
	void SetSessionTimeout( unsigned int uTo )
	{	m_uSessionTimeout = uTo; }

	/// Sets our unique transaction id
	void SetTransactionId( long nTid )
	{	m_nTransactionId = nTid; }

	/// Returns this sessions unique transaction id
	long GetTransactionId()
	{	return m_nTransactionId; }

	/// Sets the mapped folders list
	void SetMappedFoldersList( t_pb8 *pList, CLock *pLock )
	{	m_pMappedFolders = pList; m_pMappedFoldersLock = pLock; }

	/// Returns non-zero if the port is still connected
	int IsConnected()
	{	if ( !m_pPort )
			return 0;
		return m_pPort->IsConnected();
	}

	/// Sets the maximum size of a request in bytes
	/**
		If you plan to accept large file uploads, 
		you will want to increase this value.
	*/
	void setMaxRequestSize( unsigned int m ) { m_uMaxRequestBytes = m; }

	/// Returns the maximum size of a request in bytes
	unsigned int getMaxRequestSize() { return m_uMaxRequestBytes; }

	/// Sets the maximum time in seconds a client is allowed to be idle
	void setMaxKeepAlive( unsigned int m ) { m_uMaxKeepAlive = m; }

	/// Returns the maximum time in seconds a client is allowed to be idle
	unsigned int getMaxKeepAlive() { return m_uMaxKeepAlive; }

	/// Set the default path
	void setDefaultPath( const t_string &s ) { m_sDefaultPath = s; }

	/// Returns the default path
	t_string getDefaultPath() { return m_sDefaultPath; }

private:

	/// Our port
	T_PORT						*m_pPort;

	/// Parameters
	t_pb8						m_pb;
/*
    /// Incomming HTTP headers
    t_pb8		     			m_pbRxHeaders;

    /// Outgoing HTTP headers
    t_pb8		     		  	m_pbTxHeaders;

    /// Request information
    t_pb8		    		  	m_pbRequest;

    /// Get variables
    t_pb8		    		   	m_pbGet;

    /// Post variables
    t_pb8		     		  	m_pbPost;

    /// Session variables
    t_pb8		     		  	m_pbSession;
*/
    /// Non-zero if the complete HTTP headers have been received.
    int       	 		    	m_bHeaderReceived;

	/// Number of transactions processed
    unsigned int				m_nTransactions;

    /// Content to return to client
    t_string           		    m_sContent;

    /// Error code
    unsigned int      		   	m_nErrorCode;

	/// Receive buffer
    t_string					m_rx;

	/// Log file
	t_string					m_sLog;

	/// Name of a file to send if m_sContent is empty
	t_string					m_sFile;

	/// File type
	t_string					m_sFileType;

	/// Pointer to callback function
	PFN_Callback		    	m_fnCallback;

	/// Data passed to callback function
	void						*m_pData;

	/// Pointer to authentication function
	PFN_Authenticate			m_fnAuthenticate;

	/// Data passed to callback function
	void						*m_pAuthData;

	/// Pointer to close function
	PFN_Close					m_fnClose;

	/// Data passed to callback function
	void						*m_pCloseData;

	/// Non-zero to enable compression
	int							m_bEnableCompression;

	/// Unique server id
	t_string					m_sServerId;

	/// Session cookie id
	t_string					m_sCookieId;

	/// Stores session data
	t_pb8						*m_ppbSession;

	/// Lock for session data access
	CLock						*m_plockSession;

	/// Non-zero if a new session was just created
	int							m_bNewSession;

	/// Length of time in seconds that session data is to be valid
	unsigned int				m_uSessionTimeout;

	/// Keep-alive timeout
	unsigned int				m_uKeepAlive;

	/// Maximum amount of time a client can be idle
	unsigned int				m_uMaxKeepAlive;

	/// Maximum request size
	unsigned int				m_uMaxRequestBytes;

	/// Sets our transaction id
	long						m_nTransactionId;

	/// Pointer to list of mapped folders
	t_pb8						*m_pMappedFolders;

	/// Pointer to mapped folders lock
	CLock						*m_pMappedFoldersLock;

	/// Sets the default path for http requests
	t_string					m_sDefaultPath;

};

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
// CHttpServer
//
/// Provides basic HTTP server
/**

  \code


  \endcode

*/
//==================================================================

template < typename T_SPORT, typename T_SESSION, typename T_CPORT = T_SPORT > class THttpServer : public CThread
{
public:

	enum
	{
		/// Invalid event
		eSeNone = 0,

		/// Server connected
		eSeConnect = 1,

		/// Server disconnected
		eSeDisconnect = 2,

		/// Server accepted new connection
		eSeAccept = 3,

		/// Cleanup interval in seconds
		eCleanupInterval = 10,

		/// Maximum number of sessions to keep, and simultaneous clients
		///  After this amount is reached, oldest sessions will be dropped
		/**
			I'm being conservative, this is pretty low.  If you have really
			optimized your pages, and you really want to serve lots of pages
			per second, you will want to use higher values.

			On my DOS testing, I used setMaxConn( 256 ), and was able to get
			performance of around 1000 requests per second.

			However, if you have pages that are really slow, the system may 
			become seriously backlogged.  You may want to use setMaxKeepAlive() 
			to set the session processing time low to help gaurd against this.
		*/
		eMaxSessions = 16,

		/// Maximum socket queue size
		eMaxQueue = 16

	};

	// +++ This also exists in THttpSession, should probably be in once place
	enum
	{
		// Don't allow clients to linger idle longer than one minute
		eDefaultKeepAlive = 60,

		// Don't process requests bigger than this
		eMaxRecvBytes = 1 * 1024 * 1024
	};

	/// Server callback
	typedef int (*PFN_OnServerEvent)( void *x_pData, int x_nEvent, int x_nErr, THttpServer< T_SPORT, T_SESSION, T_CPORT > *x_pServer );

	/// String type
	typedef str::t_string8 	t_string;

public:

	/// Holds information about a session
	class CSessionInfo
	{
	public:

		/// Constructor
		CSessionInfo()
		{
			pPortFactory = 0;
			port = 0;
		}

		/// Destructor
		~CSessionInfo()
		{
			if ( pPortFactory && port )
				pPortFactory->Free( port );
			pPortFactory = 0;
			port = 0;
		}

		/// Returns non-zero if we need to keep the connection running
		int IsValid()
		{
			// One transaction?
			if ( !session.KeepAlive() )
				return 0;

			// Is the port still alive?
			if ( !port->IsRunning() )
				return 0;

			return 1;
		}

		// Updates the connection
		int Update( unsigned long x_uTimeout )
		{
			if ( !port )
				return -1;

			// Any data waiting?
			if ( !port->WaitEvent( T_SPORT::eReadEvent, x_uTimeout ) )
				return 0;

			// Go process it
			if ( 0 > session.OnRead( 0 ) )
				return -2;

			return 1;
		}

		/// Sets port and free function
		void SetPort( T_CPORT *p, cmn::CFactory *f )
		{	port = p; pPortFactory = f; }

		/// Session object
		T_SESSION			session;

		/// Session port
		T_CPORT				*port;

		/// Function that frees the port
		cmn::CFactory		*pPortFactory;

	};

	/// Holds information about a session
	class CSessionThread :
		public CSessionInfo,
		public CThread
	{
	public:

		virtual long InitThread( void *x_pData )
		{
			return 1;
		}

		virtual long DoThread( void *x_pData )
		{
			// While thread is running and no transactions
			while ( getStopEvent().Wait( 0 ) && CSessionInfo::IsValid() )
				if ( 0 > CSessionInfo::Update( 100 ) )
					return -2;

			return -1;
		}

	};

	/// Session list type
	typedef std::list< CSessionThread > t_LstSessionThread;

	/// Session list type
	typedef std::list< CSessionInfo > t_LstSessionInfo;

	/// Processes multiple sessions
	class CSingleSessionThread : public CThread
	{
	public:

		/// Constructor
		CSingleSessionThread()
		{
			m_pSessionInfo = 0;
			m_pSessionLock = 0;
		}

		virtual long InitThread( void *x_pData )
		{
			return 1;
		}

		/// Do the work
		virtual long DoThread( void *x_pData )
		{
			if ( !m_pSessionInfo || !m_pSessionLock )
				return 0;

			unsigned int uWait = 0;
			while ( getStopEvent().Wait( 0 ) )
			{
				int bUpdate = 0;

				// Get current size of the list
				unsigned int uSize = m_pSessionInfo->size();
				unsigned int uProcessed = 0;

				// Anything to do?
				if ( !uSize )
					sys::sleep( 15 );

				else
					for ( typename t_LstSessionInfo::iterator it = m_pSessionInfo->begin();
						  uSize-- && m_pSessionInfo->end() != it; )
						if ( it->port->GetConnectState() )
						{	uProcessed++;
							if ( it->IsValid() )
							{
								int r = it->Update( uWait );

								if ( 0 > r )
								{	if ( it->port )
										it->port->Destroy();
								} // end if

								else if ( r )
									bUpdate = 1;

								uWait = 0;

								it++;

							} // end if
							else if ( it->port )
							{
								// Must lock if we're close to the end
								CScopeLock ll( m_pSessionLock );
								if ( !ll.isLocked() )
									return 0;

								// Erase item
								m_pSessionInfo->erase( it++ );

							} // end if

						} // end if

				// Maybe we had sockets that weren't ready?
				if ( uSize && !uProcessed )
					sys::sleep( 15 );

				// Don't hog the processor
				if ( !bUpdate )
					uWait = 15;
				else
					uWait = 0;

			} // end while

			return 0;
		}

		/// Session information
		t_LstSessionInfo 	*m_pSessionInfo;

		/// Session information lock
		CLock				*m_pSessionLock;

	};

public:

	/// Default constructor
	THttpServer()
	{
		m_pData = 0;
		m_fnOnServerEvent = 0;
		m_nTransactions = 0;
		m_pSessionData = 0;
		m_pSessionCallback = 0;
		m_pAuthData = 0;
		m_pAuthCallback = 0;
		m_bEnableSessions = 0;
		m_bLocalOnly = 0;
		m_uSessionTimeout = 60 * 60;
		m_uCleanup = 0;
		m_bMultiThreaded = 1;
		m_pPortFactory = 0;
		m_uMaxConnections = eMaxSessions;
		m_uMaxQueue = eMaxQueue;
		m_uMaxKeepAlive = eDefaultKeepAlive;
		m_uMaxRequestBytes = eMaxRecvBytes;
	}

	~THttpServer()
	{
		// Stop the server
		Stop();

		m_pData = 0;
		m_fnOnServerEvent = 0;
		m_pSessionData = 0;
		m_pSessionCallback = 0;
		m_pAuthData = 0;
		m_pAuthCallback = 0;
		m_bEnableSessions = 0;
		m_bLocalOnly = 0;
		m_pPortFactory = 0;
		m_uMaxKeepAlive = eDefaultKeepAlive;
		m_uMaxRequestBytes = eMaxRecvBytes;
	}

	int StartServer( int x_nPort, PFN_OnServerEvent fnOnServerEvent = 0, void *x_pData = 0 )
	{
		Destroy();

		m_nPort = x_nPort;
		m_pData = x_pData;
		m_fnOnServerEvent = fnOnServerEvent;

		// Ensure we have a server id
		if ( !m_sServerId.length() )
			m_sServerId = uid::ToStr< t_string >();

		return 0 == CThread::Start();
	}

	void SetServerId( const t_string &x_sServerId )
	{
		m_sServerId = x_sServerId;
	}

	t_string& GetServerId() { return m_sServerId; }

	void SetPortFactory( cmn::CFactory *p )
	{	m_pPortFactory = p; }

	cmn::CFactory* GetPortFactory()
	{ return m_pPortFactory; }

protected:

	int ThreadStartServer()
	{
		// Bind to port
		if ( !m_server.Bind( m_nPort ) )
		{	if ( m_fnOnServerEvent )
				m_fnOnServerEvent( m_pData, eSeConnect, -1, this );
			return 0;
		} // end if

		// Listen
		if ( !m_server.Listen( m_uMaxQueue ) )
		{	if ( m_fnOnServerEvent )
				m_fnOnServerEvent( m_pData, eSeConnect, -2, this );
			return 0;
		} // end if

		// Notify that server is running
		if ( m_fnOnServerEvent )
			m_fnOnServerEvent( m_pData, eSeConnect, 0, this );

		return 1;
	}

	virtual long InitThread( void *x_pData )
	{
		// Attempt to start the server
		if ( !ThreadStartServer() )
			return 0;

		return 1;
	}

	/// Authenticate the connection
	virtual int OnAuthenticate( T_CPORT &port )
	{
		// Accepting anyone?
		if ( !m_bLocalOnly )
			return 1;

		// Get local and remote address
		t_string sLocal = port.LocalAddress().GetDotAddress();
		t_string sRemote = port.PeerAddress().GetDotAddress();

		// Verify it is a local address
		if ( sLocal != sRemote && sRemote != "127.0.0.1" )
			return 0;

		return 1;
	}

	int InformSession( T_SESSION &session, T_CPORT &port )
	{
		// Initialize the session
		session.Init();

		// Count a transaction
		session.SetTransactionId( m_nTransactions++ );

		// Set default maximum request size
		session.setMaxRequestSize( getMaxRequestSize() );

		// Set default maximum idle time
		session.setMaxKeepAlive( getMaxKeepAlive() );

		// Set the default path
		session.setDefaultPath( getDefaultPath() );

		// Set the log file name
		session.SetLogFile( m_sLog );

		// Set the callback function for the data
		session.SetCallback( m_pSessionCallback, m_pSessionData );

		// Set the authentication callback
		session.SetAuthCallback( m_pAuthCallback, m_pAuthData );

		// Connect the port
		session.SetPort( &port );

		// Set default session timeout
		session.SetSessionTimeout( m_uSessionTimeout );

		// Let the session know the server id
		session.SetServerId( m_sServerId );

		// Mapped folders
		session.SetMappedFoldersList( &m_pbMappedFolders, &m_lockMappedFolders );

		// Enable sessions?
		if ( m_bEnableSessions )
			session.SetSessionObject( &m_pbSession, &m_lockSession );

		return 1;
	}

	/// Accepts multi-threaded connection
	int MultiAccept()
	{
		// Allocate a new port object
		T_CPORT *port = 0;
		if ( m_pPortFactory )
			port = (T_CPORT*)m_pPortFactory->Create();
		else
			port = (T_CPORT*)m_cDefaultPortFactory.Create();
		if ( !port )
			return 0;

		// Add a new session
		m_lstSessionThread.resize( m_lstSessionThread.size() + 1 );
		typename t_LstSessionThread::iterator it = --m_lstSessionThread.end();

		// Set the port
		it->SetPort( port, m_pPortFactory ? m_pPortFactory : &m_cDefaultPortFactory );

		// Attempt to connect session
		if ( !m_server.Accept( *it->port ) )
		{
			// Erase session
			m_lstSessionThread.erase( it );

			// Let user in on the error
			if ( m_fnOnServerEvent )
				m_fnOnServerEvent( m_pData, eSeAccept, -1, this );
				
			return 0;

		} // end if

		// Drop session if too many connections
		if ( m_lstSessionThread.size() >= (unsigned)m_uMaxConnections )
			m_lstSessionThread.erase( it );

		// Authenticate the connection
		else if ( !OnAuthenticate( *it->port ) )
		{
			m_lstSessionThread.erase( it );

			if ( m_fnOnServerEvent )
				m_fnOnServerEvent( m_pData, eSeAccept, -1, this );

		} // end if

		else
		{
			// Fill in session information
			InformSession( it->session, *it->port );

			// Notify of new connection
			if ( m_fnOnServerEvent )
				m_fnOnServerEvent( m_pData, eSeAccept, 0, this );

			// Start the thread
			it->Start();

		} // end if

		return 1;
	}

	/// Accepts single-threaded connection
	int SingleAccept()
	{
		// Allocate a new port object
		T_CPORT *port = 0;
		if ( m_pPortFactory )
			port = (T_CPORT*)m_pPortFactory->Create();
		else
			port = (T_CPORT*)m_cDefaultPortFactory.Create();
		if ( !port )
			return 0;

		// Must lock to make changes
		CScopeLock ll( m_lockSessionInfo );
		if ( !ll.isLocked() )
			return 0;

		// Add a new session
		m_lstSessionInfo.resize( m_lstSessionInfo.size() + 1 );
		typename t_LstSessionInfo::iterator it = --m_lstSessionInfo.end();

		// Set the port
		it->SetPort( port, m_pPortFactory ? m_pPortFactory : &m_cDefaultPortFactory );

		// Attempt to connect session
		if ( !m_server.Accept( *it->port ) )
		{
			// Erase session
			m_lstSessionInfo.erase( it );

			// Let user in on the error
			if ( m_fnOnServerEvent )
				m_fnOnServerEvent( m_pData, eSeAccept, -1, this );
				
			return 0;

		} // end if

		// Erase session
		if ( m_lstSessionInfo.size() >= (unsigned)m_uMaxConnections )
			m_lstSessionInfo.erase( it );

		// Authenticate the connection
		else if ( !OnAuthenticate( *it->port ) )
		{
			m_lstSessionInfo.erase( it );

			if ( m_fnOnServerEvent )
				m_fnOnServerEvent( m_pData, eSeAccept, -1, this );

		} // end if

		else
		{
			// Fill in session information
			InformSession( it->session, *it->port );

			// Notify of new connection
			if ( m_fnOnServerEvent )
				m_fnOnServerEvent( m_pData, eSeAccept, 0, this );

		} // end if

		// Start the processing thread if needed
		if ( !m_cSingleSessionThread.isRunning() )
		{
			// Some info the thread will need
			m_cSingleSessionThread.m_pSessionInfo = &m_lstSessionInfo;
			m_cSingleSessionThread.m_pSessionLock = &m_lockSession;

			// Start the thread
			m_cSingleSessionThread.Start();

		} // end if

		return 1;
	}

	virtual long DoThread( void *x_pData )
	{
		// Wait for stop event
		while ( getStopEvent().Wait( 0 ) )
		{
			// Wait for connect event
			if ( m_server.WaitEvent( T_SPORT::eAcceptEvent, 100 ) )
			{
				if ( m_bMultiThreaded )
					MultiAccept();
				else
					SingleAccept();

			} // end if

			// Did we lose the connection?
			else if ( m_server.IsError() )
			{
				// Drop old socket
				m_server.Destroy();

				// Wait a bit
				sys::sleep( 3000 );

				// Restart the server
				ThreadStartServer();

			} // end else

			// Clean up expired connections
			CleanupConnections();

			// Is it time to cleanup sessions?
			if ( m_uCleanup )
				m_uCleanup--;
			else
				CleanupSessions();

		} // end while

		return 0;
	}

	int CleanupConnections()
	{
		// Check for expired connections
		if ( m_lstSessionThread.size() )
			for ( typename t_LstSessionThread::iterator it = m_lstSessionThread.begin();
				  m_lstSessionThread.end() != it; )
				if ( !it->isRunning() )
					it = m_lstSessionThread.erase( it++ );
				else
					it++;

		return 1;
	}

	int CleanupSessions()
	{
		m_uCleanup = eCleanupInterval * 10;

		// Attempt to cleanup session data
		CScopeLock ll( m_lockSession );
		if ( ll.isLocked() )
		{
			// +++ This gets us by, but it would be nice to drop the oldest
			//     connections based on _ts.  Currently just dropping anything
			//     that hasn't communicated in 3 seconds when we're over the limit.

			// Do we need to drop sessions?
			int bDrop = eMaxSessions < m_pbSession.size();

			// Remove timed out sessions
			long ts = (long)sys::get_gmt_timestamp();
			for ( t_pb8::iterator it = m_pbSession.begin(); m_pbSession.end() != it; )
				if ( !it->second->isSet( "_ts" ) )
					m_pbSession.erase( it++ );

				else if ( ( (*it->second)[ "_ts" ].ToLong() + (long)m_uSessionTimeout ) < ts )
					m_pbSession.erase( it++ );

				else if ( bDrop && ( (*it->second)[ "_ts" ].ToLong() + 3 ) < ts )
					m_pbSession.erase( it++ );

				else
					it++;

		} // end if

		return 1;
	}

	virtual long EndThread( void *x_pData )
	{
		// Ensure the session thread has stopped
		m_cSingleSessionThread.Stop();

		// Stop the server
		m_server.Destroy();

		// Lose all sessions
		m_lstSessionThread.clear();

		// Lose session info objects
		m_lstSessionInfo.clear();

		// Notify that server is running
		if ( m_fnOnServerEvent )
			m_fnOnServerEvent( m_pData, eSeDisconnect, 0, this );

		return 1;
	}

public:

	/// Returns the number of client transactions serviced
	int GetNumTransactions()
	{	return m_nTransactions; }

	/// Sets the session callback function
	void SetSessionCallback( void *x_pCallback, void *x_pData )
	{	m_pSessionCallback = (typename T_SESSION::PFN_Callback)x_pCallback; m_pSessionData = x_pData; }

	/// Sets the session callback function
	void SetSessionCallback( typename T_SESSION::PFN_Callback x_pCallback, void *x_pData )
	{	m_pSessionCallback = x_pCallback; m_pSessionData = x_pData; }

	/// Sets the session callback function
	void SetAuthCallback( void *x_pCallback, void *x_pData )
	{	m_pAuthCallback = (typename T_SESSION::PFN_Authenticate)x_pCallback; m_pAuthData = x_pData; }

	/// Sets the session callback function
	void SetAuthCallback( typename T_SESSION::PFN_Authenticate x_pCallback, void *x_pData )
	{	m_pAuthCallback = x_pCallback; m_pAuthData = x_pData; }

	/// Returns the number of active sessions
	int GetNumActiveClients()
	{	return m_lstSessionThread.Size() + m_lstSessionInfo.Size(); }

	/// Sets the log file name
	int SetLogFile( const t_string &x_sLog )
	{	m_sLog = x_sLog;
		return 1;
	}

	/// Returns reference to port object
	T_SPORT& Port() { return m_server; }

	/// Enable / disable sessions
	void EnableSessions( int b ) { m_bEnableSessions = b; }

	/// Enable / disable remote connections
	void EnableRemoteConnections( int b ) { m_bLocalOnly = !b; }

	/// Sets the length of time that session data is to be valid
	void SetSessionTimeout( unsigned int uTo )
	{	m_uSessionTimeout = uTo; }

	/// Enable / disable multi threading
	//  For some reason, I enable doing this while the server is running ;)
	void EnableMultiThreading( int b )
	{	m_bMultiThreaded = b; }

	/// Maps / unmaps a folder
	int MapFolder( const t_string &sName, const t_string &sFolder, int bMap )
	{
		if ( !sName.length() )
			return 0;

		if ( !sFolder.length() )
			return 0;

		CScopeLock ll( m_lockMappedFolders );
		if ( !ll.isLocked() )
			return 0;

		if ( bMap )
			m_pbMappedFolders[ sName ][ sFolder ] = 1;
		else
		{	m_pbMappedFolders[ sName ].erase( sFolder );
			if ( !m_pbMappedFolders[ sName ].size() )
				m_pbMappedFolders = 0;
		} // end else

		return 1;
	}

	/// Sets the maximum number of connections
	void setMaxConn( int m ) { m_uMaxConnections = m; }

	/// Gets the maximum number of connections
	int getMaxConn() { return m_uMaxConnections; }

	/// Sets the maximum connection queue size
	void setMaxQueue( int m ) { m_uMaxQueue = m; }

	/// Gets the maximum connection queue size
	int getMaxQueue() { return m_uMaxQueue; }

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

	/// The TCP port to listen
	int										m_nPort;

	/// Server port
	T_SPORT									m_server;

	/// Max connections
	int										m_uMaxConnections;

	/// Max connection queue
	int										m_uMaxQueue;

	/// Transactions
	long									m_nTransactions;

	/// Maximum amount of time a client can be idle
	unsigned int							m_uMaxKeepAlive;

	/// Maximum request size
	unsigned int							m_uMaxRequestBytes;

	/// List of session thread objects (multi-threaded)
	t_LstSessionThread						m_lstSessionThread;

	/// List of session info objects (single-threaded)
	t_LstSessionInfo						m_lstSessionInfo;

	/// Runs single threaded sessions
	CSingleSessionThread					m_cSingleSessionThread;

	/// Data passed to m_fnOnServerEvent
	void									*m_pData;

	/// Function to call on server event
	PFN_OnServerEvent						m_fnOnServerEvent;

	/// Data passed to session callback
	void									*m_pSessionData;

	/// Pointer to session callback function
	typename T_SESSION::PFN_Callback		m_pSessionCallback;

	/// Data passed to session callback
	void									*m_pAuthData;

	/// Pointer to session callback function
	typename T_SESSION::PFN_Authenticate 	m_pAuthCallback;

	/// Log file name
	t_string								m_sLog;

	/// Enable sessions
	int										m_bEnableSessions;

	/// Unique server id
	t_string								m_sServerId;

	/// Stores session data
	t_pb8									m_pbSession;

	/// Lock for session data access
	CLock									m_lockSession;

	/// Non-zero to accept local connections only
	int										m_bLocalOnly;

	/// Length of time in seconds that session data is to be valid
	unsigned long							m_uSessionTimeout;

	/// Time to cleanup sessions;
	unsigned long							m_uCleanup;

	/// Non-zero to enable multi-threading
	int										m_bMultiThreaded;

	/// Session info lock
	CLock									m_lockSessionInfo;

	/// Lock for the mapped folders list
	CLock									m_lockMappedFolders;

	/// List of mapped folders
	t_pb8									m_pbMappedFolders;

	/// Manages port creation / destruction
	cmn::CFactory							*m_pPortFactory;

	/// Default port factory
	cmn::TFactory< T_CPORT >				m_cDefaultPortFactory;

	/// Sets the default path for http requests
	t_string								m_sDefaultPath;

};

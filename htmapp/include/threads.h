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
// CThreadResource
//
/// Encapsulates a system resource handle
/**
	Ok, the idea here, right or wrong, is to encapsulate all the
	different types of handles like files, threads, mutexes, etc
	into a single structure so that they can be released, and most
	importantly to me at the moment, 'waited on' in a unified
	manner.  Windows already does this to a large extent,
	unfortunately, Posix does not.

	This class acts as a true handle, there is no memory management.
	You must call Destroy() manually when your done with this resource.

	This may be better implemented if CThreadResource were a super class
	and subing out the specifics.  Not sure, I have my reasons for
	doing it this way atm. ;)

*/
//==================================================================
struct SThreadResourceInfo;
class CThreadResource
{
public:

	enum E_RESOURCE_TYPE
	{
		/// Invalid resource type
		eRtInvalid 			= 0,

		/// System file
		eRtFile				= 1,

		/// Network socket
		eRtSocket			= 2,

		/// Process thread
		eRtThread			= 3,

		/// Thread mutex
		eRtMutex			= 4,

		/// Event
		eRtEvent			= 5,

		/// Lock
		eRtLock				= 6
	};

	enum etWait
	{
		/// Value indicating wait succeeded
		waitSuccess 		= 0,

		/// Value indicating wait timed out
		waitTimeout 		= -1,

		/// Value indicating wait expired
		waitFailed 			= -2
	};

	/// Default wait time in milliseconds
	enum { eDefaultWaitTime = 60000 };

	/// The wait resolution in micro-seconds
	enum { eWaitResolution = 15000 };

	/// Thread callback function
	typedef void* (*PFN_ThreadProc)( void* x_pData );

public:

	/// Call to initialize threads
	static bool Init();

	/// Call to release thread resources
	static void UnInit();
	
public:

	/// Handle type
	typedef void*		t_HANDLE;


	/// Exception class
	class CException
	{
	public:

		// Default constructor
		CException() { m_error = 0; }

		// Initializing constructor
		CException( long e ) { m_error = 0; }

		// Error code
		long getError() { return m_error; }

		// Error code
		long m_error;
	};

	/// Injects an exception into the specified thread
	static unsigned long InjectException( void* hThread, long nError );

	/// Initialize exception function
	static void InitException();

	/// Error value
	// volatile static long s_last_error;

public:

	/// Constructor
	CThreadResource()
	{	m_hHandle = cInvalid();
		m_eType = eRtInvalid;
	}

	/// Destructor
	virtual ~CThreadResource()
	{
		Destroy();
	}

	/// Copy constructor
	CThreadResource( const CThreadResource &x_r )
	{	if ( 1 < x_r.AddRef() )
		{	m_hHandle = x_r.m_hHandle;
			m_eType = x_r.m_eType;
		} // end if
		else
		{	m_hHandle = cInvalid();
			m_eType = eRtInvalid;
		} // end else
	}


	/// Assignment operator
	CThreadResource& operator = ( const CThreadResource &x_r )
	{	Destroy();
		if ( 1 < x_r.AddRef() )
		{	m_hHandle = x_r.m_hHandle;
			m_eType = x_r.m_eType;
		} // end if
		else
		{	m_hHandle = cInvalid();
			m_eType = eRtInvalid;
		} // end else
		return *this;
	}

	/// Adds a reference count to the objcet
	long AddRef() const;

	//==============================================================
	// Destroy()
	//==============================================================
	/// Releases the handle
	/**
		\param [in] x_uTimeout	-	Maximum amount of time to wait for
									the resource to be released
									gracefully.
		\param [in] x_bForce	-	If x_uTimeout expires, and x_bForce
									is non-zero, the resource will be
									forcefully removed.  If x_uTimeout
									expires and x_bForce is zero, the
									resource will be abandoned.

		return	error code for the release operation.   Zero if
				success, -1 if the operation could not be performed.
	*/
	virtual long Destroy( unsigned long x_uTimeout = eDefaultWaitTime, bool x_bForce = true );

	/// Detaches from the resource without freeing it
	void Detach()
	{	m_hHandle = cInvalid();
		m_eType = eRtInvalid;
	}

	/// Creates a mutex object
	/**
		\param [in] x_sName 		-	Name for the mutex
		\param [in] x_bInitialOwner	-	Non-zero if the calling thread
										should take ownership before
										the function returns.
	*/
	long NewMutex( const str::t_char8* x_sName = tcNULL, bool x_bInitialOwner = false );

	/// Creates an event object
	/**
		\param [in] x_sName			-	Event name, NULL for unamed event.
		\param [in] x_bManualReset	-	TRUE if the event must be manually
										reset
		\param [in] x_bInitialState	-	The initial state of the event,
										non-zero for signaled.
	*/
	long NewEvent( const str::t_char8* x_sName = tcNULL, bool x_bManualReset = true, bool x_bInitialState = false );

	/// Creates a thread
	/**
		\param [in] x_fnCallback	-	Thread callback function
		\param [in] x_pData			-	User data passed to thread
										callback function
	*/
	long NewThread( PFN_ThreadProc x_fnCallback, void* x_pData );

	/// Creates a thread lock
	/**
		\param [in] x_sName			-	Event name, NULL for unamed event.
		\param [in] x_bInitialOwner	-	Non-zero if the calling thread
										should take ownership before
										the function returns.
	*/
	long NewLock( const str::t_char8* x_sName = tcNULL, bool x_bInitialOwner = false );


private:

    /// The thread procedure
    static void* ThreadProc( void* x_pData );

    // Protect access to ThreadProc()
    class CThreadProcImpl;
    friend class CThreadProcImpl;

public:

	//==============================================================
	// Wait()
	//==============================================================
	/// Waits for the handle to signal
	/**
		\param [in] x_uTimeout 		-	Maximum time to wait for signal

		This is a consolidated wait function.  It's exact characteristics
		depend on the type of object being waited on.

		\return Returns zero if success, otherwise an error code is
				returned.
	*/
	virtual long Wait( unsigned long x_uTimeout = eDefaultWaitTime );

	//==============================================================
	// Signal()
	//==============================================================
	/// Signals the handle if the resource type supports it
	/**
		\param [in] x_uTimeout	- Maximum amount of time to wait for
								  the signal to become available for
								  signaling.

		\return Returns zero if success, otherwise an error code is
				returned.
	*/
	virtual long Signal( unsigned long x_uTimeout = eDefaultWaitTime );

	//==============================================================
	// Reset()
	//==============================================================
	/// Resets the signaled state if the resource type supports it
	/**
		\param [in] x_uTimeout	- Maximum amount of time to wait for
								  the signal to become available for
								  resetting.

		\return Returns zero if success, otherwise an error code is
				returned.
	*/
	virtual long Reset( unsigned long x_uTimeout = eDefaultWaitTime );

	//==============================================================
	// vInfinite()
	//==============================================================
	/// Value representing infinite timeout.
	/**
		Use with wait functions
	*/
	static unsigned long cInfinite()
    {
		return c_Infinite;
    }

	//==============================================================
	// vInvalid()
	//==============================================================
	/// Value corresponding to an invalid handle value
	static t_HANDLE cInvalid()
    {
    	return c_Invalid;
    }

	//==============================================================
	// getThreadId()
	//==============================================================
	/// Returns the current thread id
	static t_HANDLE getThreadId();

	//==============================================================
	// isValid()
	//==============================================================
	/// Returns the os dependent handle for the resource
    bool isValid()
    {
    	return ( cInvalid() != m_hHandle );
	}

	//==============================================================
	// getHandle()
	//==============================================================
	/// Returns the os dependent handle for the resource
    t_HANDLE getHandle()
    {	return m_hHandle;
	}

	//==============================================================
	// getOwner()
	//==============================================================
	/// Returns the thread id of the resource owner
	/**
		For threads this is the creator.

		For Mutexes and Locks, it is the thread that has it locked.

		For Events, it is the thread that last signaled it

	*/
    void* getOwner();

	//==============================================================
	// GetRi()
	//==============================================================
	/// Returns m_hHandle as SResourceInfo pointer
	SThreadResourceInfo* getRi()
	{	return (SThreadResourceInfo*)m_hHandle; }

	//==============================================================
	// SetHandle()
	//==============================================================
	/// Sets the os dependent handle for the resource
    void setHandle( void* x_hHandle, E_RESOURCE_TYPE x_eType )
    {
    	m_hHandle = x_hHandle;
		m_eType = x_eType;
	}

	//==============================================================
	// GetType()
	//==============================================================
	/// Returns the type of the os dependent resource
    E_RESOURCE_TYPE getType()
    {
    	return m_eType;
	}

	//==============================================================
	// SetType()
	//==============================================================
	/// Sets the type of the os dependent resource
    void setType( E_RESOURCE_TYPE x_eType )
    {
    	m_eType = x_eType;
	}

public:

	//==============================================================
	// WaitMultiple()
	//==============================================================
	/// Waits for multiple resources to become signaled
	/**
		\param [in] x_nCount		-	Number of CResource objects in
										x_pResources
		\param [in] x_pResources	-	Array of CResource object pointers
		\param [in] x_uTimeout 		-	Maximum time to wait for signal
		\param [in] x_nMin			-	Minimum number of events to become
										signaled.  This only works on posix
										systems.  On Windows, if x_nMin is
										zero, WaitMultiple() will return when
										ANY of the events become signaled,
										if x_nMin is non-zero, it will only
										return once ALL events become signaled.
										Note: To ensure the same result on all
										platforms, set x_nMin to zero for ANY
										and set to x_nCount for ALL.

		This is a consolidated wait function.  It's exact characteristics
		depend on the type of object being waited on.

		\return Returns negative number if failure, otherwise, returns the index of a
				signaled object.
	*/
	static long WaitMultiple( long x_nCount, CThreadResource **x_pResources,
			   	  			  unsigned long x_uTimeout = eDefaultWaitTime,
			   	  			  long x_nMin = 0 );


	/// Waits for the specified events or the stop signal
	long Wait( CThreadResource& rRes1, unsigned long x_uTimeout = eDefaultWaitTime, long x_nMin = 0 )
	{	CThreadResource* p[] = { this, &rRes1 };
		return CThreadResource::WaitMultiple( 2, p, x_uTimeout, x_nMin );
	}

	/// Waits for the specified events or the stop signal
	long Wait( CThreadResource& rRes1, CThreadResource& rRes2,
				unsigned long x_uTimeout = eDefaultWaitTime, long x_nMin = 0 )
	{	CThreadResource* p[] = { this, &rRes1, &rRes2 };
		return CThreadResource::WaitMultiple( 3, p, x_uTimeout, x_nMin );
	}

	/// Waits for the specified events or the stop signal
	long Wait( CThreadResource& rRes1, CThreadResource& rRes2, CThreadResource& rRes3,
				unsigned long x_uTimeout = eDefaultWaitTime, long x_nMin = 0 )
	{	CThreadResource* p[] = { this, &rRes1, &rRes2, &rRes3 };
		return CThreadResource::WaitMultiple( 4, p, x_uTimeout, x_nMin );
	}

	/// Waits for the specified events or the stop signal
	long Wait( CThreadResource& rRes1, CThreadResource& rRes2, CThreadResource& rRes3,
				CThreadResource& rRes4, unsigned long x_uTimeout = eDefaultWaitTime, long x_nMin = 0 )
	{	CThreadResource* p[] = { this, &rRes1, &rRes2, &rRes3, &rRes4 };
		return CThreadResource::WaitMultiple( 5, p, x_uTimeout, x_nMin );
	}

	/// Waits for the specified events or the stop signal
	long Wait( CThreadResource& rRes1, CThreadResource& rRes2, CThreadResource& rRes3,
				CThreadResource& rRes4, CThreadResource& rRes5,
					unsigned long x_uTimeout = eDefaultWaitTime, long x_nMin = 0 )
	{	CThreadResource* p[] = { this, &rRes1, &rRes2, &rRes3, &rRes4, &rRes5 };
		return CThreadResource::WaitMultiple( 6, p, x_uTimeout, x_nMin );
	}

	/// Waits for the specified events or the stop signal
	long Wait( CThreadResource& rRes1, CThreadResource& rRes2, CThreadResource& rRes3,
				CThreadResource& rRes4, CThreadResource& rRes5, CThreadResource &rRes6,
					unsigned long x_uTimeout = eDefaultWaitTime, long x_nMin = 0 )
	{	CThreadResource* p[] = { this, &rRes1, &rRes2, &rRes3, &rRes4, &rRes5, &rRes6 };
		return CThreadResource::WaitMultiple( 7, p, x_uTimeout, x_nMin );
	}

	/// Waits for the specified events or the stop signal
	long Wait( CThreadResource& rRes1, CThreadResource& rRes2, CThreadResource& rRes3,
				CThreadResource& rRes4, CThreadResource& rRes5, CThreadResource &rRes6,
				CThreadResource& rRes7, unsigned long x_uTimeout = eDefaultWaitTime, long x_nMin = 0 )
	{	CThreadResource* p[] = { this, &rRes1, &rRes2, &rRes3, &rRes4, &rRes5, &rRes6, &rRes7 };
		return CThreadResource::WaitMultiple( 8, p, x_uTimeout, x_nMin );
	}

	/// Waits for the specified events or the stop signal
	long Wait( CThreadResource& rRes1, CThreadResource& rRes2, CThreadResource& rRes3,
				CThreadResource& rRes4, CThreadResource& rRes5, CThreadResource &rRes6,
				CThreadResource& rRes7, CThreadResource& rRes8, 
				unsigned long x_uTimeout = eDefaultWaitTime, long x_nMin = 0 )
	{	CThreadResource* p[] = { this, &rRes1, &rRes2, &rRes3, &rRes4, &rRes5, &rRes6, &rRes7, &rRes8 };
		return CThreadResource::WaitMultiple( 9, p, x_uTimeout, x_nMin );
	}

private:

	/// Invalid event handle value
	static t_HANDLE		c_Invalid;

	/// Infinite timeout value
	static unsigned long	c_Infinite;

private:

	/// The resource handle
	t_HANDLE					m_hHandle;

	/// Resource type
	E_RESOURCE_TYPE				m_eType;
};

//==================================================================
// CLock
//
/// Generic thread lock
//==================================================================
class CLock : public CThreadResource
{
public:

	/// Copy constructor
	CLock( const CLock &r ) : CThreadResource( r ) { }
	CLock& operator = ( const CLock &r ) { *(CThreadResource*)this = r;  return *this; }

	/// Default constructor
	CLock( const str::t_char8* x_pName = tcNULL )
	{	NewLock( x_pName ); }

	/// Create lock
	bool Create( const str::t_char8* x_pName = tcNULL )
	{	return 0 == NewLock( x_pName ); }

};


//==================================================================
// CScopeLock
//
/// Use this to lock and automatically unlock CResource objects
/**
	Use this to lock and automatically unlock CResource objects
*/
//==================================================================
class CScopeLock
{

public:

	/// Default constructor
	CScopeLock()
    {
        m_ptr = tcNULL;
    }

	/// Destructor - Unlocks the underlying CResource object
	virtual ~CScopeLock()
    {
        Unlock();
    }

	//==============================================================
	// CScopeLock()
	//==============================================================
	/// Constructor - Takes a CResource pointer
	/**
		\param [in] x_ptr		-	Pointer to CResource object
		\param [in] x_uTimeout	-	Maximum time in milli-seconds to
									wait for lock.
	*/
	CScopeLock( CThreadResource *x_ptr, unsigned long x_uTimeout = CThreadResource::eDefaultWaitTime )
	{   m_ptr = tcNULL;
		if ( x_ptr )
            if ( 0 == x_ptr->Wait( x_uTimeout ) )
                m_ptr = x_ptr;
    }

	//==============================================================
	// CScopeLock()
	//==============================================================
	/// Constructor - Takes a CResource reference
	/**
		\param [in] x_lock		-	Reference to CResource object
		\param [in] x_uTimeout	-	Maximum time in milli-seconds to
									wait for lock.
	*/
	CScopeLock( CThreadResource &x_lock, unsigned long x_uTimeout = CThreadResource::eDefaultWaitTime )
	{   m_ptr = tcNULL;
        if ( 0 == x_lock.Wait( x_uTimeout ) )
            m_ptr = &x_lock;
    }

	//==============================================================
	// CScopeLock()
	//==============================================================
	/// Constructor - Takes a CResource reference
	/**
		\param [in] x_ptr		-	Pointer to CResource object
		\param [in] x_uTimeout	-	Maximum time in milli-seconds to
									wait for lock.
	*/
	CScopeLock( CLock *x_ptr, unsigned long x_uTimeout = CThreadResource::eDefaultWaitTime )
	{   m_ptr = tcNULL;
		if ( x_ptr )
            if ( 0 == x_ptr->Wait( x_uTimeout ) )
                m_ptr = x_ptr;
    }

	//==============================================================
	// CScopeLock()
	//==============================================================
	/// Constructor - Takes a CResource reference
	/**
		\param [in] x_lock		-	Reference to CLock object
		\param [in] x_uTimeout	-	Maximum time in milli-seconds to
									wait for lock.
	*/
	CScopeLock( CLock &x_lock, unsigned long x_uTimeout = CThreadResource::eDefaultWaitTime )
	{   m_ptr = tcNULL;
        if ( 0 == x_lock.Wait( x_uTimeout ) )
            m_ptr = &x_lock;
    }

	//==============================================================
	// isLocked()
	//==============================================================
	/// Returns true if the local object is locked
	bool isLocked( void* x_uWho = CThreadResource::getThreadId() )
    {
		if ( !m_ptr )
			return false;

		return m_ptr->getOwner() == x_uWho;

    }

	//==============================================================
	// Attach()
	//==============================================================
	/// Attaches to an existing CResource without locking
	void Attach( CThreadResource *x_ptr )
    {
        Unlock();
        m_ptr = x_ptr;
    }

	//==============================================================
	// Detach()
	//==============================================================
	/// Detaches from CResource without unlocking
	void Detach()
    {
        m_ptr = tcNULL;
    }

	//==============================================================
	// Lock()
	//==============================================================
	/// Locks a CResource object.  Returns true only if lock was achieved
	/**
		\param [in] x_ptr		-	Pointer to CResource object
		\param [in] x_uTimeout	-	Maximum time in milli-seconds to
									wait for lock.

		\return Non-zero if lock was acquired.

		\see
	*/
	bool Lock( CThreadResource *x_ptr, unsigned long x_uTimeout = CThreadResource::eDefaultWaitTime )
	{
		// Do we already have the lock?
        if ( x_ptr == m_ptr )
            return true;

		// Unlock existing
        Unlock();

		// Ensure valid lock pointer
		if ( !x_ptr )
            return false;

		// Attempt to acquire the lock
		if ( x_ptr->Wait( x_uTimeout ) )
			return false;

        m_ptr = x_ptr;

		return true;
	}

	//==============================================================
	// Lock()
	//==============================================================
	/// Locks a CResource object.  Returns true only if lock was achieved
	/**
		\param [in] x_lock		-	Reference to CResource object
		\param [in] x_uTimeout	-	Maximum time in milli-seconds to
									wait for lock.

		\return Non-zero if lock was acquired.

		\see
	*/
	bool Lock( CThreadResource &x_lock, unsigned long x_uTimeout = CThreadResource::eDefaultWaitTime )
	{	return Lock( &x_lock, x_uTimeout ); }

	//==============================================================
	// Unlock()
	//==============================================================
	/// Unlocks attached CResource object
	/**
		\return Always returns non-zero
	*/
	bool Unlock()
	{
		if ( !m_ptr )
            return true;

		m_ptr->Reset();
        m_ptr = tcNULL;

		return true;
	}

private:

	/// Pointer to CResource object
	CThreadResource		*m_ptr;

};

//==================================================================
// CEvent
//
/// Thread event
//==================================================================
class CEvent : public CThreadResource
{
public:

	/// Copy constructor
	CEvent( const CEvent &r ) : CThreadResource( r ) { }
	CEvent& operator = ( const CEvent &r ) { *(CThreadResource*)this = r; return *this; }

	/// Default constructor
	CEvent( const str::t_char8* x_pName = tcNULL )
	{	NewEvent( x_pName ); }

	/// Create lock
	bool Create( const str::t_char8* x_pName = tcNULL )
	{	return 0 == NewEvent( x_pName ); }

};

//==================================================================
// CSignal
//
/// Thread signals
//==================================================================
class CSignal : public CThreadResource
{
public:

	/// Copy constructor
	CSignal( const CSignal &r ) : CThreadResource( r ), m_nosig( r.m_nosig ) { }
	CSignal& operator = ( const CSignal &r ) { *(CThreadResource*)this = r;  m_nosig = r.m_nosig; return *this; }

	/// Default constructor
	CSignal( const str::t_char8* x_pName = tcNULL )
	{	Create( x_pName ); }

	/// Create lock
	bool Create( const str::t_char8* x_pName = tcNULL );

	/// Destroys the event
	virtual void Destroy() { CThreadResource::Destroy(); m_nosig.Destroy(); }

	/// Signals the event
	virtual long Signal( unsigned long x_uTimeout = CThreadResource::eDefaultWaitTime )
	{	return m_nosig.Reset() | CThreadResource::Signal(); }

	/// Resets the event
	virtual long Reset( unsigned long x_uTimeout = CThreadResource::eDefaultWaitTime )
	{	return CThreadResource::Reset() | m_nosig.Signal(); }

	/// Returns reset signal
	CThreadResource& GetResetEvent() { return m_nosig; }

private:

	/// Triggered when not signaled
	CThreadResource	m_nosig;
};


//==================================================================
// CThread
//
/// Thread class
/**
*/
//==================================================================
class CThread : public CThreadResource
{
public:

	/// Default Constructor
	CThread();

	/// Destrucotr
	virtual ~CThread();

public:

	/// Atomically increments the specified value
	static unsigned long Increment( long *p );

	/// Atomically decrements the specified value
	static unsigned long Decrement( long *p );

public:

	//==============================================================
	// Start()
	//==============================================================
	/// Starts the thread
	/**
		\param [in] x_pData			-	Custom data passed on to thread
        \param [in] x_uSleep        -   Sleep time in mSec.  If not
                                        zero, a sleep of this length
                                        will be injected after every
                                        call to DoThread().

		\return Non-zero if thread was started.

		\see
	*/
	virtual long Start( void* x_pData = tcNULL );

	//==============================================================
	// Stop()
	//==============================================================
	/// Stops any running thread
	/**
		\param [in] x_uWait    	-	Time in milli-seconds to wait for
								    thread to stop before killing it.
		\param [in] x_bKill	    -	Set to non-zero to kill thread if
                                    it fails to stop gracefully.

		\return Non-zero if success

		\see
	*/
	virtual long Stop( unsigned long x_uWait = CThreadResource::eDefaultWaitTime, bool x_bKill = true );

protected:

	//==============================================================
	// InitThread()
	//==============================================================
	/// Do thread initialization here
	/**
		\param [in] x_pData - User defined data

		The pData value passed to these functions represent the original value
		in m_pvoidData when the thread started.  Changing m_pvoidData in these
		functions does not affect the passed value to the next function.
		This -hopefully- garentees that x_pData is always equal to the value
		passed to Start()

		Overide this function to provide custom thread Initialization
		If you don't need initialization you can just use DoThread()

		x_pData - whatever was passed to StartThread()

		return non-zero if you want to continue the thread ( calling DoThread() )
		return zero to end the thread now

		\return Non-zero to continue thread, zero to terminate thread execution.

		\see DoThread(), EndThread(), GhostThread()
	*/
	virtual long InitThread( void* x_pData ) { return 1; }

	//==============================================================
	// DoThread()
	//==============================================================
	/// Normal thread work
	/**
		\param [in] x_pData	-	User defined value

		Overide this function to provide custom thread work

		This function is called over and over until it returns less
		than zero or Stop() is called

		x_pData - whatever was passed to Start()

        return >= zero if you want more processing
        (i.e. DoThread() will be called again after retval sleep )

		return less than zero to end the thread

		\return >= zero to continue thread,
				< zero to terminate thread execution.

		\see InitThread(), EndThread(), GhostThread()
	*/
	virtual long DoThread( void* x_pData ) { return -1; }

	//==============================================================
	// EndThread()
	//==============================================================
	/// Do thread cleanup here
	/**
		\param [in] x_pData	-	User defined value

		Overide this function to provide custom cleanup

		x_pData - whatever was passed to StartThread()

		The return value is ignored
		if you want to return a thread value then set m_dwThreadReturn

		\return Return value is the return value for the thread

		\see InitThread(), DoThread(), GhostThread()
	*/
	virtual long EndThread( void* x_pData ) { return 0; }

public:

    /// Returns the running state of the thread
    bool isRunning();

    /// Returns the user value passed to Start()
    void* getUserData() { return m_pData; }

	//==============================================================
	// getThreadCount()
	//==============================================================
	/// Returns the total number of threads started using this class
	/**
		This is retrieved from a static variable.  Indicates
		the number of threads started using this class.

		\return Number of threads started using this class

		\see
	*/
	static long getThreadCount();

	//==============================================================
	// getRunningThreadCount()
	//==============================================================
	/// Returns the total number of threads running using this class
	/**
		This is retrieved from a static variable.  Indicates
		the number of threads currently running using this class.

		\return Number of threads running using this class

		\see
	*/
	static long getRunningThreadCount();

    /// Waits for thread to initialize
    bool WaitThreadExit( unsigned long x_uTimeout = CThreadResource::eDefaultWaitTime );

	/// Returns a reference to the stop event
	CThreadResource& getStopEvent() { return m_evStop; }

	/// Returns a reference to the init event
	CThreadResource& getInitEvent() { return m_evInit; }
	
	/// Returns a pointer to the stop flag
	volatile long* getStopFlag() { return &m_lStopFlag; }

	/// Return value of InitThread()
	/// call GetInitEvent().Wait() to ensure it's valid
	bool getInitStatus() { return m_bInitStatus; }

	/// Force an exception in the thread
	unsigned long InjectException( long nCode );

private:

	//==============================================================
	// ThreadProc()
	//==============================================================
	/// Static thread callback function
	static void* ThreadProc( void* x_pData );

protected:

	//==============================================================
	// IncThreadCount()
	//==============================================================
	/// Increments the total thread count
	static void IncThreadCount();

	//==============================================================
	// IncRunningThreadCount()
	//==============================================================
	/// Increments the running thread count
	static void IncRunningThreadCount();

	//==============================================================
	// DecRunningThreadCount()
	//==============================================================
	/// Decrements the running thread count
	static void DecRunningThreadCount();

private:

    /// Users data
    void*	                                m_pData;

	/// Return value of InitThread()
	bool									m_bInitStatus;

	/// Stop flag, non-zero if thread should stop
	volatile long							m_lStopFlag;

public:
	
    /// Count of all threads that were created
    static long                             m_lThreadCount;

    /// Number of threads still running
    static long                             m_lRunningThreadCount;
	
protected:

    /// Quit signal
	CEvent									m_evStop;

    /// Signals when the thread has initialized
	CEvent									m_evInit;

};



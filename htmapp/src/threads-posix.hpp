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

#include <stdlib.h>

#if !defined( HTM_NOTHREADTIMEOUTS )
#	define HTM_THREAD_TIMEOUTS
#endif

#if !defined( OHTM_NOTHREADFORCEFAIRNESS )
#	define HTM_THREAD_FORCEFAIRNESS
#endif

// #define HTM_PSEUDOLOCKS
#if defined( HTM_PSEUDOLOCKS )
#endif

//#if defined( CII_FRWK_apache )
//#	define HTM_THREAD_USE_GETTIMEOFDAY
//#endif

#if defined( HTM_THREAD_USE_GETTIMEOFDAY )
#	include <sys/time.h>
#endif

CThreadResource::t_HANDLE CThreadResource::c_Invalid = (CThreadResource::t_HANDLE)-1;

unsigned long CThreadResource::c_Infinite = 0xffffffff;

// This is currently the limit in Windows.   Tho there is no real
// limit in Posix, but we want things to be cross platform right? ;)
#define MAXIMUM_WAIT_OBJECTS	64

/// Contains OS specific information about a resource
struct SThreadResourceInfo
{
	/// Reference count
	long								uRef;

	CThreadResource						cSync;

	// Handle
	union
	{
		void*							pVoid;
		pthread_mutex_t					hMutex;
		pthread_cond_t					hCond;
		pthread_t						hThread;
	};

	// Owner information
	void*								uOwner;
	void*								uLastOwner;

	// Thread info
	void*								pData;
	CThreadResource::PFN_ThreadProc		fnCallback;

	// Event status
	bool								bSignaled;
	bool								bManualReset;

	// Lock info
	unsigned long						nCount;
	unsigned long						nWaiting;

};

static SThreadResourceInfo* CreateRi()
{
	SThreadResourceInfo *pRi = new SThreadResourceInfo;
	if ( !pRi )
		return tcNULL;

	pRi->uRef = 1;

	pRi->uOwner = 0;
	pRi->uLastOwner = 0;
	pRi->fnCallback = 0;
	pRi->nCount = 0;
	pRi->bSignaled = 0;
	pRi->bManualReset = 0;
	pRi->nWaiting = 0;

	return pRi;
}

static long FreeRi( SThreadResourceInfo* x_pRi, long x_eType, unsigned long x_uTimeout, bool x_bForce )
{
	// Any resources to release?
	if ( CThreadResource::cInvalid() == x_pRi || CThreadResource::eRtInvalid == x_eType )
		return -1;

	// Get pointer to resource information
	if ( !x_pRi )
		return -1;

	// Decrement reference
	long lRef = CThread::Decrement( &x_pRi->uRef );
	if ( 0 < lRef )
		return lRef;

	// Error code
	long nErr = 0;

	// Execute proper release function if valid handle
	switch( x_eType )
	{
		case CThreadResource::eRtInvalid :
			break;

		case CThreadResource::eRtFile :
			break;

		case CThreadResource::eRtSocket :
			break;

		case CThreadResource::eRtThread :
		{
			// Usually the creator will be the destructor
			if ( x_pRi->uOwner != CThreadResource::getThreadId() )
				;// ( 0, tcT( "Thread not being destroyed by owner!" ) );

			// Wait for thread to exit
			if ( x_pRi->cSync.Wait( x_uTimeout )
#ifndef HTM_NOPTHREADCANCEL
				 && pthread_cancel( x_pRi->hThread )
#endif
				 && x_pRi->cSync.Wait( x_uTimeout ) )
			{
				// iii  This should not happen, don't ignore the problem,
				//      figure out how to shut this thread down properly!
				// ( nErr, tcT( "!! Terminating thread !!" ) );

#ifndef HTM_NOPTHREADCANCEL

				// Kill the thread
				if ( x_bForce )
					if ( ( nErr = pthread_cancel( x_pRi->hThread ) ) )
						;// ( nErr, tcT( "pthread_cancel() failed" ) );
#else
				if ( x_bForce )
					;// ( nErr, tcT( "pthread_cancel() not available" ) );
#endif

			} // end if

			// Attempt to join the thread
			else if ( ( nErr = pthread_join( x_pRi->hThread, tcNULL ) ) )
				;// ( nErr, tcT( "pthread_join() failed" ) );

			// Release Mutex
			x_pRi->cSync.Destroy();

		} break;

		case CThreadResource::eRtLock :
		case CThreadResource::eRtMutex :

			// Initiailze mutex object
			if ( ( nErr = pthread_mutex_destroy( &x_pRi->hMutex ) ) )
				;//( nErr, tcT( "pthread_mutex_destroy() failed : Error releasing mutex object" ) );

			break;

		case CThreadResource::eRtEvent :

			// Release Mutex
			x_pRi->cSync.Destroy();

			// Release condition object
			if ( ( nErr = pthread_cond_destroy( &x_pRi->hCond ) ) )
				;//( nErr, tcT( "pthread_cond_destroy() failed : Error releasing pthread_cond object" ) );

			break;

		default :
			break;

	} // end switch

	// Drop the memory for real
	delete x_pRi;

	return 0;
}

/// Atomically increments the specified value
unsigned long CThread::Increment( long *p )
{
	return __sync_add_and_fetch( p, 1 );
}

/// Atomically decrements the specified value
unsigned long CThread::Decrement( long *p )
{
	return __sync_sub_and_fetch( p, 1 );
}

void* CThreadResource::getThreadId()
{
	return (void*)pthread_self();
}

long CThreadResource::AddRef() const
{
	if ( CThreadResource::cInvalid() == m_hHandle || !m_hHandle )
		return -1;
	SThreadResourceInfo *pRi = (SThreadResourceInfo*)m_hHandle;
	return CThread::Increment( &pRi->uRef );
}

long CThreadResource::Destroy( unsigned long x_uTimeout, bool x_bForce )
{
	// Ensure reasonable values
	if ( CThreadResource::cInvalid() == m_hHandle || CThreadResource::eRtInvalid == m_eType )
	{	m_hHandle = c_Invalid;
		m_eType = eRtInvalid;
		return -1;
	} // end if

	// Get destroy info
	SThreadResourceInfo* pRi = (SThreadResourceInfo*)m_hHandle;
	E_RESOURCE_TYPE eType = m_eType;

	// Good practice and all...
	m_hHandle = CThreadResource::cInvalid();
	m_eType = CThreadResource::eRtInvalid;

	// Free the resource
	if ( 0 > FreeRi( pRi, eType, x_uTimeout, x_bForce ) )
	{	;//( 0, tcT( "Invalid resource info object pointer" ) );
		return -1;
	} // end if

	return 0;
}

volatile long s_last_error = 0;

__attribute__((noreturn)) void ThrowException()
{
	// Throw exception
	throw( CThreadResource::CException( s_last_error ) );

	// Just in case
	exit( s_last_error );

	// Forever
	for(;;) sleep( 1000 );
}

bool CThreadResource::Init()
{
	s_last_error = 0;
	CThreadResource::c_Invalid = (CThreadResource::t_HANDLE)-1;
	CThreadResource::c_Infinite = 0xffffffff;
	CThread::m_lThreadCount = 0;
	CThread::m_lRunningThreadCount = 0;
	
	return true;
}

void CThreadResource::UnInit()
{
}

void CThreadResource::InitException()
{
	volatile int i = 0;
	if ( i )
	throw( CThreadResource::CException() );
}

unsigned long CThreadResource::InjectException( void* hThread, long nError )
{
	s_last_error = nError;

	return -1;
}

void* CThreadResource::getOwner()
{
	SThreadResourceInfo *pRi = (SThreadResourceInfo*)m_hHandle;
	if ( !pRi )
		return 0;

	return pRi->uOwner;
}

long CThreadResource::NewThread( PFN_ThreadProc x_fnCallback, void* x_pData )
{
	// Out with the old
	Destroy();

	SThreadResourceInfo *pRi = CreateRi();
	if ( !pRi )
		return -1; //( ENOMEM, tcT( "Out of memory" ) );

	m_eType = eRtThread;
	m_hHandle = (CThreadResource::t_HANDLE)pRi;

	// Save user data
	pRi->pData = x_pData;
	pRi->fnCallback = x_fnCallback;
	pRi->uOwner = getThreadId();

	// Create event object to indicate when thread has shut down properly
	if ( long nErr = pRi->cSync.NewEvent() )
	{	Destroy(); return nErr; } //( nErr, tcT( "Create event failed" ) ); }

	// Make thread joinable
	pthread_attr_t attr;
	pthread_attr_init( &attr );
	pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_JOINABLE );

	// Create the thread
	long nRet = pthread_create( &pRi->hThread, &attr,
							    CThreadResource::ThreadProc, pRi );

	// Lose attributes structure
	pthread_attr_destroy( &attr );

	if ( nRet )
	{	;//( nRet, tcT( "Error creating thread" ) );
		Destroy();
		return -1;
	} // end if

    return 0;
}

void* CThreadResource::ThreadProc( void* x_pData )
{
#ifndef HTM_NOPTHREADCANCEL

	// Allow thread to be canceled
	pthread_setcancelstate( PTHREAD_CANCEL_ENABLE, 0 );
	pthread_setcanceltype( PTHREAD_CANCEL_ASYNCHRONOUS, 0 );

#endif

	// Get pointer to resource information
	SThreadResourceInfo *pRi = (SThreadResourceInfo*)x_pData;
	if ( !pRi )
	{	;//( 0, tcT( "Invalid resource info object pointer" ) );
		return (void*)-1;
	} // end if

	// Call user thread
	void* pRet = pRi->fnCallback( pRi->pData );

	// Signal that we're done
	pRi->cSync.Signal();

	// Quit thread
	pthread_exit( pRet );

	// ???
	;//( 0, tcT( "pthread_exit() returned!" ) );

	return pRet;
}

long CThreadResource::NewMutex( const str::t_char8* x_sName, bool x_bInitialOwner )
{
	// Out with the old
	Destroy();

	SThreadResourceInfo *pRi = CreateRi();
	if ( !pRi )
		return -1; //( ENOMEM, tcT( "Out of memory" ) );

	// Save information
	m_eType = eRtMutex;
	m_hHandle = (CThreadResource::t_HANDLE)pRi;

	// Mutex attributes
	pthread_mutexattr_t attr;
	if ( pthread_mutexattr_init( &attr ) )
	{	Destroy(); return -1; } //( nErr, tcT( "pthread_mutexattr_init() failed" ) ); }

	// Set mutex type
	if ( pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE ) )
	{	Destroy(); return -1; } //( nErr, tcT( "pthread_mutexattr_settype() failed" ) ); }

	// Initiailze mutex object
	if ( pthread_mutex_init( &pRi->hMutex, &attr ) )
	{	Destroy(); return -1; } //( nErr, tcT( "ptherad_mutex_init() failed" ) ); }

	// Does the caller want to own it?
	if ( x_bInitialOwner )
		Wait( 0 );

	return 0;
}

long CThreadResource::NewEvent( const str::t_char8* x_sName, bool x_bManualReset, bool x_bInitialState )
{
	// Out with the old
	Destroy();

	SThreadResourceInfo *pRi = CreateRi();
	if ( !pRi )
		return -1; //( ENOMEM, tcT( "Out of memory" ) );

	// Initialize member variables
	m_eType = eRtEvent;
	m_hHandle = (CThreadResource::t_HANDLE)pRi;

	// Initialize event
	pRi->bSignaled = x_bInitialState;
	pRi->bManualReset = x_bManualReset;

	// Create the mutex object
	if ( pRi->cSync.NewMutex() )
	{	Destroy(); return -1; } //( nErr, tcT( "Create mutex failed" ) ); }

	// Create condition object
	if ( pthread_cond_init( &pRi->hCond, 0 ) )
	{	Destroy(); return -1; } //( nErr, tcT( "pthread_cond_init: failed" ) ); }

	return 0;
}

long CThreadResource::NewLock( const str::t_char8* x_sName, bool x_bInitialOwner )
{
	return NewMutex( x_sName, x_bInitialOwner );
}

long CThreadResource::Wait( unsigned long x_uTimeout )
{
	// Verify information
	if ( cInvalid() == m_hHandle || eRtInvalid == m_eType )
		return -1;

	// Get pointer to resource information
	SThreadResourceInfo *pRi = (SThreadResourceInfo*)m_hHandle;
	if ( !pRi )
	{	; //( 0, tcT( "Invalid resource info object pointer" ) );
		return -1;
	} // end if

	// Execute proper release function if valid handle
	switch( m_eType )
	{
		case eRtInvalid :
			break;

		case eRtFile :
			break;

		case eRtSocket :
			break;

		case eRtThread :
		{
			// Check to see if the thread has signaled a normal shutdown
			return pRi->cSync.Wait( x_uTimeout );

		} break;

		case eRtLock :
		case eRtMutex :
		{

#if defined( HTM_THREAD_FORCEFAIRNESS )

			// +++ This is to force fairness among threads
			//     There HAS to be a way to force this natively ????
			unsigned long nWaiting = pRi->nWaiting;
			if ( 0 < nWaiting )
			{
				// If we don't own the lock,
				// but we were the last to have it,
				// give someone else a chance.
				void* uId = getThreadId();
				if ( pRi->uOwner != uId && pRi->uLastOwner == uId )
				{
					unsigned long uMax = 10;
					while ( uMax && nWaiting == pRi->nWaiting )

						// iii Zero doesn't work ;)
						usleep( eWaitResolution ),
						uMax--;

					// Did something go wrong with the waiting count?
					if ( !uMax )
						pRi->nWaiting = 0;

				} // end if

			} // end if

			// Attempt lock
			if ( !pthread_mutex_trylock( &pRi->hMutex ) )
			{	pRi->nCount++;
				pRi->uOwner = getThreadId();
				return waitSuccess;
			} // end if
// (*tq::pb())[ "b" ] << "timeout(), ";

			// Does caller want to wait?
			if ( !x_uTimeout )
				return waitTimeout;

			// Someone is waiting
			pRi->nWaiting++;

#endif

		struct timespec to;

		// Get time
#	if !defined( HTM_THREAD_USE_GETTIMEOFDAY )
			clock_gettime( CLOCK_REALTIME, &to );
#	else
			struct timeval tp;
			gettimeofday( &tp, 0 );
			to.tv_sec = tp.tv_sec; to.tv_nsec = tp.tv_usec * 1000;
#	endif

			// Add our time
			to.tv_sec += x_uTimeout / 1000;
			to.tv_nsec += ( x_uTimeout % 1000 ) * 1000000;
			if ( to.tv_nsec > 1000000000 )
				to.tv_sec += to.tv_nsec / 1000000000,
				to.tv_nsec %= 1000000000;

				
#if defined( HTM_THREAD_TIMEOUTS )

			// Wait for the lock
			if ( !pthread_mutex_timedlock( &pRi->hMutex, &to ) )
			{	pRi->nCount++;
				if ( pRi->nWaiting ) pRi->nWaiting--;
				pRi->uOwner = getThreadId();
				return waitSuccess;
			} // end if

#else

			struct timespec toNow;

			do
			{
				// Wait for the lock
				if ( !pthread_mutex_trylock( &pRi->hMutex ) )
				{	pRi->nCount++;
					if ( pRi->nWaiting ) pRi->nWaiting--;
					pRi->uOwner = getThreadId();
					return waitSuccess;
				} // end if

				usleep( eWaitResolution );

#	if !defined( HTM_THREAD_USE_GETTIMEOFDAY )
				clock_gettime( CLOCK_REALTIME, &toNow );
#	else
				struct timeval tp;
				gettimeofday( &tp, 0 );
				toNow.tv_sec = tp.tv_sec; toNow.tv_nsec = tp.tv_usec * 1000;
#	endif
				if ( toNow.tv_nsec > 1000000000 )
					toNow.tv_sec += toNow.tv_nsec / 1000000000,
					toNow.tv_nsec %= 1000000000;

			} while ( toNow.tv_sec < to.tv_sec || toNow.tv_nsec < to.tv_nsec );

#endif
			if ( pRi->nWaiting ) pRi->nWaiting--;

			return waitTimeout;

		} break;

		case eRtEvent :
		{
			// Try to get out for free
			if ( pRi->bSignaled )
				return waitSuccess;

				// Does user want to wait?
			if ( !x_uTimeout && pRi->bManualReset )
				return waitTimeout;


			// Lock the mutex
			CScopeLock sl( pRi->cSync, x_uTimeout );
			if ( !sl.isLocked() )
				return waitFailed;


			// Ensure it wasn't signaled while we were waiting
			if ( pRi->bSignaled )
				return waitSuccess;

			struct timespec to;

			// Get time

#	if !defined( HTM_THREAD_USE_GETTIMEOFDAY )
			clock_gettime( CLOCK_REALTIME, &to );
#	else
			struct timeval tp;
			gettimeofday( &tp, 0 );
			to.tv_sec = tp.tv_sec; to.tv_nsec = tp.tv_usec * 1000;
#	endif

			// Add our time
			to.tv_sec += x_uTimeout / 1000;
			to.tv_nsec += ( x_uTimeout % 1000 ) * 1000000;
			if ( to.tv_nsec > 1000000000 )
				to.tv_sec += to.tv_nsec / 1000000000,
				to.tv_nsec %= 1000000000;


			// Get mutex handle
			SThreadResourceInfo *pRiMutex = (SThreadResourceInfo*)pRi->cSync.m_hHandle;

#if !defined( HTM_NOCONDTIMEDWAIT )

			// Wait for the lock
			if ( !pthread_cond_timedwait( &pRi->hCond, &pRiMutex->hMutex, &to ) )
				return pRi->bSignaled ? waitSuccess : waitTimeout;

#else

			struct timespec toNow;

			do
			{

#error +++ Need a solution here ;)

				// Wait for the lock
//				if ( !pthread_cond_wait( &pRi->hCond, &pRiMutex->hMutex ) )
					return pRi->bSignaled ? waitSuccess : waitTimeout;

				usleep( eWaitResolution );

#	if !defined( HTM_THREAD_USE_GETTIMEOFDAY )
				clock_gettime( CLOCK_REALTIME, &toNow );
#	else
				struct timeval tp;
				gettimeofday( &tp, 0 );
				toNow.tv_sec = tp.tv_sec; toNow.tv_nsec = tp.tv_usec * 1000;
#	endif
				if ( toNow.tv_nsec > 1000000000 )
					toNow.tv_sec += toNow.tv_nsec / 1000000000,
					toNow.tv_nsec %= 1000000000;

			} while ( toNow.tv_sec < to.tv_sec || toNow.tv_nsec < to.tv_nsec );

#endif

			return waitTimeout;

		} break;

	} // end switch

	// Unwaitable, ha ha
	; //( waitFailed, tcT( "Attempt to wait on unwaitable resource type" ) );

	return waitFailed;
}

long CThreadResource::Signal( unsigned long x_uTimeout )
{
	// Verify information
	if ( cInvalid() == m_hHandle || eRtInvalid == m_eType )
		return -1;

	// Get pointer to resource information
	SThreadResourceInfo *pRi = (SThreadResourceInfo*)m_hHandle;
	if ( !pRi )
	{	; //( 0, tcT( "Invalid resource info object pointer" ) );
		return -1;
	} // end if

	// Error code
	long nErr = 0;

	// Execute proper release function if valid handle
	if ( cInvalid() != m_hHandle )
		switch( m_eType )
	{
		case eRtInvalid :
			break;

		case eRtFile :
			break;

		case eRtSocket :
			break;

		case eRtThread :
			break;

		case eRtLock :
		case eRtMutex :
			return Wait( x_uTimeout );
			break;

		case eRtEvent :
		{
			// Auto reset releases only one thread
			if ( !pRi->bManualReset )
			{	pRi->bSignaled = 0;
				if ( !pthread_cond_signal( &pRi->hCond ) )
					return waitSuccess;
			} // end if

			// Manual reset releases all threads
			else
			{
				// Lock the mutex
				CScopeLock sl( pRi->cSync, x_uTimeout );
				if ( !sl.isLocked() )
					return waitTimeout;

				// Set signal
				pRi->bSignaled = 1;

				// Wake up waiting threads
				if ( !pthread_cond_broadcast( &pRi->hCond ) )
					return waitSuccess;

			} // end else

			return waitFailed;

		} break;

		default :
			; //( -1, tcT( "Attempt to signal unknown resource type" ) );
			break;

	} // end switch

	return nErr;
}

long CThreadResource::Reset( unsigned long x_uTimeout )
{
	// Verify information
	if ( cInvalid() == m_hHandle || eRtInvalid == m_eType )
		return -1;

	// Get pointer to resource information
	SThreadResourceInfo *pRi = (SThreadResourceInfo*)m_hHandle;
	if ( !pRi )
	{	; //( 0, tcT( "Invalid resource info object pointer" ) );
		return -1;
	} // end if

	// Error code
	long nErr = 0;

	// Execute proper release function if valid handle
	if ( cInvalid() != m_hHandle )
		switch( m_eType )
	{
		case eRtInvalid :
			break;

		case eRtFile :
			break;

		case eRtSocket :
			break;

		case eRtThread :
			break;

		case eRtLock :
		case eRtMutex :
		{
			// Attempt lock
			if ( pthread_mutex_trylock( &pRi->hMutex ) )
				return waitFailed;

			// Update lock count
			if ( pRi->nCount )
				pRi->nCount--;

			// Cleanup if count is zero
			if ( !pRi->nCount )
			{	pRi->uLastOwner = pRi->uOwner;
				pRi->uOwner = 0;
			} // end if

			// Unlock the mutex twice
			long nErr;
			if ( ( nErr = pthread_mutex_unlock( &pRi->hMutex ) )
			     || ( nErr = pthread_mutex_unlock( &pRi->hMutex ) ) )
			{	; //( nErr, tcT( "pthread_mutex_unlock() failed" ) );
				return waitFailed;
			} // end if

			return waitSuccess;

		} break;

		case eRtEvent :
		{
			pRi->bSignaled = 0;

		} break;

		default :
			; //( -1, tcT( "Attempt to reset unknown resource type" ) );
			break;

	} // end switch

	return nErr;
}

long CThreadResource::WaitMultiple( long x_nCount, CThreadResource **x_pResources, unsigned long x_uTimeout, long x_nMin )
{
	long nNumHandles = 0;
	long nNumSignaled = 0;
	long nRealIndex[ MAXIMUM_WAIT_OBJECTS ];
	long nSignaled[ MAXIMUM_WAIT_OBJECTS ];
	long nFailed[ MAXIMUM_WAIT_OBJECTS ];

	// Warn user if we surpass ms windows caps
	if ( x_nCount > MAXIMUM_WAIT_OBJECTS )
		; //( 0, tcT( "Number of handles specified is beyond MS Windows system capabilities!!!" ) );

	// Organize the wait handles
	for( long i = 0; i < x_nCount && nNumHandles < MAXIMUM_WAIT_OBJECTS; i++ )
	{
		// Valid?
		if ( !x_pResources[ i ] || !x_pResources[ i ]->isValid() )
			; //( 0, tcT( "Invalid handle specified to WaitMultiple()" ) );

		// Add to wait chain
		else
		{
			// Integrity check
			long res = x_pResources[ i ]->Wait( 0 );

			// Is it signaled?
			if ( waitSuccess == res )
			{
				// Enough to ditch?
				if ( ++nNumSignaled >= x_nMin )
					return i;

			} // end if

			// We'll have to wait on this one
			else if ( waitTimeout == res )
			{
				// Save info
				nFailed[ nNumHandles ] = 0;
				nSignaled[ nNumHandles ] = 0;
				nRealIndex[ nNumHandles++ ] = i;

			} // end if

			// ??? Must be in an unwaitable state?
			else
				; //( 0, tcT( "Unwaitable handle specified to WaitMultiple()" ) );

		} // end else

	} // end for

	unsigned long uDelay = 0, uRes = eWaitResolution / 1000;

	// Forever
	for( ; ; )
	{
		// See who's signaled
		for ( long i = 0; i < nNumHandles; i++ )
		{
			// Has this one already been signaled?
			if ( !nSignaled[ i ] && !nFailed[ i ] )
			{
				// Wait on this item
				long res = x_pResources[ nRealIndex[ i ] ]->Wait( uDelay );

				// Did it succeed?
				if ( waitSuccess == res )
				{
					// Do we have the required number of signals
					if ( ++nNumSignaled >= x_nMin )
						return nRealIndex[ i ];

					// Another one bites the dust
					nSignaled[ i ]++;

				} // end if

				// Failed?
				else if ( waitTimeout != res )
					nFailed[ i ] = 1;

				// Clear delay
				uDelay = 0;

			} // end for

		} // end for

		// ???
		if ( uDelay )
			return waitFailed;

		// Timed out?
		if ( x_uTimeout < uRes )
			return waitTimeout;

		// How long to wait
		uDelay = uRes;

		// Subtract from total
		x_uTimeout -= uDelay;

	} // end forever

	return waitTimeout;
}

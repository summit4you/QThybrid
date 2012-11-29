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

#include "windows.h"

CThreadResource::t_HANDLE CThreadResource::c_Invalid = (void*)~0;

unsigned long CThreadResource::c_Infinite = INFINITE;

/// Contains OS specific information about a resource
struct SThreadResourceInfo
{
	/// Reference count
	long								uRef;

	/// Resource handle
	HANDLE								hHandle;

	/// Owner
	void*								uOwner;

	/// Thread info
	void*								uThreadId;
	void*								pData;
	CThreadResource::PFN_ThreadProc		fnCallback;

	/// Lock info
	long								nCount;
};


/// Atomically increments the specified value
unsigned long CThread::Increment( long *p )
{
	return InterlockedIncrement( p );
}

/// Atomically decrements the specified value
unsigned long CThread::Decrement( long *p )
{
	return InterlockedDecrement( p );
}


//==============================================================
// CThread::CThreadProcImpl
//==============================================================
/// This is just a function stub that calls CThread::ThreadProc()
/**
	\param [in] x_pData		-	CThread class pointer

    I went ahead and left the signature as DWORD(*)(LPVOID);
    This means I had to do this stub since DWORD and LPVOID
    aren't in the header name space.

	\return Thread return value
*/
class CThreadResource::CThreadProcImpl
{
public:
    static DWORD WINAPI ThreadProc( LPVOID x_pData )
    {
		// Do the thread stuff
#if defined( _WIN64 )
		return (DWORD)(str::tc_uint64)CThreadResource::ThreadProc( x_pData );
#else
		return (DWORD)CThreadResource::ThreadProc( x_pData );
#endif
	}
};


static SThreadResourceInfo* CreateRi()
{
	SThreadResourceInfo *pRi = new SThreadResourceInfo();
	if ( !pRi )
		return tcNULL;

	pRi->uRef = 1;
	pRi->uOwner = 0;
	pRi->hHandle = INVALID_HANDLE_VALUE;
	pRi->uThreadId = 0;
	pRi->pData = 0;
	pRi->fnCallback = 0;
	pRi->nCount = 0;

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

	// Check for other references
	long lRef = CThread::Decrement( &x_pRi->uRef );
	if ( 0 < lRef )
		return lRef;

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

			// Wait to see if the thread will shutdown on it's own
			if ( WAIT_OBJECT_0 != WaitForSingleObject( x_pRi->hHandle, x_uTimeout ) )
			{
				if ( x_bForce )
				{
					; //( 0, tcT( "!!! Thread failed to exit gracefully, attempting to force an exception" ) );

					// Attempt exception injection
					if ( CThreadResource::InjectException( x_pRi->hHandle, -199 )
						 || WAIT_OBJECT_0 != WaitForSingleObject( x_pRi->hHandle, x_uTimeout ) )
						{
							; //( GetLastError(), tcT( "!!! Forced exception failed, Calling TerminateThread() !!!" ) );

							// Fine, do things the hard way
							TerminateThread( x_pRi->hHandle, false );

						} // end if

				} // end if

				else
					; //( GetLastError(), tcT( "!!! Thread failed to exit gracefully, it is being abandonded !!!" ) );

			} // end if

		case CThreadResource::eRtMutex :
		case CThreadResource::eRtEvent :
		case CThreadResource::eRtLock :
			CloseHandle( x_pRi->hHandle );
			break;

		default :
			; //( -1, tcT( "Attempt to release unknown resource type" ) );
			break;

	} // end switch

	// Drop the memory for real
	delete x_pRi;

	return 0;
}

bool CThreadResource::Init()
{
	CThreadResource::c_Invalid = (CThreadResource::t_HANDLE)-1;
	CThreadResource::c_Infinite = 0xffffffff;
	CThread::m_lThreadCount = 0;
	CThread::m_lRunningThreadCount = 0;

	return true;
}

void CThreadResource::UnInit()
{
}

void* CThreadResource::getThreadId()
{
	return (void*)::GetCurrentThreadId();
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
	m_hHandle = c_Invalid;
	m_eType = eRtInvalid;

	// Free the resource
	if ( 0 > FreeRi( pRi, eType, x_uTimeout, x_bForce ) )
	{	; //( 0, tcT( "Invalid resource info object pointer" ) );
		return -1;
	} // end if

	return 0;
}

volatile long s_last_error = 0;

static __declspec(noreturn) void ThrowException()
{
	// Throw exception
	throw( CThreadResource::CException( s_last_error ) );

	// Just in case
	ExitThread( s_last_error );

	// Forever
	for(;;) Sleep( 1000 );
}

void CThreadResource::InitException()
{
	volatile int i = 0;
	if ( i )
		throw( CThreadResource::CException() );
}

unsigned long CThreadResource::InjectException( void* hThread, long nError )
{
	unsigned long nRet = -1;

	// Save error code
	s_last_error = nError;

#if !defined( _WIN32_WCE )

	// Get handle
	HANDLE h = (HANDLE)hThread;
	if ( INVALID_HANDLE_VALUE == h )
		return -1;

	// Attemp to suspend the thread
	if ( INFINITE == SuspendThread( h ) )
		return -2;

	CONTEXT ctx;
	ctx.ContextFlags = CONTEXT_CONTROL;
	if ( GetThreadContext( h, &ctx ) )
	{
		// Jump into our Throw() function
#if !defined( _WIN64 )
		ctx.Eip = (DWORD) (DWORD_PTR) ThrowException;
#else
		ctx.Rip = (DWORD) (DWORD_PTR) ThrowException;
#endif

		if ( SetThreadContext( h, &ctx ) )
			nRet = 0;

		ResumeThread( h );

	} // end if

#endif

	return nRet;
}


void* CThreadResource::getOwner()
{
	SThreadResourceInfo *pRi = (SThreadResourceInfo*)m_hHandle;
	if ( !pRi )
		return 0;

	return pRi->uOwner;
}

long CThreadResource::NewEvent( const str::t_char8* x_sName, bool x_bManualReset, bool x_bInitialState )
{
	// Out with the old
	Destroy();

	SThreadResourceInfo *pRi = CreateRi();
	if ( !pRi )
		return -1; //( ERROR_OUTOFMEMORY, tcT( "Out of memory" ) );

	// Initialize member variables
	m_eType = eRtEvent;
	m_hHandle = (CThreadResource::t_HANDLE)pRi;

	// Create the event
	pRi->hHandle = CreateEvent( NULL, x_bManualReset, x_bInitialState, x_sName );
	if ( c_Invalid == pRi->hHandle )
		return -1;

	return 0;
}

long CThreadResource::NewMutex( const str::t_char8* x_sName, bool x_bInitialOwner )
{
	// Out with the old
	Destroy();

	SThreadResourceInfo *pRi = CreateRi();
	if ( !pRi )
		return -1; //( ERROR_OUTOFMEMORY, tcT( "Out of memory" ) );

	// Save information
	m_eType = eRtMutex;
	m_hHandle = (CThreadResource::t_HANDLE)pRi;

	// Create the mutex
	pRi->hHandle = CreateMutex( NULL, x_bInitialOwner, x_sName );
	if ( c_Invalid == pRi->hHandle )
		return -1;

	return 0;
}

long CThreadResource::NewThread( PFN_ThreadProc x_fnCallback, void* x_pData )
{
	// Out with the old
	Destroy();

	SThreadResourceInfo *pRi = CreateRi();
	if ( !pRi )
		return -1; //( ERROR_OUTOFMEMORY, tcT( "Out of memory" ) );

	m_eType = eRtThread;
	m_hHandle = (CThreadResource::t_HANDLE)pRi;

	// Save user data
	pRi->pData = x_pData;
	pRi->fnCallback = x_fnCallback;
	pRi->uOwner = getThreadId();

	// Create a thread
	pRi->hHandle = CreateThread(	(LPSECURITY_ATTRIBUTES)NULL,
									0,
									CThreadProcImpl::ThreadProc,
									(LPVOID)pRi,
									0,
									(LPDWORD)&pRi->uThreadId );

	// Seems to be confusion about what this function returns on error
	if ( NULL == pRi->hHandle || INVALID_HANDLE_VALUE == pRi->hHandle )
	{	Destroy(); return -1; }

	return 0;
}

void* CThreadResource::ThreadProc( void* x_pData )
{
	// Get pointer to resource information
	SThreadResourceInfo *pRi = (SThreadResourceInfo*)x_pData;
	if ( !pRi )
	{	; //( 0, tcT( "Invalid resource info object pointer" ) );
		return (void*)-1;
	} // end if

	// Initialize COM
#if defined( _WIN32_WCE )
//	CoInitializeEx( NULL, COINIT_MULTITHREADED );
#else
//	CoInitialize( NULL );
#endif

	// Call user thread
	void* pRet = pRi->fnCallback( pRi->pData );

	// Uninitialize COM
//	CoUninitialize();

	return pRet;
}

long CThreadResource::NewLock( const str::t_char8* x_sName, bool x_bInitialOwner )
{
	// Out with the old
	Destroy();

	SThreadResourceInfo *pRi = CreateRi();
	if ( !pRi )
		return -1; //( ERROR_OUTOFMEMORY, tcT( "Out of memory" ) );

	// Initialize member variables
	m_eType = eRtLock;
	m_hHandle = (CThreadResource::t_HANDLE)pRi;

	// Initialize structure
	pRi->nCount = 0;
	pRi->uOwner = 0;

	// Create the event
	pRi->hHandle = CreateMutex( NULL, x_bInitialOwner, x_sName );
	if ( c_Invalid == pRi->hHandle )
		return -1;

	return 0;
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
		case eRtEvent :
			return ( WAIT_OBJECT_0 == WaitForSingleObject( pRi->hHandle, x_uTimeout ) ) ? 0 : -1;

		case eRtMutex :
		case eRtLock :
			if ( WAIT_OBJECT_0 == WaitForSingleObject( pRi->hHandle, x_uTimeout ) )
			{	pRi->uOwner = getThreadId();
				return waitSuccess;
			} // end if

			return waitTimeout;
			break;

		default :
			; //( 0, tcT( "Attempt to wait on unknown resource type" ) );
			break;

	} // end switch

	return 0;
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

		case eRtMutex :
		case eRtLock :
			return Wait( x_uTimeout );
			break;

		case eRtEvent :
			return SetEvent( pRi->hHandle ) ? 0 : -1;
			break;

		default :
			; //( -1, tcT( "Attempt to wait on unknown resource type" ) );
			break;

	} // end switch

	return -1;
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

		case eRtMutex :
		case eRtLock :
			return ReleaseMutex( pRi->hHandle ) ? waitSuccess : waitFailed;
			break;

		case eRtEvent :
			return ResetEvent( pRi->hHandle ) ? waitSuccess : waitFailed;
			break;

		default :
			; //( -1, tcT( "Attempt to wait on unknown resource type" ) );
			break;

	} // end switch

	return -1;
}

long CThreadResource::WaitMultiple( long x_nCount, CThreadResource **x_pResources, unsigned long x_uTimeout, long x_nMin )
{
	long nNumHandles = 0;
	HANDLE hHandles[ MAXIMUM_WAIT_OBJECTS ];
	long nRealIndex[ MAXIMUM_WAIT_OBJECTS ];

	if ( x_nCount > MAXIMUM_WAIT_OBJECTS )
		; //( 0, tcT( "Number of handles specified is beyond system capabilities!!!" ) );

	// Loop through the handles
	for( long i = 0; i < x_nCount && nNumHandles < MAXIMUM_WAIT_OBJECTS; i++ )
	{
		if ( !x_pResources[ i ]->isValid() )
			; //( 0, tcT( "Invalid handle specified to WaitMultiple()" ) );
		else
		{
			SThreadResourceInfo *pRi = (SThreadResourceInfo*)x_pResources[ i ]->getHandle();

			// +++ Should this be INVALID_HANDLE_VALUE?
			if ( !pRi )
				; //( 0, tcT( "Invalid data pointer" ) );

			else
				hHandles[ nNumHandles ] = pRi->hHandle, nRealIndex[ nNumHandles++ ] = i;

		} // end else

	} // end for

	// Wait for signal or timeout
	long nRet = WaitForMultipleObjects( nNumHandles, hHandles, 0 != x_nMin, x_uTimeout );

	// Error?
	if ( 0 > nRet )
		return nRet;

	// Index out of range?
	else if ( nRet >= nNumHandles )
		return -1;

	// Return the index of the signaled object
	return nRealIndex[ nRet ];
}

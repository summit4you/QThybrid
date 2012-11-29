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
#	include "threads-windows.hpp"
#else
#	include "threads-posix.hpp"
#endif

CThread::CThread()
{
    m_pData = 0;
	m_lStopFlag = 0;
	m_bInitStatus = false;
}

CThread::~CThread()
{
	Stop();
}

unsigned long CThread::InjectException( long nCode )
{
	// Ensure thread is running
	if ( !isRunning() )
		return -1;

	// Attempt to inject an exception into the thread
	return CThreadResource::InjectException( getThreadId(), nCode );
}

void* CThread::ThreadProc( void* x_pData )
{
    // Get pointer
    CThread *pThread = (CThread*)x_pData;
    if ( !x_pData )
        return (void*)-1111;

	// Bad init value
    long nRet = -2222;

    // Count one thread
    CThread::IncThreadCount();
    CThread::IncRunningThreadCount();

    // Get user params
    long nSleep = 0;
    void* pData = pThread->getUserData();

	try
	{
		// Initialize exception injection
		InitException();

		// Initialize thread
		pThread->m_bInitStatus = pThread->InitThread( pData );

	} // end try

	catch( ... )
	{
		pThread->m_bInitStatus = 0;

	} // end catch

	// Signal that we're initialized
	pThread->m_evInit.Signal();

	// Was initialization a success?
	if ( pThread->m_bInitStatus )
	{
		// Loop while we're not supposed to stop
		bool nDone = false;
		while ( !nDone )
		{
			try
			{
				// Init our exception injector
				InitException();

				// Call do thread
				nSleep = pThread->DoThread( pData );

			} // end try

			catch( ... )
			{
				// +++ Should probably report error here

			} // end catch

			// Check for stop signal
			if ( 0 > nSleep 
				 || *pThread->getStopFlag()
				 || !pThread->m_evStop.Wait( nSleep ) )
				nDone = true;

		} // end while

		try
		{
			// Init our exception injector
			InitException();

			// Kill the thread
			nRet = pThread->EndThread( pData );

		} // end try

		catch( ... )
		{
			pThread->m_bInitStatus = 0;

		} // end catch

	} // end if

    /// Decrement the running thread count
    CThread::DecRunningThreadCount();

    return (void*)nRet;

}

long CThread::Start( void* x_pData )
{
    // Are we already running?
    if ( isRunning() )
        return 0;

    // Save users data
    m_pData = x_pData;

    // Give the thread a fighting chance
    m_evStop.Reset();
    m_evInit.Reset();
	
	// Clear stop flag
	m_lStopFlag = 0;

	// Attempt to create the thread
	if ( CThreadResource::NewThread( CThread::ThreadProc, this ) )
		return -1;

    return 0;
}

long CThread::Stop( unsigned long x_uWait, bool x_bKill )
{
	// Set the stop flag
	m_lStopFlag = 1;

	// Signal that the thread should exit
	m_evStop.Signal();

	// Ensure thread resource is valid
	if ( !CThreadResource::isValid() )
		return 0;

	// Wait for thread to stop
	if ( !WaitThreadExit( x_uWait ) )
		return -1;

	// Kill the thread
	long nErr = CThreadResource::Destroy( x_uWait, x_bKill );

    // Clear thread data
    m_pData = 0;
	m_bInitStatus = false;

    return nErr;
}

bool CThread::isRunning()
{
	// Ensure thread resource is valid
	if ( !CThreadResource::isValid() )
		return false;

	// Has the thread exited?
	return !WaitThreadExit( 0 );
}

bool CThread::WaitThreadExit( unsigned long x_uTimeout )
{
	// Ensure valid thread handle
	if ( !CThreadResource::isValid() )
		return true;

	// See if the thread is still alive
	if ( CThreadResource::waitSuccess != CThreadResource::Wait( x_uTimeout ) )
		return false;

	return true;
}

// The number of threads running
long CThread::m_lThreadCount = 0;
long CThread::m_lRunningThreadCount = 0;

long CThread::getThreadCount()
{   return m_lThreadCount; }

long CThread::getRunningThreadCount()
{   return m_lRunningThreadCount; }

void CThread::IncThreadCount()
{   m_lThreadCount++; }

void CThread::IncRunningThreadCount()
{   m_lRunningThreadCount++; }

void CThread::DecRunningThreadCount()
{   if ( m_lRunningThreadCount )
        m_lRunningThreadCount--;
}

bool CSignal::Create( const str::t_char8* x_pName )
{
	// Named signal?
	if ( x_pName )
	{	return 0 == NewEvent( ( str::t_string8( x_pName ) + tcT( "_sig" ) ).c_str(), true, false )
			   && 0 == m_nosig.NewEvent( ( str::t_string8( x_pName ) + tcT( "_nosig" ) ).c_str(), true, false );
	} // end if

	// No name
	return 0 == NewEvent( tcNULL, true, false )
		   && 0 == m_nosig.NewEvent( tcNULL, true, true );
}


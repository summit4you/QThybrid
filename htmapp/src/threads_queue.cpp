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

namespace tq
{
	t_pb *g_htmapp_pb = 0;

	CLock *g_htmapp_pb_lock = 0;

	t_waitpool *g_htmapp_wp = 0;

	CThreadPool *g_htmapp_tp = 0;

	void init( bool bEnableThreadPool )
	{
		/// Initialize threads resources
		CThreadResource::Init();
	
		// Property bag
		if ( !g_htmapp_pb )
			g_htmapp_pb = new t_pb;
			
		// Lock global property bag
		if ( !g_htmapp_pb_lock )
			g_htmapp_pb_lock = new CLock;
			
		// Wait pool
		if ( !g_htmapp_wp )
			g_htmapp_wp = new t_waitpool;
			
		// Thread pool
		if ( bEnableThreadPool && !g_htmapp_tp )
			g_htmapp_tp = new CThreadPool;
	}

	void uninit()
	{
		if ( g_htmapp_pb_lock )
		{
			CScopeLock sl( g_htmapp_pb_lock );
			if ( sl.isLocked() )
			{
				// Kill the thread pool
				if ( g_htmapp_tp )
				{	g_htmapp_tp->Stop();
					delete g_htmapp_tp;
					g_htmapp_tp = 0;
				} // end if

				// Property bag
				if ( g_htmapp_pb )
					delete g_htmapp_pb, g_htmapp_pb = 0;

				// Wait pool
				if ( g_htmapp_wp )
					delete g_htmapp_wp, g_htmapp_wp = 0;

			} // end if

		} // end if

		// Lose the thread lock
		if ( g_htmapp_pb_lock )
			delete g_htmapp_pb_lock, g_htmapp_pb_lock = 0;

		/// Release thread resources
		CThreadResource::UnInit();
	}

	t_pb* pb() 
	{ 
		return g_htmapp_pb; 
	}

	CLock* lock() 
	{ 
		return g_htmapp_pb_lock; 
	}

	bool set( const str::t_string &sKey, const t_pb &pbValue, const str::t_string &sep )
	{
		if ( !g_htmapp_pb_lock || !g_htmapp_pb )
			return false;

		CScopeLock sl( g_htmapp_pb_lock );
		if ( !sl.isLocked() )
			return false;

		if ( sep.length() )
			g_htmapp_pb->at( sKey, sep ) = pbValue;
		else
			(*g_htmapp_pb)[ sKey ] = pbValue;

		if ( g_htmapp_wp )
		{
			// See if someone was waiting for a change
			t_waitpool::iterator it = g_htmapp_wp->find( sKey );
			if ( g_htmapp_wp->end() != it )
				it->second.Signal();

		} // end if

		return true;
	}

	t_pb swp( const str::t_string &sKey, const t_pb &pbValue, const str::t_string &sep )
	{
		if ( !g_htmapp_pb_lock || !g_htmapp_pb )
			return t_pb();

		CScopeLock sl( g_htmapp_pb_lock );
		if ( !sl.isLocked() )
			return t_pb();

		t_pb ret;
		if ( sep.length() )
			ret = g_htmapp_pb->at( sKey, sep ),
			g_htmapp_pb->at( sKey, sep ) = pbValue;
		else
			ret = (*g_htmapp_pb)[ sKey ],
			(*g_htmapp_pb)[ sKey ] = pbValue;

		if ( g_htmapp_wp )
		{
			// See if someone was waiting for a change
			t_waitpool::iterator it = g_htmapp_wp->find( sKey );
			if ( g_htmapp_wp->end() != it )
				it->second.Signal();

		} // end if

		return ret;
	}

	t_pb get( const str::t_string &sKey, const str::t_string &sep )
	{
		if ( !g_htmapp_pb_lock || !g_htmapp_pb )
			return t_pb();

		CScopeLock sl( g_htmapp_pb_lock );
		if ( !sl.isLocked() )
			return t_pb();

		if ( sep.length() )
			return g_htmapp_pb->at( sKey, sep );

		return (*g_htmapp_pb)[ sKey ];
	}
	
	bool wait( const str::t_string &sKey, unsigned long uTimeout )
	{
		if ( !g_htmapp_pb_lock || !g_htmapp_pb || !g_htmapp_wp )
			return false;

		CEvent *w = tcNULL;
		
		// Sanity check
		if ( !sKey.length() || 0 >= uTimeout )
			return false;
		
		{ // Scope lock
		
			// Add wait
			CScopeLock sl( g_htmapp_pb_lock );
			if ( !sl.isLocked() )
				return false;

			t_waitpool::iterator it = g_htmapp_wp->find( sKey );
			if ( g_htmapp_wp->end() == it )
				w = &(*g_htmapp_wp)[ sKey ];
			else
				w = &it->second, w->AddRef();
				
		} // end scope lock
		
		if ( !w )
			return false;
		
		bool bRet = w->Wait( uTimeout );
		
		{ // Scope lock
		
			// Acquire lock
			CScopeLock sl( g_htmapp_pb_lock );
			if ( !sl.isLocked() )
				return false;

			// Remove wait
			t_waitpool::iterator it = g_htmapp_wp->find( sKey );
			if ( g_htmapp_wp->end() != it )
				if ( 0 <= it->second.Destroy() )
					g_htmapp_wp->erase( it );
				
		} // end scope lock

		return bRet;
	}
	

//------------------------------------------------------------------

	long start( const str::t_string &s, t_tqfunc f, void *p )
	{	if ( !g_htmapp_tp )
			return 0;
		return g_htmapp_tp->start( s, f, p ); 
	}

	bool stop( const str::t_string &s )
	{	if ( !g_htmapp_tp )
			return 0;
		return g_htmapp_tp->stop( s ); 
	}
	
	bool stop( long id )
	{	if ( !g_htmapp_tp )
			return 0;
		return g_htmapp_tp->stop( id ); 
	}

	CWorkerThread::CWorkerThread() 
	{
		m_f = 0; 
		m_p = 0;
	}

	CWorkerThread::~CWorkerThread() 
	{	Stop();
		m_f = 0; 
		m_p = 0;
	}

	void CWorkerThread::Run( const str::t_string &s, t_tqfunc f, void *p, bool bStart ) 
	{
		m_f = f;
		m_p = p;
		m_s = s;
		if ( bStart ) 
			Start(); 
	}
	long CWorkerThread::DoThread( void* x_pData ) 
	{
		if ( m_f ) 
			return m_f( this, m_p ); 

		return -1; 
	}
	

	CThreadPool::CThreadPool()
	{	m_id = 0; 
		m_enable = true; 
		Start();
	}

	CThreadPool::~CThreadPool()
	{	m_enable = false;
		Stop(); 
	}

	long CThreadPool::DoThread( void* x_pData )
	{
		// While not stopped
		getStopEvent().Wait( m_event, 100 );
		
		// Command waiting?
		if ( !m_event.Wait( 0 ) )
		{
			// Function
			long id = -1;
			SCmdInfo ci;

			{ // Scope lock
			
				// Lock the command list
				CScopeLock sl( m_lock );
				if ( sl.isLocked() )
				{
					// Grab the first command in the list
					t_threadcmds::iterator it = m_cmds.begin();
					if ( m_cmds.end() != it )
					{	id = m_cmds.begin()->first;
						ci = m_cmds.begin()->second;
						m_cmds.erase( m_cmds.begin() );
					} // end if

					// Reset the event
					else
						m_event.Reset();

				} // end scope locked

			} // end scope lock

			// Command?
			if ( 0 <= id )
			{
				// New thread?
				if ( ci.f )
				{	CWorkerThread &r = m_tp[ id ];
					r.Run( ci.s, ci.f, ci.p, true );
				} // end if

				// Kill thread if that id exists
				else 
				{	t_threadpool::iterator it = m_tp.find( id );
					if ( m_tp.end() != it )
						m_tp.erase( it );
				} // end else

			} // end if

		} // end while

		// Cleanup expired thread objects
		for( t_threadpool::iterator it = m_tp.begin(); m_tp.end() != it; )
			if ( !it->second.getInitEvent().Wait( 0 ) && !it->second.isRunning() )
				m_tp.erase( it++ );
			else
				it++;

		return 0;
	}

	long CThreadPool::EndThread( void* x_pData )
	{
		// Delete all threads
		m_tp.clear();

		return 0;
	}

	long CThreadPool::start( const str::t_string &s, t_tqfunc f, void *p )
	{
		if ( !m_enable )
			return -1;

		CScopeLock sl( m_lock );
		if ( !sl.isLocked() )
			return -1;

		// Create a new thread object and start it
		long id = m_id++;

		// Will the thread be named?
		if ( s.length() )
		{
			// See if we already have a thread by that name
			t_threadnames::iterator it = m_names.find( s );
			if ( m_names.end() != it )
				return -1;

			// Save the thread name
			m_names[ s ] = id;

		} // end if

		// Send start thread command
		m_cmds[ id ] = SCmdInfo( s, f, p );
		
		// New command waiting
		m_event.Signal();

		// Return the thread id
		return id;
	}

	bool CThreadPool::stop( const str::t_string &s )
	{
		if ( !m_enable || !s.length() )
			return false;

		CScopeLock sl( m_lock );
		if ( !sl.isLocked() )
			return false;

		// See if we have a thread by that name
		t_threadnames::iterator it = m_names.find( s );
		if ( m_names.end() == it )
			return -1;

		// Erase name mapping
		m_names.erase( it );
			
		// Ensure valid id
		if ( it->second >= m_id )
			return false;

		// Stop signal
		m_cmds[ it->second ] = SCmdInfo();

		// New command waiting
		m_event.Signal();

		return true;
	}

	bool CThreadPool::stop( long id )
	{
		if ( !m_enable )
			return false;

		CScopeLock sl( m_lock );
		if ( !sl.isLocked() )
			return false;

		// Do we have such a thread?
		if ( id >= m_id )
			return false;

		// Stop signal
		m_cmds[ id ] = SCmdInfo();

		// New command waiting
		m_event.Signal();

		return true;
	}

}; // namespace tq

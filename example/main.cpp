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

#include "frwk.h"
#include "htmapp_resources.h"
#include "network.h"
#include "web_page.h"
#include "mainwindow.h"

int main( int argc, char* argv[] )
{
	// Initialize resources
	HTMAPP_INIT_RESOURCES();

	// Initialize thread queue
	tq::init( true );

	// Initialize application object
	QApplication app( argc, argv );

	// Create main window
	CMainWindow *pWindow = new CMainWindow;
	if ( !pWindow )
		return -1;

#if defined( CII_PROJECT_NAME )
	pWindow->setName( CII_PROJECT_NAME );
#endif	

#if defined( CII_PROJECT_DESC )
	pWindow->setDescription( CII_PROJECT_DESC );
#endif	

	// Make the command line accessible
	tq::set( "cmdline", TCmdLine< str::t_string >( argc, argv ).pb() );

	// Make the makefile parameters accessible
#if defined( CII_PARAMS )
	tq::set( "ciid", parser::DecodeJson< t_pb >( CII_PARAMS ) );
#endif
	
	// Initialize the window
	pWindow->Init();

	// Show the window
	pWindow->show();

	// Run the app
	int ret = app.exec();

	// Uninitialize thread queue
	tq::uninit();

	return ret;
}


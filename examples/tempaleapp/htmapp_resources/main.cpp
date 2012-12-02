#include "frwk.h"
#include "htmapp_resources.h"
#include "network.h"
#include "web_page.h"
#include "mainwindow.h"
#include <Windows.h>

int main( int argc, char* argv[] )
{

	HTMAPP_INIT_RESOURCES();

	// Initialize thread queue
	tq::init( true );

	// Initialize application object
	QApplication app( argc, argv );

	// Create main window
	CMainWindow *pWindow = new CMainWindow;
	if ( !pWindow )
		return -1;

	pWindow->setHomeUrl("http://embedded/index.csp");

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


#include "htmapp.h"

static int _internal_run( TPropertyBag< str::t_string8 > &in, TPropertyBag< str::t_string8 > &out )
{


	t_pb8 pb;
	pb[ "map" ] = tq::get( "indexer.out", "." );
	pb[ "drive" ] = tq::get( "indexer.job.drive", "." );
	pb[ "drive" ][ "progress" ] = tq::get( "indexer.progress", "." ).ToInt64();
		
	// Send back indexer status
	out << parser::EncodeJson( pb );

	// Set the center folder
	tq::set( "indexer.center", in[ "GET" ][ "center" ], "." );

	// Set the max top
	tq::set( "indexer.top", cmn::Range( in[ "GET" ][ "top" ].ToInt(), 1, 25 ), "." );
	
	// Set the max depth
	tq::set( "indexer.depth", cmn::Range( in[ "GET" ][ "depth" ].ToInt(), 1, 5 ), "." );
	
	// Request another update from the indexer thread
	tq::set( "indexer.update", 1, "." );
	
	return 0;
;

	out << "\n";

	return 0;
}


void * f_T1354214889I3 = (void*)&_internal_run;

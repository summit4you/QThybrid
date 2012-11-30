
#include "htmapp.h"


	
#include "index_thread.h"

static int _internal_run( TPropertyBag< str::t_string8 > &in, TPropertyBag< str::t_string8 > &out )
{


	// Start the indexer thread if we got root
	str::t_string sRoot = in[ "GET" ][ "root" ].str();
	
	if ( sRoot.length() )
	{
		// Ensure valid drive
		t_pb job;
		if ( disk::GetDiskInfo( job[ "drive" ], sRoot ) )
		{	
			// Set the job information
			job[ "params" ] = in[ "GET" ];
			
			// Run the new job
			job[ "run" ] = 1;
			
			// Cancel any previous job
			job[ "cancel" ] = 1;

			// Set the job info
			tq::set( "indexer.job", job, "." );
			
			// Ensure indexer thread is running
			tq::start( "indexer", index_thread );

		} // end if

	} // end if

;

	out << "\n<body>\n    <div id=\"chart\"></div>\n\t<div class='rightoptions'>\n\t\t<label for='maxtop'>Top</label>\n\t\t<select id='maxtop' class='seloption'>\n\t\t\t";

				for( long i = 5; i <= 10; i++ )
					out << "<option value='" 
						<< str::ToString< str::t_string8 >( i ) 
						<< ( ( i == 5 ) ? "' selected>" : "'>" )
						<< str::ToString< str::t_string8 >( i ) 
						<< "</option>";
			;

	out << "\n\t\t</select>\n\t\t<label for='maxdepth'>Depth</label>\n\t\t<select id='maxdepth' class='seloption'>\n\t\t\t";

				for( long i = 1; i <= 5; i++ )
					out << "<option value='" 
						<< str::ToString< str::t_string8 >( i ) 
						<< ( ( i == 3 ) ? "' selected>" : "'>" )
						<< str::ToString< str::t_string8 >( i ) 
						<< "</option>";
			;

	out << "\n\t\t</select>\n\t</div>\n\t<script type=\"text/javascript\">\n\n\t\tvar w = 640,\n\t\t\th = 480,\n\t\t\tr = Math.min(w, h) / 2,\n\t\t\tcolor = d3.scale.category20c();\n\n\t\tvar vis = d3.select(\"#chart\").append(\"svg\")\n\t\t\t.attr(\"width\", w)\n\t\t\t.attr(\"height\", h)\n\t\t\t.append(\"g\")\n\t\t\t.attr(\"transform\", \"translate(\" + w / 2 + \",\" + h / 2 + \")\");\n\n\t\tvar partition = d3.layout.partition()\n\t\t\t.sort(null)\n\t\t\t.size([2 * Math.PI, r * r])\n\t\t\t.value(function(d) { return d.size; });\n\n\t\tvar arc = d3.svg.arc()\n\t\t\t.startAngle(function(d) { return d.x; })\n\t\t\t.endAngle(function(d) { return d.x + d.dx; })\n\t\t\t.innerRadius(function(d) { return Math.sqrt(d.y); })\n\t\t\t.outerRadius(function(d) { return Math.sqrt(d.y + d.dy); });\n\n\t\tfunction notzero( a )\n\t\t{\tfor ( var i in a )\n\t\t\t\treturn true;\n\t\t\treturn false;\n\t\t}\n\n\t\tfunction poll_data()\n\t\t{\n\t\t\tif ( getPage() != 'graph' )\n\t\t\t\treturn;\n\t\t\n\t\t\t$.ajax({ url: \"data.htm\", \n\t\t\t\t\t dataType: \"json\",\n\t\t\t\t\t data:  { \t\n\t\t\t\t\t\t\t\tcenter: $('.centeritem').html(),\n\t\t\t\t\t\t\t\ttop:\t$('#maxtop').val(),\n\t\t\t\t\t\t\t\tdepth:\t$('#maxdepth').val()\n\t\t\t\t\t\t\t}\n\t\t\t\t\t\t\t,\n\t\t\t\t\t success: function(json) \n\t\t\t\t\t {\t\n\t\t\t\t\t\tif ( json.drive )\n\t\t\t\t\t\t\tsetProgress( '#diskprogress', json.drive.progress, json.drive.bytes_used );\n\t\t\t\t\t \n\t\t\t\t\t\tif ( json.map && notzero( json.map ) )\n\t\t\t\t\t\t{\n\t\t\t\t\t\t\tvar path = vis\n\t\t\t\t\t\t\t   .data([json.map])\n\t\t\t\t\t\t\t   .selectAll(\"path\")\n\t\t\t\t\t\t\t   .data(partition.nodes)\n\t\t\t\t\t\t\t   .attr(\"d\", arc)\n\t\t\t\t\t\t\t   .style(\"stroke\", \"#222\")\n\t\t\t\t\t\t\t   .style(\"fill\", function(d) \n\t\t\t\t\t\t\t   { \tif ( d.path == $('.highlight').html() )\n\t\t\t\t\t\t\t\t\t{\t$('.highlight').html( d.path );\n\t\t\t\t\t\t\t\t\t\t$('.hdr_size').html( d.szstr );\n\t\t\t\t\t\t\t\t\t\treturn d.hi_colour;\n\t\t\t\t\t\t\t\t\t} // end if\n\t\t\t\t\t\t\t\t\telse\n\t\t\t\t\t\t\t\t\t\treturn d.colour; \n\t\t\t\t\t\t\t   })\n\t\t\t\t\t\t\t\t;\n\n\t\t\t\t\t\t\tpath\n\t\t\t\t\t\t\t   .enter().append(\"path\")\n//\t\t\t\t\t\t\t   .attr(\"display\", function(d) { return d.depth ? null : \"none\"; })\n\t\t\t\t\t\t\t   .attr(\"d\", arc)\n\t\t\t\t\t\t\t   .style(\"stroke\", \"#222\")\n\t\t\t\t\t\t\t   .style(\"fill\", function(d) { return d.colour; })\n\t\t\t\t\t\t\t   .on(\"mouseover\", function(d)\n\t\t\t\t\t\t\t   {\t$('.highlight').html( d.path );\n\t\t\t\t\t\t\t\t\t$('.hdr_size').html( d.szstr );\n\t\t\t\t\t\t\t\t\td3.select(this).style(\"fill\", d.hi_colour);\n\t\t\t\t\t\t\t   })\n\t\t\t\t\t\t\t   .on(\"mouseout\", function(d)\n\t\t\t\t\t\t\t   {\t$('.selitem').html( '' );\n\t\t\t\t\t\t\t\t\td3.select(this).style(\"fill\", d.colour);\n\t\t\t\t\t\t\t   })\n\t\t\t\t\t\t\t   .on(\"mousedown\", function(d)\n\t\t\t\t\t\t\t   {\tif ( d.dir )\n\t\t\t\t\t\t\t   \t\t\t$('.centeritem').html( d.dst ? d.dst : ( d.depth ? d.path : '' ) ),\n\t\t\t\t\t\t\t\t\t\t$('.highlight').html( d.dst ? d.dst : ( d.depth ? d.path : '' ) );\n\t\t\t\t\t\t\t   })\n\t\t\t\t\t\t\t   ;\n\n\t\t\t\t\t\t\tpath\n\t\t\t\t\t\t\t\t.exit()\n\t\t\t\t\t\t\t\t.remove()\n\t\t\t\t\t\t\t\t;\n\n\t\t\t\t\t\t} // end if\n\n\t\t\t\t\t\tsetTimeout( poll_data, 500 );\n\t\t\t\t\t },\n\t\t\t\t\t error: function(xhr,opt,msg) { setTimeout( poll_data, 1000 ); },\n\t\t\t\t});\n\t\t}\n\n\t\tpoll_data();\n\n\t</script>\n</body>\n\n";

	return 0;
}


void * f_T1354214889I5 = (void*)&_internal_run;

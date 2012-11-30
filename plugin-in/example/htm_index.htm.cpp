
#include "htmapp.h"

static int _internal_run( TPropertyBag< str::t_string8 > &in, TPropertyBag< str::t_string8 > &out )
{

	out << "<!DOCTYPE html>\n<html>\n<head>\n\t<title>";
 out << "DiskTest" ;

	out << "</title>\n\n\t<meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\">\n\t<meta http-equiv=\"Access-Control-Allow-Origin\" content=\"*\">\n\t<meta http-equiv=\"Access-Control-Allow-Methods\" content=\"POST GET OPTIONS\">\n\n\t<link type=\"text/css\" rel=\"stylesheet\" href=\"css/main.css\"/>\n\t<link type=\"text/css\" rel=\"stylesheet\" href=\"css/button.css\"/>\n\n\t<script type=\"text/javascript\" src=\"js/jquery.min.js\"></script>\n\t<!--<script type=\"text/javascript\" src=\"js/jquery.sparkline.min.js\"></script>-->\n\t<script type=\"text/javascript\" src=\"js/jquery.quicksand.js\"></script>\n\t<script type=\"text/javascript\" src=\"js/d3/d3.min.js\"></script>\n\t<script type=\"text/javascript\" src=\"js/d3/d3.layout.min.js\"></script>\n\n</head>\n<body>\n \n\t<div class='hdr_size hdr_text bdr_gray'></div>\n\t<div class='hdr_path hdr_text bdr_gray'></div>\n\t<div id='diskprogress' class='hdr_progress hidden'><div class='progress_a'></div><div class='progress_b'></div></div>\n    \n\t<div class='banner'>\n\t\t<img class='backbtn transparent' src='imgs/back.png' onclick='selPage( \"drives\" )'>\n\t\t<div class='banner_logo' onclick='selPage(\"about\")'><a href='javascript:' onclick='selPage(\"about\")'><img src='imgs/dsa.png'></a></div>\n\t</div>\n\n\t<div class='highlight hdr_text'></div>\n\t<div class='highlightdrive'></div>\n\t<div class='centeritem hidden'></div>\n\t<div class='rightdrive'></div>\n\t\n\t<div id='drives' class='page'></div>\n\t<div id='graph' class='page hidden'></div>\n\t<div id='about' class='page hidden'></div>\n\n\t<script type='text/javascript'>\n\n\t\t$( $('#drives').load( 'drives.htm' ) );\n\n\t\tvar g_sel = '';\n\t\tfunction getPage() { return g_sel; }\n\t\t\n\t\tfunction showPage( id, page, dir )\n\t\t{\n\t\t\tif ( page == id )\n\t\t\t\t$('#'+id).load( id+'.htm?root=' + escape( $('.highlight').html() ) ),\n\t\t\t\t$('#'+id).hide().animate( { left: dir }, 0 ),\n\t\t\t\t$('#'+id).show().animate( { left:'20px' }, 500 );\n\t\t\telse\n\t\t\t\t$('#'+id).animate( { left: dir }, 500 ),\n\t\t\t\tsetTimeout( function() { $('#'+id).hide(); }, 500 );\n\t\t}\n\t\t\n\t\t/// Selects the active page\n\t\tfunction selPage( page )\n\t\t{\tif ( g_sel == page ) return;\n\t\t\tg_sel = page;\n\t\t\tshowPage( \"drives\", page, -1000 );\n\t\t\tshowPage( \"graph\", page, 1000 );\n\t\t\tshowPage( \"about\", page, 1000 );\n\t\t\tswitch ( page )\n\t\t\t{\n\t\t\t\tcase \"about\" :\n\t\t\t\t\t$('.backbtn').animate( { opacity: 1 }, 500 );\n\t\t\t\t\tbreak;\n\t\t\t\t\n\t\t\t\tcase \"graph\" :\n\t\t\t\t\t$('.banner_logo').animate( { opacity: 0 }, 500 );\n\t\t\t\t\t$('.backbtn').animate( { opacity: 1 }, 500 );\n\t\t\t\t\t$('.hdr_size').animate( { opacity: 1 }, 500 );\n\t\t\t\t\t$('.backbtn').animate( { opacity: 1 }, 500 );\n\t\t\t\t\tsetTimeout( function() { $('.banner_logo').hide(); }, 500 );\n\t\t\t\t\tbreak;\n\t\t\t\t\t\n\t\t\t\tdefault:\n\t\t\t\t\tsetProgress( '#diskprogress', 0, 0 ),\n\t\t\t\t\t$('.banner_logo').show().animate( { opacity: 1 }, 500 );\n\t\t\t\t\t$('.hdr_size').animate( { opacity: 0 }, 500 );\n\t\t\t\t\t$('.highlight').animate( { opacity: 0 }, 500 );\n\t\t\t\t\t$('.backbtn').animate( { opacity: 0 }, 500 );\n\t\t\t\t\t$('.highlightdrive').animate( { opacity: 0 }, 500 );\n\t\t\t\t\t\n\t\t\t} // end switch;\n\t\t}\n\n\t\t/// Updates the progress bar\n\t\tfunction setProgress( container, progress, total )\n\t\t{\n\t\t\tvar pa = ( 0 < total ) ? parseInt( progress * 100 / total ) : 0;\n\t\t\tvar pb = ( 0 < total ) ? parseInt( ( total - progress ) * 100 / total ) : 0;\n\t\t\tvar w = $(container).width() - 20;\n\t\t\tvar h = $(container).height();\n\t\t\t\n\t\t\t// 0 turns progress bar off\n\t\t\tif ( 0 >= w || 0 >= h || 0 >= total || 0 > progress )\n\t\t\t{\t$(container).addClass( 'hidden' );\n\t\t\t\treturn;\n\t\t\t} // end if\n\t\t\t\n\t\t\t// Clip at 1% - 100%\n\t\t\tif ( 99 < pa )\n\t\t\t\tpa = 99, pb = 1;\n\t\t\tif ( 0 >= pa )\n\t\t\t\tpa = 1, pb = 99;\n\t\t\t\t\t\n\t\t\t$(container).removeClass( 'hidden' );\n\t\t\t\n\t\t\t$(container + ' .progress_a').height( h );\n\t\t\t$(container + ' .progress_a').width( parseInt( pa * w / 100 ) + 10 );\n\t\t\t$(container + ' .progress_b').height( h );\n\t\t\t$(container + ' .progress_b').width( parseInt( pb * w / 100 ) + 10 );\n\t\t}\n\n\t</script>\n  </body>\n</html>";

	return 0;
}


void * f_T1354214889I15 = (void*)&_internal_run;

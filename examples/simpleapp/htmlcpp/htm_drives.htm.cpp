
#include "htmapp.h"


	
	// Make single drive item
	str::t_string mkitem( const str::t_string &grp, const str::t_string &path, const str::t_string &type, 
						  str::t_string id, const str::t_string &used, const str::t_string &total )
	{
		return str::t_string() 
				+ "<li class='driveitem cid_" + grp + "_" + id +" drive-item' value='" + path + "' data-id='did_" + id + "' onclick='selDrive( \"cid_" + grp + "_" + id +"\" )'>" 
				+ "<div class='pathdrive'><img class='driveimg' src='imgs/nes/drive-" + type + ".png'></div>"
				+ "<div class='pathstr'>" + path + "</div>"
				+ "<div class='pid_" + grp + "_" + id +" progress'><div class='progress_a'></div><div class='progress_b'></div></div>"
				+ "<script language='javascript'>$( setProgress( \".pid_" + grp + "_" + id +"\", " + used + ", " + total + " ) )</script>"
				+ "</li>\n";
	}
	
	// Make buttons and drive lists
	void listitems( t_pb &di, const str::t_string &sName, const str::t_string &sType,
					str::t_string &btns, str::t_string &lists, str::t_string &last )
	{
		int i = 0;
		str::t_string ret;
		
		for( t_pb::iterator it = di.begin(); it != di.end(); it++, i++ )
			if ( sType == "all" || (*it->second)[ tcT( "drive_type" ) ].ToString() == sType )
				if ( it->second->get( "bytes" ).ToDouble() )
					ret += mkitem( sType, it->first, it->second->get( "drive_type" ).str(), 
								   str::ToString< str::t_string>( i ),
								   it->second->get( "bytes_used" ).str(),
								   it->second->get( "bytes" ).str() );
		if ( !ret.length() )
			return;
			
		last = sType;
			
		lists += str::t_string() + "<ul class='list_" + sType + " hidden'>" + ret + "</ul>";
		
		btns += "<button id='btn_" + sType + "' onclick='switch_list(\"" + sType + "\")' class='" 
				+ ( !btns.length() ? "first" : "" ) + "'>" + sName + "</button>";
	}

static int _internal_run( TPropertyBag< str::t_string8 > &in, TPropertyBag< str::t_string8 > &out )
{

	out << "<body>\n";


	// Stop the indexer thread
	tq::stop( "indexer" );

	t_pb di;
	disk::GetDisksInfo( di, true );
	
	// Create buttons and drive lists
	str::t_string btns, lists, last;		
	listitems( di, "All", "all", btns, lists, last );
	listitems( di, "Fixed", "fixed", btns, lists, last );
	listitems( di, "CD/DVD", "cdrom", btns, lists, last );
	listitems( di, "Removable", "removable", btns, lists, last );
	listitems( di, "RamDisk", "ramdisk", btns, lists, last );
	listitems( di, "Network", "remote", btns, lists, last );
;

	out << "\n\t\n\t";
 out << btns;;

	out << "\n\n\t<ul id='dlist'></ul>\n\t\n\t";
 out << lists;;

	out << "\n\t\n\t<script type=\"text/javascript\">\n\n\t\t$('#btn_";
 out << last ;

	out << "').addClass( 'last' );\n\t\n\t\tvar g_list = 0;\n\t\tfunction switch_list( id )\n\t\t{\t$('button').removeClass( 'active' );\n\t\t\t$('#btn_'+id).addClass( 'active' );\n\t\t\t$('#dlist').quicksand( $('.list_'+id+' li') );\n\t\t\tif ( ++g_list > 3 ) g_list = 0;\n\t\t}\t\t\n\t\t\n\t\tfunction moveItem( item, from, to )\n\t\t{\tvar posSrc = from.offset(), posDst = to.offset();\t\t\t\n\t\t\tvar w = to.width(), h = to.height();\n\t\t\titem.html( from.html() );\n\t\t\titem.css( { opacity: 1, left: posSrc.left, top: posSrc.top, width: w, height: h } );\n\t\t\titem.show().animate( { left: posDst.left, top: posDst.top }, 500 );\t\t\t\n\t\t}\n\n\t\tfunction selDrive(id)\n\t\t{\t$('.centeritem').html( '' );\n\t\t\tmoveItem( $('.highlight'), $('.'+id+' .pathstr'), $('.hdr_path') );\n\t\t\tmoveItem( $('.highlightdrive'), $('.'+id+' .pathdrive'), $('.rightdrive') );\n\t\t\tselPage( 'graph' );\n\t\t}\n\n\t\t$(function() { switch_list( \"all\" ); } );\n\n\t</script>\n</body>\n";

	return 0;
}


void * f_T1354214889I4 = (void*)&_internal_run;

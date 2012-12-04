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

#include <time.h>

#include "htmapp.h"
#include "process_binary.h"
#include "process_cpp.h"
#include "process_csp.h"

typedef str::t_char8 t_char8;
typedef str::t_string8 t_string8;
typedef TCmdLine< t_string8 > t_cmdline8;
typedef std::list< t_string8 > t_strlist8;

int process_file( t_string8 x_sInDir, t_string8 x_sOutDir, t_string8 x_sOutPre, t_string8 x_sRel, t_string8 x_sFile, t_cmdline8 &cl, t_strlist8 &lstCmp, t_string8 &sRoot, long &lI )
{
	// Create unique variable name
	t_string8 sVar = sRoot + str::ToString< t_string8 >( lI++ );
	
	// Create full input and output file paths
	t_string8 sIn = disk::FilePath( x_sInDir, x_sFile );
	t_string8 sOut = disk::FilePath( x_sOutDir, disk::Path( x_sOutPre, x_sFile, '_' ) ) + t_string8( ".cpp" );
	t_string8 sFn = cl.pb()[ "f" ].str();

	// Dependency file
	if ( cl.pb()[ "t" ].length() )
		disk::AppendFile( cl.pb()[ "t" ].str(), t_string8( sOut ) + ": " + sIn + "\n\n" );

	// Is this a compiled type?
	stdForeach( t_strlist8::iterator, it, lstCmp )
		if ( 0 <= str::MatchPattern( sIn.data(), sIn.length(), it->data(), it->length(), true ) )
		{
			if ( !cl.pb().isSet( "q" ) )
				str::Print( "c: %s -> %s\n", sIn.c_str(), sOut.c_str() );

			// Declare variables
			disk::AppendFile( disk::FilePath< t_string8 >( x_sOutDir, "htmapp_resource_extern.hpp" ),
							  t_string8()
							  + "\nextern void * f_" + sVar + ";\n"
							);

			// Add to map
			disk::AppendFile( disk::FilePath< t_string8 >( x_sOutDir, "htmapp_resources.cpp" ),
							  t_string8( "\n{" )
							  + "\n\t\"" + disk::WebPath( x_sRel, x_sFile ) + "\","
							  + "\n\t" + str::ToString< t_string8 >( (long)disk::WebPath( x_sRel, x_sFile ).length() ) + ","
							  + "\n\tf_" + sVar + ","
							  + "\n\t0,"
							  + "\n\t2\n},\n"
							);
			
			// Process the embedded c/c++ file
			if ( !cl.pb().isSet( "l" ) && !cl.pb().isSet( "template" ))
				process_cpp( sIn, sOut, sVar, sFn );
			else
				process_csp( sIn, sOut, sVar, sFn );
				
			return 0;
		} // end if
		
	if ( !cl.pb().isSet( "q" ) )
		str::Print( "b: %s -> %s\n", sIn.c_str(), sOut.c_str() );

	// Declare variables
	disk::AppendFile( disk::FilePath< t_string8 >( x_sOutDir, "htmapp_resource_extern.hpp" ),
					  t_string8()
					  + "\nextern const char data_" + sVar + "[];"
					  + "\nextern const long size_" + sVar + ";\n"
					);

	// Add to map
	disk::AppendFile( disk::FilePath< t_string8 >( x_sOutDir, "htmapp_resources.cpp" ),
					  t_string8( "\n{" )
					  + "\n\t\"" + disk::WebPath( x_sRel, x_sFile ) + "\","
					  + "\n\t" + str::ToString< t_string8 >( (long)disk::WebPath( x_sRel, x_sFile ).length() ) + ","
					  + "\n\tdata_" + sVar + ","
					  + "\n\tsize_" + sVar + ",\n\t1\n},\n"
					);

	// Create the binary data file
	long lBytes = process_binary< t_string8 >( sIn, sOut,
											   t_string8( "\nextern const char data_" ) + sVar + "[] = \n{\n\t",
											   t_string8( "\n\t0\n};\n\nextern const long size_" ) + sVar + " = ",
											   ";\n"
											 );

	return lBytes;
}

int process_directory( t_string8 x_sIn, t_string8 x_sOut, t_string8 x_sOutPre, t_string8 x_sRel, t_cmdline8 &cl, t_strlist8 &lstCmp, t_string8 &sRoot, long &lI )
{
	if ( !x_sIn.length() || !x_sOut.length() )
		return 0;

	// Process directory contents
	disk::SFindData fd;
	disk::HFIND hFind = disk::FindFirst( x_sIn.c_str(), "*", &fd );
	if ( disk::c_invalid_hfind == hFind )
		return 0;

	do 
	{ 
		// Ignore relative
		if ( '.' != *fd.szName )
		{
			// Recurse if directory
			if ( disk::eFileAttribDirectory & fd.uFileAttributes )
				process_directory( disk::FilePath< t_string8 >( x_sIn, fd.szName ), 
								   x_sOut,
								   disk::Path< t_string8 >( x_sOutPre, fd.szName, '_' ),
								   disk::FilePath< t_string8 >( x_sRel, fd.szName ), 
								   cl, lstCmp, sRoot, lI );

			// Go process the file
			else
				process_file( x_sIn, x_sOut, x_sOutPre, x_sRel, fd.szName, cl, lstCmp, sRoot, lI );

		} // end if

	// While we can find more files
	} while ( disk::FindNext( hFind, &fd ) );

	// Close the find handle
	disk::FindClose( hFind );

	return 1;
}

int main( int argc, char* argv[] )
{
	// Parse the command line
    t_cmdline8 cl( argc, argv );

	// Dumping the command line options to STDOUT?
	if ( cl.pb().isSet( "d" ) || cl.pb().isSet( "debug" ) )
	{	for ( t_cmdline8::iterator it = cl.begin(); cl.end() != it; it++ )
			str::Print( "[%s] = '%s'\n", it->first.c_str(), it->second->c_str() );
		return 0;
	} // end if

	// Ensure all needed parameters are present
	if ( cl.pb().isSet( "h" ) || cl.pb().isSet( "help" )
		 || !cl.pb().isSet( "i" ) || !cl.pb().isSet( "o" ) )
	{	str::Print( "Options\n"
				" -i            '<comma separated input directories>'\n"
				" -o            '<output directory>'\n"
				" -t            'dependencies file'\n"
				" -c            '<semi-colon separated compiled file types>' - default is '*.htm'\n"
				" -f / --fdec   Function declaration\n"
				" -q / --quiet  Suppress all output\n"
				" -d / --debug  'Show processed command line array'\n"
				" -p / --plugin 'produce htmapp_resources'\n"
				" -h / --help   Display this information\n"
				" -l / --template use CTemplate\n"
				);
		return cl.pb().isSet( "h" ) ? 0 : -1;
	} // end if

	// Combine quiet options
	if ( cl.pb().isSet( "quiet" ) )
		cl.pb()[ "q" ] = 1;

	// Combine quiet options
	if ( cl.pb().isSet( "fdec" ) )
		cl.pb()[ "f" ] = cl.pb()[ "fdec" ];
		
	// Create output folder if needed
	if ( !disk::exists( cl.pb()[ "o" ] ) )
	{	if ( !cl.pb().isSet( "q" ) )
			str::Print( "Creating directory : %s\n", cl.pb()[ "o" ].c_str() );
		disk::mkdir( cl.pb()[ "o" ].c_str() );
	} // end if

	// res.d
	disk::unlink( cl.pb()[ "t" ].str().c_str() );

	// res_extern.hpp
	disk::unlink( disk::FilePath< t_string8 >( cl.pb()[ "o" ].str(), "htmapp_resource_extern.hpp" ).c_str() );


	
	// res_list.hpp
	disk::WriteFile( disk::FilePath< t_string8 >( cl.pb()[ "o" ].str(), "htmapp_resources.cpp" ),
							 t_string8() + "#include \"htmapp_resources.h\"\n"
										   "#include \"htmapp_resource_extern.hpp\"\n"
										   "SHmResourceInfo _htmapp_resources[] = \n{\n" );

	// FindQThybrid.cmake
	disk::WriteFile( disk::FilePath< t_string8 >( cl.pb()[ "o" ].str(), "FindQThybrid.cmake" ),
							 t_string8() + "IF (QTHYBRID_INCLUDE_DIR AND QTHYBRID_LIBRARIES)\n"
											"  # in cache already\n"
											"  SET(QTHYBRID_FOUND TRUE)\n\n"
											"ELSE (QTHYBRID_INCLUDE_DIR AND QTHYBRID_LIBRARIES)\n\n"
											"  IF(NOT QTHYBRID_FIND_COMPONENTS)\n"
											"    # default\n"
											"    SET(QTHYBRID_FIND_REQUIRED_COMPONENTS FRWK HTMAPP)\n\n"
											"  ENDIF(NOT QTHYBRID_FIND_COMPONENTS)\n\n"
											"  FIND_PATH(QTHYBRID_INCLUDE_DIR htmapp.h)\n"
											"  FIND_LIBRARY(QTHYBRID_FRWK_LIBRARY NAMES QThybridfrwk  HINTS \"${QTHYBRID_INCLUDE_DIR}/../../lib\")\n"
											"  FIND_LIBRARY(QTHYBRID_HTMAPP_LIBRARY NAMES QThybridhtmapp   HINTS \"${QTHYBRID_INCLUDE_DIR}/../../lib\")\n\n"
											"  SET(QTHYBRID_FOUND ON)\n\n"
											"  FOREACH(NAME ${QTHYBRID_FIND_REQUIRED_COMPONENTS})\n"
											"        # only good if header and library both found   \n"
											"        IF( QTHYBRID_${NAME}_LIBRARY)\n"
											"            LIST(APPEND QTHYBRID_LIBRARIES    ${QTHYBRID_${NAME}_LIBRARY})\n"
											"        ELSE( QTHYBRID_${NAME}_LIBRARY)\n"
											"            SET(QTHYBRID_FOUND OFF)\n"
											"        ENDIF( QTHYBRID_${NAME}_LIBRARY)\n"
											"    ENDFOREACH(NAME)\n\n\n"
											"MARK_AS_ADVANCED(FORCE\n"
											"                 QTHYBRID_ROOT_DIR\n"
											"                 QTHYBRID_INCLUDE_DIRS\n"
											"                 QTHYBRID_LIBRARIES\n"
											"                 QTHYBRID_FRWK_LIBRARY\n"
											"                 QTHYBRID_HTMAPP_LIBRARY\n"
											"                 )\n\n"
											" # display help message\n"
											"IF(NOT QTHYBRID_FOUND)\n"
											"    # make FIND_PACKAGE friendly\n"
											"    IF(QTHYBRID_FIND_REQUIRED)\n"
											"        MESSAGE(FATAL_ERROR \"QTHybrid 1.0 not found. Please specify it's location with the QTHYBRID_ROOT_DIR env. variable.\")\n"
											"    ELSE(QTHYBRID_FIND_REQUIRED)\n"
											"        MESSAGE(STATUS \"QTHybrid 1.0 not found.\")\n"
											"    ENDIF(QTHYBRID_FIND_REQUIRED)\n"
											"ENDIF(NOT QTHYBRID_FOUND)\n\n"
											"ENDIF (QTHYBRID_INCLUDE_DIR AND QTHYBRID_LIBRARIES)\n" );

	if ( cl.pb().isSet( "p" ) || cl.pb().isSet( "plugin" ))
	{
		// htmapp_resources.h
		disk::WriteFile( disk::FilePath< t_string8 >( cl.pb()[ "o" ].str(), "htmapp_resources.h" ),
								 t_string8() + 
											   "#pragma once\n\n"
											   "#define HTMAPP_RESOURCES 1\n\n"
											   "#define hmResourceFn( n ) int (*n)( const TPropertyBag< str::t_string8 > &in, std::basic_string< str::t_string8 > &out );\n"
											   "\ntypedef struct td_SHmResourceInfo\n{"
											   "\n\tconst char*   name;"
											   "\n\tunsigned long sz_size;"
											   "\n\tconst void*   data;"
											   "\n\tunsigned long sz_data;"
											   "\n\tunsigned long type;"
											   "\n} SHmResourceInfo;\n"
											   "\n\t extern \"C\" __declspec(dllexport) SHmResourceInfo _htmapp_resources[];\n"
											 );
		// cmakelists.txt
		disk::WriteFile( disk::FilePath< t_string8 >( cl.pb()[ "o" ].str(), "CMakeLists.txt" ),
								 t_string8() + 
											   "PROJECT(htmapp_resources)\n\n"
											   "CMAKE_MINIMUM_REQUIRED(VERSION 2.8)\n\n"
											   "IF (MSVC)\n"
											   "\tADD_DEFINITIONS( /D \"NOMINMAX\" /D \"WIN32_LEAN_AND_MEAN\" )\n"
											   "ENDIF (MSVC)\n"
											   "\nadd_definitions( \"/W3 /D_CRT_SECURE_NO_WARNINGS /wd4309 /nologo\" )\n"
											   "\nset(htmapp_resources_FILES ${htmapp_resources_SOURCE_DIR})\n"
											   "\nfile(GLOB_RECURSE htmapp_resources_FILES ${htmapp_resources_FILES}/*.cpp )\n"
											   "\nINCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}  ${htmapp_INCLUDE_DIRS} )\n"
											   "\nadd_library(htmapp_resources SHARED ${htmapp_resources_FILES} )\n"
											 );
	}else{
		// htmapp_resources.h
		disk::WriteFile( disk::FilePath< t_string8 >( cl.pb()[ "o" ].str(), "htmapp_resources.h" ),
								 t_string8() + 
											   "#pragma once\n\n"
											   "#define HTMAPP_RESOURCES 1\n\n"
											   "#define hmResourceFn( n ) int (*n)( const TPropertyBag< str::t_string8 > &in, std::basic_string< str::t_string8 > &out );\n"
											   "\ntypedef struct td_SHmResourceInfo\n{"
											   "\n\tconst char*   name;"
											   "\n\tunsigned long sz_size;"
											   "\n\tconst void*   data;"
											   "\n\tunsigned long sz_data;"
											   "\n\tunsigned long type;"
											   "\n} SHmResourceInfo;\n"
											   "\n#if defined( _cplusplus )\n"
											   "\nextern \"C\"\n"
											   "\n#else\n"
											   "\nextern\n"
											   "\n#endif\n"
											   "\n\tSHmResourceInfo _htmapp_resources[];\n"
											 );
		
		if ( !cl.pb().isSet( "l" ) && !cl.pb().isSet( "template" )){
			// cmakelists.txt
			disk::WriteFile( disk::FilePath< t_string8 >( cl.pb()[ "o" ].str(), "CMakeLists.txt" ),
								 t_string8() + 
											  "PROJECT(QThybridApp)\n"
												"CMAKE_MINIMUM_REQUIRED(VERSION 2.8)\n"
												"SET(QT_USE_QTMAIN TRUE)\n"
												"SET(QT_USE_QTWEBKIT TRUE)\n\n"
												"IF (MSVC)\n"
												"  ADD_DEFINITIONS( /D \"NOMINMAX\" /D \"WIN32_LEAN_AND_MEAN\" )\n"
												"ENDIF (MSVC)\n\n"
												"add_definitions( \"/W3 /D_CRT_SECURE_NO_WARNINGS /wd4309 /nologo\" )\n\n\n"
												"INCLUDE(FindQThybrid)\n"
												"FIND_PACKAGE(QThybrid REQUIRED)\n"
												"INCLUDE_DIRECTORIES(${QTHYBRID_INCLUDE_DIR})\n\n"
												"FIND_PACKAGE(Qt4 REQUIRED)\n"
												"INCLUDE(${QT_USE_FILE})\n\n"
												"file(GLOB_RECURSE QThybridApp_FILES ${QThybridApp_SOURCE_DIR}/*.cpp )\n\n"
												"ADD_EXECUTABLE(QThybridApp  WIN32 ${QThybridApp_FILES} )\n\n"
												"TARGET_LINK_LIBRARIES(QThybridApp ${QT_LIBRARIES} ${QTHYBRID_LIBRARIES})\n"
											 );

			// main.c
			disk::WriteFile( disk::FilePath< t_string8 >( cl.pb()[ "o" ].str(), "main.cpp" ),
				t_string8() + 
				"#include \"frwk.h\"\n"
				"#include \"htmapp_resources.h\"\n"
				"#include \"network.h\"\n"
				"#include \"web_page.h\"\n"
				"#include \"mainwindow.h\"\n"
				"#include <Windows.h>\n\n"
				"int main( int argc, char* argv[] )\n"
				"{\n\n"
				"	#if defined(_DEBUG)\n"
				"		QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled,\n"
				"			true);\n"
				"	#endif\n\n"
				"	HTMAPP_INIT_RESOURCES();\n\n"
				"	// Initialize thread queue\n"
				"	tq::init( true );\n\n"
				"	// Initialize application object\n"
				"	QApplication app( argc, argv );\n\n"
				"	// Create main window\n"
				"	CMainWindow *pWindow = new CMainWindow;\n"
				"	if ( !pWindow )\n"
				"		return -1;\n\n"
				"#if defined( CII_PROJECT_NAME )\n"
				"	pWindow->setName( CII_PROJECT_NAME );\n"
				"#endif	\n\n"
				"#if defined( CII_PROJECT_DESC )\n"
				"	pWindow->setDescription( CII_PROJECT_DESC );\n"
				"#endif	\n\n"
				"	// Make the command line accessible\n"
				"	tq::set( \"cmdline\", TCmdLine< str::t_string >( argc, argv ).pb() );\n\n"
				"	// Make the makefile parameters accessible\n"
				"#if defined( CII_PARAMS )\n"
				"	tq::set( \"ciid\", parser::DecodeJson< t_pb >( CII_PARAMS ) );\n"
				"#endif\n"
				"	\n"
				"	// Initialize the window\n"
				"	pWindow->Init();\n\n"
				"	// Show the window\n"
				"	pWindow->show();\n\n"
				"	// Run the app\n"
				"	int ret = app.exec();\n\n"
				"	// Uninitialize thread queue\n"
				"	tq::uninit();\n\n"
				"	return ret;\n"
				"}\n"
				);

		}else{
			disk::WriteFile( disk::FilePath< t_string8 >( cl.pb()[ "o" ].str(), "CMakeLists.txt" ),
				t_string8() + 
				"PROJECT(QThybridApp)\n"
				"CMAKE_MINIMUM_REQUIRED(VERSION 2.8)\n"
				"SET(QT_USE_QTMAIN TRUE)\n"
				"SET(CMAKE_MODULE_PATH ${QThybridApp_SOURCE_DIR} ${CMAKE_MODULE_PATH})\n"
				"SET(QT_USE_QTWEBKIT TRUE)\n\n"
				"IF (MSVC)\n"
				"  ADD_DEFINITIONS( /D \"NOMINMAX\" /D \"WIN32_LEAN_AND_MEAN\" )\n"
				"ENDIF (MSVC)\n\n"
				"add_definitions( \"/W3 /D_CRT_SECURE_NO_WARNINGS /wd4309 /nologo\" )\n\n\n"
				"INCLUDE(FindCtemplate)\n"
				"FIND_PACKAGE(Ctemplate)\n\n"
				"IF(NOT CTEMPLATE_FOUND)\n"
				"	message(SEND_ERROR \"Ctemplate needs to be built\")\n"
				"ENDIF()\n\n"
				"INCLUDE_DIRECTORIES(${CTEMPLATE_INCLUDE_DIR})\n\n"
				"INCLUDE(FindQThybrid)\n"
				"FIND_PACKAGE(QThybrid REQUIRED)\n"
				"INCLUDE_DIRECTORIES(${QTHYBRID_INCLUDE_DIR})\n\n"
				"FIND_PACKAGE(Qt4 REQUIRED)\n\n"
				"INCLUDE(${QT_USE_FILE})\n\n"
				"file(GLOB_RECURSE QThybridApp_FILES ${QThybridApp_SOURCE_DIR}/*.cpp )\n\n"
				"ADD_EXECUTABLE(QThybridApp  WIN32 ${QThybridApp_FILES} )\n\n"
				"TARGET_LINK_LIBRARIES(QThybridApp ${QT_LIBRARIES} ${CTEMPLATE_LIBRARIES} ${QTHYBRID_LIBRARIES})\n"
				);

			
			disk::WriteFile( disk::FilePath< t_string8 >( cl.pb()[ "o" ].str(), "csp_template.cpp" ),
				t_string8() + "#include \"csp_template.h\"\n"
							"str::t_string8 csp_template(str::t_string8 path, T_dictvalues values)\n"
							"{\n"
							"	str::t_string8 full = \n"
							"		disk::WebPath< str::t_string8 >( \"htm\", str::t_string8( path.data(), path.length() ) );\n"
							"	str::t_string8 htm = \"\";\n\n"
							"	CHmResources res;\n"
							"	if ( res.IsValid() )\n"
							"	{\n"
							"		// See if there is such a resource\n"
							"		HMRES hRes = res.FindResource( 0, full.c_str() );\n"
							"		if ( hRes )\n"
							"		{\n"
							"			switch ( res.Type( hRes ) )\n"
							"			{\n"
							"				default :\n"
							"					break;\n\n"
							"				case 1 :\n"
							"				{\n"
							"					const char *ptr = (const char* )res.Ptr( hRes );\n"
							"					unsigned long sz = res.Size( hRes );\n"
							"					ctemplate::TemplateDictionary dict(\"csp\");\n\n"
							"					for (T_dictvalues::iterator it=values.begin(); it!=values.end() ; it++)\n"
							"					{\n"
							"						dict[it->first] = it->second;\n"
							"					}\n\n"
							"					ctemplate::StringToTemplateCache(\"mytpl\", ptr, ctemplate::DO_NOT_STRIP);\n"
							"					ctemplate::ExpandTemplate(\"mytpl\", ctemplate::DO_NOT_STRIP, &dict, &htm);\n"
							"				}\n"
							"			}\n"
							"		}\n"
							"	}\n"
							"	return htm;\n"
							"}\n");

			disk::WriteFile( disk::FilePath< t_string8 >( cl.pb()[ "o" ].str(), "csp_template.h" ),
				t_string8() + "#pragma once\n\n"
							"#include \"htmapp.h\"\n"
							"#include \"ctemplate/template.h\"\n"
							"#include <map> \n\n"
							"using namespace std;\n\n"
							"typedef map<string , string> T_dictvalues;\n\n"
							"str::t_string8 csp_template(str::t_string8 path, T_dictvalues values);\n"
							
			);

			disk::WriteFile( disk::FilePath< t_string8 >( cl.pb()[ "o" ].str(), "FindCtemplate.cmake" ),
				t_string8() + 
				"if (CTEMPLATE_INCLUDE_DIR AND CTEMPLATE_LIBRARIES)\n"
				"  # in cache already\n"
				"  set(CTEMPLATE_FOUND TRUE)\n\n"
				"else (CTEMPLATE_INCLUDE_DIR AND CTEMPLATE_LIBRARIES)\n"
				"  find_path(CTEMPLATE_INCLUDE_DIR ctemplate/template.h)\n"
				"  find_library(CTEMPLATE_LIBRARIES NAMES ctemplate)\n"
				"  find_program(CTEMPLATE_COMPILER make_tpl_varnames_h)\n\n"
				"  include(FindPackageHandleStandardArgs)\n"
				"  FIND_PACKAGE_HANDLE_STANDARD_ARGS(Ctemplate DEFAULT_MSG CTEMPLATE_INCLUDE_DIR CTEMPLATE_LIBRARIES )\n\n"
				"endif (CTEMPLATE_INCLUDE_DIR AND CTEMPLATE_LIBRARIES)\n\n\n"
				"macro (CTEMPLATE_VARNAMES outfiles)\n"
				"  foreach (infile ${ARGN})\n"
				"    get_filename_component(infile ${infile} ABSOLUTE)\n"
				"    CTEMPLATE_MAKE_OUTPUT_file(${infile} varnames.h outfile)\n"
				"    add_custom_command(OUTPUT ${outfile}\n"
				"                       COMMAND ${CTEMPLATE_COMPILER} -q -f ${outfile} ${infile} \n"
				"                       DEPENDS ${infile} VERBATIM)\n"
				"    set(${outfiles} ${${outfiles}} ${outfile})\n"
				"  endforeach(infile)\n\n"
				"endmacro (CTEMPLATE_VARNAMES outfiles)\n\n"
				"# macro used to create the names of output files preserving relative dirs\n"
				"macro (CTEMPLATE_MAKE_OUTPUT_file infile ext outfile )\n"
				"  string(LENGTH ${CMAKE_CURRENT_BINARY_DIR} _binlength)\n"
				"  string(LENGTH ${infile} _infileLength)\n"
				"  set(_checkinfile ${CMAKE_CURRENT_SOURCE_DIR})\n"
				"  if(_infileLength GREATER _binlength)\n"
				"    string(SUBSTRING \"${infile}\" 0 ${_binlength} _checkinfile)\n"
				"    if(_checkinfile STREQUAL \"${CMAKE_CURRENT_BINARY_DIR}\")\n"
				"      file(RELATIVE_PATH rel ${CMAKE_CURRENT_BINARY_DIR} ${infile})\n"
				"    else(_checkinfile STREQUAL \"${CMAKE_CURRENT_BINARY_DIR}\")\n"
				"      file(RELATIVE_PATH rel ${CMAKE_CURRENT_SOURCE_DIR} ${infile})\n"
				"    endif(_checkinfile STREQUAL \"${CMAKE_CURRENT_BINARY_DIR}\")\n"
				"  else(_infileLength GREATER _binlength)\n"
				"    file(RELATIVE_PATH rel ${CMAKE_CURRENT_SOURCE_DIR} ${infile})\n"
				"  endif(_infileLength GREATER _binlength)\n"
				"  if(WIN32 AND rel MATCHES \"^[a-zA-Z]:\") # absolute path \n"
				"    string(REGEX REPLACE \"^([a-zA-Z]):(.*)$\" \"\\1_\\2\" rel \"${rel}\")\n"
				"  endif(WIN32 AND rel MATCHES \"^[a-zA-Z]:\") \n"
				"  set(_outfile \"${CMAKE_CURRENT_BINARY_DIR}/${rel}\")\n"
				"  string(REPLACE \"..\" \"__\" _outfile ${_outfile})\n"
				"  get_filename_component(outpath ${_outfile} PATH)\n"
				"  get_filename_component(_outfile ${_outfile} NAME)\n"
				"  file(MAKE_DIRECTORY ${outpath})\n"
				"  set(${outfile} ${outpath}/${_outfile}.${ext})\n"
				"endmacro (CTEMPLATE_MAKE_OUTPUT_file)\n"
				);

			// main.c
			disk::WriteFile( disk::FilePath< t_string8 >( cl.pb()[ "o" ].str(), "main.cpp" ),
				t_string8() + 
				"#include \"frwk.h\"\n"
				"#include \"htmapp_resources.h\"\n"
				"#include \"network.h\"\n"
				"#include \"web_page.h\"\n"
				"#include \"mainwindow.h\"\n"
				"#include <Windows.h>\n\n"
				"int main( int argc, char* argv[] )\n"
				"{\n\n"
				"	#if defined(_DEBUG)\n"
				"		QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled,\n"
				"			true);\n"
				"	#endif\n\n"
				"	HTMAPP_INIT_RESOURCES();\n\n"
				"	// Initialize thread queue\n"
				"	tq::init( true );\n\n"
				"	// Initialize application object\n"
				"	QApplication app( argc, argv );\n\n"
				"	// Create main window\n"
				"	CMainWindow *pWindow = new CMainWindow;\n"
				"	if ( !pWindow )\n"
				"		return -1;\n\n"
				"#if defined( CII_PROJECT_NAME )\n"
				"	pWindow->setName( CII_PROJECT_NAME );\n"
				"#endif	\n\n"
				"#if defined( CII_PROJECT_DESC )\n"
				"	pWindow->setDescription( CII_PROJECT_DESC );\n"
				"#endif	\n\n"
				"pWindow->setHomeUrl(\"http://embedded/index.csp\");\n\n"
				"	// Make the command line accessible\n"
				"	tq::set( \"cmdline\", TCmdLine< str::t_string >( argc, argv ).pb() );\n\n"
				"	// Make the makefile parameters accessible\n"
				"#if defined( CII_PARAMS )\n"
				"	tq::set( \"ciid\", parser::DecodeJson< t_pb >( CII_PARAMS ) );\n"
				"#endif\n"
				"	\n"
				"	// Initialize the window\n"
				"	pWindow->Init();\n\n"
				"	// Show the window\n"
				"	pWindow->show();\n\n"
				"	// Run the app\n"
				"	int ret = app.exec();\n\n"
				"	// Uninitialize thread queue\n"
				"	tq::uninit();\n\n"
				"	return ret;\n"
				"}\n"
				);
		}
	}

	// Get compiled types
	t_strlist8 lstCmp;
	if ( !cl.pb().isSet( "c" ) )
	{
		if ( !cl.pb().isSet( "l" ) && !cl.pb().isSet( "template" ))
			lstCmp.push_back( "*.htm" );
		else
			lstCmp.push_back( "*.csp" );
	}
	else
		lstCmp = str::SplitQuoted< t_string8, t_strlist8 >
								   ( (char*)cl.pb()[ "c" ].data(), cl.pb()[ "c" ].length(), 
								     ";, \t", "\"'", "\"'", "\\", true );

	// Separate the different directories
	t_strlist8 lstDir = str::SplitQuoted< t_string8, t_strlist8 >
										  ( (char*)cl.pb()[ "i" ].data(), cl.pb()[ "i" ].length(), 
											";, \t", "\"'", "\"'", "\\", true );

	// Root variable name
	t_string8 sRoot( "T" );
	sRoot += str::ToString< t_string8 >( (unsigned long)time( 0 ) );
	sRoot += "I";

	// Process each directory
	long lI = 0;
	stdForeach( t_strlist8::iterator, it, lstDir )
		process_directory( *it, cl.pb()[ "o" ].str(), *it, it->c_str(), cl, lstCmp, sRoot, lI );

	// Close up res_list.hpp
	disk::AppendFile( disk::FilePath< t_string8 >( cl.pb()[ "o" ].str(), "htmapp_resources.cpp" ), 
					  t_string8( "\n{0,0,0,0,0}\n\n};\n" ) );

	return 0;
}

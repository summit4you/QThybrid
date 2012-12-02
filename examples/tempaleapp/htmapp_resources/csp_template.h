#pragma once

#include "htmapp.h"
#include "ctemplate/template.h"
#include <map> 

using namespace std;

typedef map<string , string> T_dictvalues;

str::t_string8 csp_template(str::t_string8 path, T_dictvalues values)
{
	str::t_string8 full = 
		disk::WebPath< str::t_string8 >( "htm", str::t_string8( path.data(), path.length() ) );
	str::t_string8 htm = "";

	CHmResources res;
	if ( res.IsValid() )
	{
		// See if there is such a resource
		HMRES hRes = res.FindResource( 0, full.c_str() );
		if ( hRes )
		{
			switch ( res.Type( hRes ) )
			{
				default :
					break;

				case 1 :
				{
					const char *ptr = (const char* )res.Ptr( hRes );
					unsigned long sz = res.Size( hRes );
					ctemplate::TemplateDictionary dict("csp");

					for (T_dictvalues::iterator it=values.begin(); it!=values.end() ; it++)
					{
						dict[it->first] = it->second;
					}

					ctemplate::StringToTemplateCache("mytpl", ptr, ctemplate::DO_NOT_STRIP);
					ctemplate::ExpandTemplate("mytpl", ctemplate::DO_NOT_STRIP, &dict, &htm);
				}
			}
		}
	}
	return htm;
}
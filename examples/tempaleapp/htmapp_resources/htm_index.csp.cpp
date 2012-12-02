
#include "htmapp.h"
#include <map>
#include "csp_template.h"

	int i = 100;
	
static int _internal_run( TPropertyBag< str::t_string8 > &in, TPropertyBag< str::t_string8 > &out )
{

	out << "\r\n\r\n";

	std::map<string, string> values;
	values["i"] = "100";
	values["title"] = "hello world";

	out << csp_template("tpl/index.htm", values);


	return 0;
}


void * f_T1354434704I0 = (void*)&_internal_run;

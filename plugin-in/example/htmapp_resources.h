#pragma once

#define HTMAPP_RESOURCES 1

#define hmResourceFn( n ) int (*n)( const TPropertyBag< str::t_string8 > &in, std::basic_string< str::t_string8 > &out );

typedef struct td_SHmResourceInfo
{
	const char*   name;
	unsigned long sz_size;
	const void*   data;
	unsigned long sz_data;
	unsigned long type;
} SHmResourceInfo;



extern "C" __declspec(dllexport)	SHmResourceInfo _htmapp_resources[];

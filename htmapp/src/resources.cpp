
#include "htmapp.h"

#include "stdio.h"

// Resource pointer
const _SHmResourceInfo* _htmapp_resources_ptr = 0;

CHmResources::CHmResources()
{
	m_ptr = (const _SHmResourceInfo*)_htmapp_resources_ptr;
}

CHmResources::~CHmResources()
{
	Unload();
}

CHmResources::CHmResources( HMRES hRes )
{
	m_ptr = hRes;
}

CHmResources::CHmResources( const char *pModule )
{
	if ( pModule && *pModule )
		LoadFromModule( pModule );
	else
		m_ptr = 0;
}

void CHmResources::Unload()
{
	m_ptr = 0;
}

int CHmResources::LoadFromModule( const char *pModule )
{
	Unload();
	
	return 0;
}

int CHmResources::IsValid()
{
	return m_ptr ? 1 : 0;
}

void CHmResources::SetResourcePtr( HMRES hRes )
{
	m_ptr = hRes;
}

HMRES CHmResources::FindResource( const char *pRoot, const char *pName )
{
	if ( !pName || !*pName )
		return 0;
	
	unsigned long lName = zstr::Length( pName );	
	for ( long i = 0; m_ptr[ i ].type && *m_ptr[ i ].name; i++ )
	{	// printf( "%s(%d) : %s\n", __FILE__, __LINE__, m_ptr[ i ].name );
		if ( *m_ptr[ i ].name == *pName && m_ptr[ i ].sz_name == lName )
			if ( !str::Compare( m_ptr[ i ].name, m_ptr[ i ].sz_name, pName, lName ) )
				return &m_ptr[ i ];
	} // end for
	
	return 0;
}

const void* CHmResources::Ptr( HMRES hRes )
{
	if ( !hRes )
		return 0;
	return (const void*)hRes->data;
}

CHmResources::t_size CHmResources::Size( HMRES hRes )
{
	if ( !hRes )
		return 0;
	return hRes->sz_data;
}

CHmResources::t_fn CHmResources::Fn( HMRES hRes )
{
	if ( !hRes || 2 != hRes->type )
		return 0;
	return (t_fn)hRes->data;
}

CHmResources::t_type CHmResources::Type( HMRES hRes )
{
	if ( !hRes )
		return 0;
	return hRes->type;
}

const char* CHmResources::Name( HMRES hRes )
{
	if ( !hRes )
		return 0;
	return hRes->name;
}

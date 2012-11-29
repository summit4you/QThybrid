
#include <string>
#include <vector>
#include <string.h>

#include "htmapp.h"
#include "file_index.h"

/// Initialize space constructor
CFileIndex::CFileIndex( t_size nBlock, t_size nBlob, const void *pRootName, t_size szRootName )
{
	m_f = 0;
	m_user = 0;
	m_root = 0;
	m_deleted = 0;
	m_blob_offset = 0;
	m_block_offset = 0;

	if ( nBlock )
		Init( nBlock, nBlob, pRootName, szRootName );
}

CFileIndex::~CFileIndex()
{
	m_f = 0;
	m_user = 0;
	Destroy();
}

void CFileIndex::Destroy()
{
	// Reset blocks
	clear();
}

void CFileIndex::clear()
{
	// Clear data
	m_deleted = 0;
	m_blob_offset = 0;
	m_block_offset = 0;
	m_root = 0;

	// Release memory
	m_blocks.clear();
	m_blobs.clear();
}

bool CFileIndex::Init( t_size nBlock, t_size nBlob, const void *pRootName, t_size szRootName )
{
	// Out with the old
	Destroy();

	// Assume user wants them the same
	if ( !nBlob )
		nBlob = nBlock;
	
	// Impose reasonable minimums
	nBlob = std::max( nBlob, (t_size)1024 );
	nBlock = std::max( nBlock, (t_size)1024 );

	try { m_blocks.resize( nBlock ); }
	catch ( ... ) { Destroy(); return false; }

	try { m_blobs.resize( nBlob ); }
	catch ( ... ) { Destroy(); return false; }
	
	// Allocate NULL Block and Blob
	if ( !getBlob( NewBlob( "<<NULL>>", 4 ) ) || !getItem( NewBlock() ) )
	{	Destroy(); return false; }

	// Allocate a root block
	m_root = NewBlock();
	if ( !m_root )
	{	Destroy(); return false; }
	
	// Allocate root item
	SBlockItem *root = getItem( m_root );
	if ( !root ) 
	{	Destroy(); return false; }
	
	// Initialize root item
	memset( root, 0, sizeof( SBlockItem ) );

	// Set the root name
	if ( pRootName )
		root->name = NewBlob( pRootName, szRootName );

	return true;
}

CFileIndex::t_block CFileIndex::NewBlock()
{
	// Is there a deleted block we can use?
	if ( m_deleted )
	{	
		// Save away the block we want to use
		t_block t = m_deleted;
		
		// Is the deleted block valid?
		SBlockItem *p = getItem( m_deleted );
		if ( p )
		{	m_deleted = p ? p->next : 0;
			return t; 
		} // end if
		
		// !!! Reset deleted chain
		else
			m_deleted = 0;

	} // end if

	// Ensure space
	if ( ( m_block_offset + sizeof( SBlockItem ) ) > m_blocks.size() )
	{
		// Are we at least initialized?
		if ( 0 >= m_blocks.size() )
			return 0;

		// Attempt to allocate space
		try { m_blocks.resize( m_blocks.size() << 1 ); }
		catch ( ... ) { return 0; }

	} // end if

	// Allocate a block
	t_block p = (t_block)m_block_offset;
	m_block_offset += sizeof( SBlockItem );

	return p;
}

void CFileIndex::RemoveCount( SBlockItem *p )
{
	if ( !p )
		return;

	// Remove size from the parent chain
	t_size size = p->size;
	while ( p->parent )
		p = getItem( p->parent ), p->size -= size;
}

void CFileIndex::RemoveChildren( SBlockItem *p )
{
	/// Sanity check
	if ( !p || !p->child )
		return;

	// Get child item
	SBlockItem *pChild = getItem( p->child );
	if ( !pChild )
		return;

	// Remove children in the list
	while ( pChild->next ) 
		RemoveCount( pChild ),
		RemoveChildren( pChild ),
		pChild = getItem( pChild->next );

	// Remove children of the children
	RemoveCount( pChild );
	RemoveChildren( pChild );

	// Put this list in the deleted items
	pChild->next = m_deleted;
	m_deleted = p->child;
	p->child = 0;

}

void CFileIndex::RemoveBlock( t_block hBlock )
{
	// Can't remove the root block
	if ( !hBlock || getRoot() == hBlock )
		return;

	// Get block info
	SBlockItem *p = getItem( hBlock );
	if ( !p )
		return;

	// Remove children
	RemoveChildren( p );

	// Update parent
	if ( p->parent )
	{
		// Get parent pointer
		SBlockItem *pParent = getItem( p->parent );
		if ( pParent )
		{
			// Are we the first in the list?
			if ( pParent->child == hBlock )
				pParent->child = p->next;

			// If there are children
			else if ( pParent->child )
			{
				// Find us in the sibling list
				SBlockItem *pSib = getItem( pParent->child );
				while ( pSib )
				{
					// Is this the previous node?
					if ( pSib->next == hBlock )
					{	pSib->next = p->next;
						break;
					} // end if

					// Next sibling
					pSib = getItem( pSib->next );

				} // end while

			} // end else
			
			// !!! This shouldn't happen
			else
				;

		} // end if

	} // end if

	// Update counts
	RemoveCount( p ),

	// Add us to the deleted chain
	p->next = m_deleted;
	m_deleted = hBlock;
}

CFileIndex::t_blob CFileIndex::NewBlob( const void* pInit, t_size size )
{
	if ( 0 >= size )
	{
		// Must have valid pointer if no size
		if ( !pInit )
			return 0;

		// NULL terminiated
		size = 0;
		while ( ((char*)pInit)[ size ] )
			size++;

		// Non-zero size?
		if ( 0 >= size )
			return 0;

	} // end if

	// We're allocating an implicit null character
	t_size asize = sizeof( t_size ) + size + 1;
	
	// Ensure space
	if ( ( m_blob_offset + asize ) > m_blobs.size() )
	{
		// Are we at least initialized?
		if ( 0 >= m_blobs.size() )
			return 0;

		// Increase size / we're allocating an implicit null character
		t_size newsize = m_blobs.size();
		while ( newsize && newsize < ( m_blob_offset + asize ) )
			newsize <<= 1;

		// Did it overflow?
		if ( newsize < asize )
			return 0;
 
		// Attempt to allocate space
		try{ m_blobs.resize( newsize ); }
		catch( ... ) { return 0; }

	} // end if
	
	// Get block pointer
	char *p = (char*)&m_blobs[ m_blob_offset ];
	if ( !p )
		return 0;

	// Allocate the block
	t_blob b =(t_blob)m_blob_offset;
	m_blob_offset += asize;

	// Set the blob size
	*((t_size*)p) = size;
	p += sizeof( t_size );
		
	// Copy blob data if needed
	if ( pInit )
		memcpy( p, pInit, size );
	
	// Null terminate
	p[ size ] = 0;

	// Return blob offset
	return b;
}

CFileIndex::t_block CFileIndex::AddSibling( t_block hBlock, const t_char *name, t_size sz_name, t_size size, long flags )
{
	// Note, the root node can't have siblings
	if ( !hBlock || getRoot() == hBlock )
		return 0;
		
	// Allocate new block, this could invalidate all pointers
	t_block hNew = NewBlock();
	if ( !hNew )
		return 0;
		
	SBlockItem *pNew = getItem( hNew );
	if ( !pNew )
		return 0;

	SBlockItem *p = getItem( hBlock );
	if ( !p || !p->parent )		
		return 0;

	SBlockItem *pParent = getItem( p->parent );
	if ( !pParent )
		return 0;
	
	// Add us into the list
	pNew->next = pParent->child;
	pParent->child = hNew;
		
	// Initialize block
	pNew->parent = p->parent;
	pNew->child = 0;
	pNew->size = size;
	pNew->flags = flags;
	pNew->name = NewBlob( name, sz_name );

	// Update sizes
	while ( pParent )
		pParent->size += size,
		pParent = pParent->parent ? getItem( pParent->parent ) : 0;

	return hNew;
}

CFileIndex::t_block CFileIndex::AddChild( t_block hBlock, t_char *name, t_size sz_name, t_size size, long flags )
{
	// Allocate a new block, this could invalidate all pointers
	t_block hNew = NewBlock();
	if ( !hNew )
		return 0;
		
	// Get new block structure
	SBlockItem *pNew = getItem( hNew );
	if ( !pNew )
		return 0;

	// Zero means root
	if ( !hBlock )
		hBlock = getRoot();

	// Get block info
	SBlockItem *p = getItem( hBlock );
	if ( !p )		
		return 0;

	// Add as child
	pNew->next = p->child;
	p->child = hNew;

	// Initialize block
	pNew->parent = hBlock;
	pNew->child = 0;
	pNew->size = size;
	pNew->flags = flags;
	pNew->name = NewBlob( name, sz_name );

	// Update sizes
	while ( p )
		p->size += size,
		p = p->parent ? getItem( p->parent ) : 0;

	return hNew;
}

CFileIndex::t_block CFileIndex::findBlock( t_block hBlock, const t_string &name, const t_string &sep )
{
	// Sanitch check
	if ( !hBlock || !name.length() )
		return hBlock;
	
	// Get block info
	SBlockItem *p = getItem( hBlock );
	if ( !p )		
		return 0;

	// Get first key
	t_string key, next = name;
	while ( !key.length() && next.length() )
	{	t_string::size_type pos = next.find_first_of( sep );
		key = ( t_string::npos == pos ) ? next : t_string( next, 0, pos );
		next = ( t_string::npos == pos ) ? "" : t_string( next, pos + 1 );
	};

	// Null key means this block
	if ( !key.length() )
		return hBlock;
		
	// Search children for a match
	hBlock = p->child;
	p = hBlock ? getItem( hBlock ) : 0;
	while ( p )
	{
		// See if this is our block
		t_size sz = 0;
		const char *name = getBlob( p->name, &sz );
		if ( name && sz && key == t_string( name, sz ) )
			return findBlock( hBlock, next, sep );
	
		// Next item
		hBlock = p->next;
		p = hBlock ? getItem( hBlock ) : 0;
	
	} // end while
	
	// Not found
	return 0;
}

long CFileIndex::Index( t_block hBlock, const t_string &sRoot, long lMinDepth, long lMaxDepth, volatile long *plCancel )
{
	// Sanity check
	if ( !hBlock || 0 >= lMaxDepth )
		return 0;

	// Do we need a cancel variable?
	long lCancel = 0;
	if ( !plCancel )
		plCancel = &lCancel;

	// Assume no additions
	long lAdded = 0, lCount = 0;
	
	// Are we at the minimum depth?
	if ( 0 < lMinDepth )
	{
		// Get child
		SBlockItem *p = getItem( hBlock );
		if ( !p->child )
			return 0;
		
		// Step down one level
		hBlock = p->child;
	
		// Proces all blocks at this level
		while ( hBlock )
		{	
			// Check for cancel signal
			if ( plCancel && *plCancel )
				return 0;
		
			// Get block information
			p = getItem( hBlock );
			
			// Go ahead and point to next block, 
			// pointer may not be good after Index() returns
			t_block hThis = hBlock;
			hBlock = p->next;
			
			// Process this block if directory
			if ( p && p->name && 0 != ( p->flags & disk::eFileAttribDirectory ) )
				lAdded += Index( hThis, disk::FilePath< t_string >( sRoot, getBlob( p->name ) ),
								 lMinDepth - 1, lMaxDepth - 1, plCancel );

			// Callback function every 100 items
			if ( lCount++ && !( lCount % 100 ) )
				if ( m_f && plCancel )
					if ( m_f( this, m_user ) )
						*plCancel = 1;

		} // end while		

		return lAdded;
		
	} // end if

	disk::SFindData fd; disk::HFIND hFind;
	if ( disk::c_invalid_hfind != ( hFind = disk::FindFirst( sRoot.c_str(), "*", &fd, disk::eReqSize ) ) )
	{
		do 
		{
			// Check for cancel signal
			if ( plCancel && *plCancel )
			{	disk::FindClose( hFind );			
				return 0;
			} // end if
		
			// Dot check
			if ( fd.szName[ 0 ] != '.'
				 || ( fd.szName[ 1 ] && ( fd.szName[ 1 ] != '.' || fd.szName[ 2 ] ) ) )
			{
				// One added
				lAdded++;

				t_string sFull = disk::FilePath< t_string >( sRoot, fd.szName );
//				str::Print( "%s\n", sFull.c_str() );

				// Directory
				if ( 0 != ( fd.uFileAttributes & disk::eFileAttribDirectory ) )
						lAdded += Index( AddChild( hBlock, fd.szName, 0, 0, fd.uFileAttributes ), 
										 sFull, lMinDepth - 1, lMaxDepth - 1, plCancel );

				// File
				else
					AddChild( hBlock, fd.szName, 0, fd.llSize, fd.uFileAttributes );

			} // end if

			// Callback function every 100 items
			if ( lCount++ && !( lCount % 100 ) )
				if ( m_f && plCancel )
					if ( m_f( this, m_user ) )
						*plCancel = 1;
			
		// For each directory item
		} while ( disk::FindNext( hFind, &fd ) );

		// Close the find
		disk::FindClose( hFind );

	} // end if

	// Callback function
	if ( m_f && plCancel )
		if ( m_f( this, m_user ) )
			*plCancel = 1;

	return lAdded;
	
}


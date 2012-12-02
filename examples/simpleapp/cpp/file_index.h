
#pragma once

#define tcDECLARE_HANDLE( h ) struct h##_htmapp_handle; typedef struct h##_htmapp_handle *h

class CFileIndex
{
private:
	CFileIndex( const CFileIndex & ) {}
	CFileIndex& operator = ( const CFileIndex & ) { return *this; }
	
public:

	/// Size type
	typedef long long int	t_size;
	
	/// Offse type
	typedef long			t_offset;
	
	/// Character type
	typedef char			t_char;
	
	/// String type
	typedef std::basic_string< t_char >	t_string;
	
	/// Update callback type
	typedef long (*fn_callback)( CFileIndex*, void* );
	
	/// Block handle type
	tcDECLARE_HANDLE( t_block );
	
	/// Blob handle
	tcDECLARE_HANDLE( t_blob );

public:

	struct SBlockItem
	{
		/// Offset to parent block item
		t_block			parent;

		/// Offset to next block item
		t_block			next;

		/// Offset to child item
		t_block			child;

		/// Item name
		t_blob			name;

		/// Item size
		t_size			size;
		
		/// Flags
		long			flags;
	};

public:

	/// Default constructor
	CFileIndex( t_size nBlock = 0, t_size nBlob = 0, const void *pRootName = 0, t_size szRootName = 0 );

	/// Destructor
	virtual ~CFileIndex();

	/// Releases all resources
	void Destroy();
	
	/// Empties the list without releasing memory
	void clear();

	/// Allocate initial storage for indexing
	bool Init( t_size nBlock, t_size nBlob = 0, const void *pRootName = 0, t_size szRootName = 0 );

	/// Returns the root node
	t_block getRoot() { return m_root; }
	
	/// Finds the specifed block
	t_block findBlock( t_block hBlock, const t_string &name, const t_string &sep );

	/// Returns a pointer to the specified item
	SBlockItem* getItem( t_block offset ) 
	{	if ( (t_size)offset + sizeof( SBlockItem ) < m_blocks.size() ) 
			return (SBlockItem*)&m_blocks[ (t_size)offset ]; 
		return 0;
	}
	
	/// Returns a pointer to the specified blob data
	const char* getBlob( t_blob offset, t_size *pSz = 0 ) 
	{	if ( (t_size)offset + sizeof( t_size ) < m_blobs.size() ) 
		{	t_size sz = *((t_size*)&m_blobs[ (t_size)offset ]);
			if ( (t_size)offset + sizeof( t_size ) + sz > m_blobs.size() )
				return 0;
			if ( pSz )
				*pSz = sz;
			return (const char*)&m_blobs[ (t_size)offset + sizeof( t_size ) ]; 
		} // end if
		return 0;
	}
	
	/// Returns a new uninitialized block
	t_block NewBlock();

	/// Returns a new blob, blobs cannot be deleted
	t_blob NewBlob( const void* p, t_size size );

	/// Removes a blocks size from the parent chain counts
	void RemoveCount( SBlockItem *p );

	/// Removes a blocks children
	void RemoveChildren( SBlockItem *p );

	/// Removes the specified block, the block is not released, but can be reused
	void RemoveBlock( t_block hBlock );

	/// Add a sibling to the specified block
	t_block AddSibling( t_block hBlock, const t_char *name, t_size sz_name, t_size size, long flags );

	/// Adds a child item to the specified block
	t_block AddChild( t_block hBlock, t_char *name, t_size sz_name, t_size size, long flags );

	/// Indexes the specified root folder
	long Index( t_block p, const t_string &sRoot, long lMinDepth, long lMaxDepth, volatile long *plCancel = 0 );

	/// Sets the output key
	void setCallback( fn_callback p, void* u ) { m_f = p; m_user = u; }
	
private:

	// Callback function
	fn_callback				m_f;
	
	/// User data passed to callback function
	void					*m_user;

	/// Root node
	t_block					m_root;
	
	/// Deleted block chain
	t_block					m_deleted;

	/// Block array
	std::vector< char >		m_blocks;

	/// Block offset
	t_offset				m_block_offset;

	/// Blob data
	std::vector< char >		m_blobs;

	/// Blob offset
	t_offset				m_blob_offset;

};


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

#pragma once

#include "str.h"

/**
	Platform independent file functions

	File find
	@code

		const char *pRoot = "c:\\Temp";

		disk::SFindData fd; disk::HFIND hFind;
		if ( disk::c_invalid_hfind != ( hFind = disk::FindFirst( pRoot, "*", &fd ) ) )
		{
			do { printf( "%s\n", disk::FilePath< char, std::basic_string< char > >( pRoot, fd.szName ).c_str() );
			} while ( disk::FindNext( hFind, &fd ) );

			disk::FindClose( hFind );

		} // end if

	@endcode

*/
namespace disk
{
	typedef unsigned long t_size;

#if defined( _WIN32 )
		typedef void* HFILE;
#else
		typedef void* HFILE;
#endif

	const HFILE c_invalid_hfile = (HFILE)-1;
	
	const t_size MAXPATHLEN = 1024;

	template< typename T_STR >
		T_STR GetRoot( T_STR s )
		{	typedef typename T_STR::value_type T;
			typename T_STR::size_type i = s.find_first_of( tcTT( T, "\\/" ) );
			if ( T_STR::npos == i )
				return T_STR();
			return T_STR( s.data(), 0, i );
		}

	template< typename T_STR >
		T_STR StripRoot( T_STR s )
		{	typedef typename T_STR::value_type T;
			typename T_STR::size_type i = s.find_first_of( tcTT( T, "\\/" ) );
			if ( T_STR::npos == i )
				return s;
			typename T_STR::size_type e = s.find_first_not_of( tcTT( T, "\\/" ), i );
			if ( T_STR::npos == e )
				return T_STR();
			return T_STR( s.data(), e, s.length() - e );
		}
		
	template< typename T_STR >
		T_STR SkipRoot( T_STR s, T_STR root )
		{	typedef typename T_STR::value_type T;
			typename T_STR::size_type i = s.find( root );
			if ( T_STR::npos == i )
				return s;
			i = s.find_first_not_of( tcTT( T, "\\/" ), i + root.length() );
			if ( T_STR::npos == i )
				return T_STR();
			return T_STR( s, i );
		}

	template< typename T_STR >
		T_STR GetPath( T_STR s )
		{	typedef typename T_STR::value_type T;
			typename T_STR::size_type i = s.find_last_of( tcTT( T, "\\/" ) );
			if ( T_STR::npos == i )
				return T_STR();
			typename T_STR::size_type e = s.find_last_not_of( tcTT( T, "\\/" ), i );
			if ( T_STR::npos == e )
				return T_STR();
			return T_STR( s.data(), 0, e + 1 );
		}

	template< typename T_STR >
		T_STR GetName( T_STR s )
		{	typedef typename T_STR::value_type T;
			typename T_STR::size_type i = s.find_last_of( tcTT( T, "\\/" ) );
			if ( T_STR::npos == i )
				return s;
			return T_STR( s.data(), i + 1 );
		}

	template< typename T_STR >
		T_STR Path( T_STR s1, T_STR s2, typename T_STR::value_type sep )
		{	typedef typename T_STR::value_type T;
			str::ReplaceCharInPlace( s1, tcTC( T, '\\' ), sep );
			str::ReplaceCharInPlace( s2, tcTC( T, '\\' ), sep );
			return str::RTrimInPlace( str::RTrimInPlace( s1, tcTC( T, '\\' ) ), tcTC( T, '/' ) )
				   + sep
				   + str::LTrimInPlace( str::LTrimInPlace( s2, tcTC( T, '\\' ) ), tcTC( T, '/' ) );
		}

	template< typename T_STR >
		T_STR WebPath( T_STR s1, T_STR s2 )
		{	typedef typename T_STR::value_type T;
			return Path( s1, s2, tcTC( T, '/' ) ); 
		}

	template< typename T_STR >
		T_STR FilePath( T_STR s1, T_STR s2 )
		{	typedef typename T_STR::value_type T;
#if defined( _WIN32 )
			return Path( s1, s2, tcTC( T, '\\' ) );
#else
			return Path( s1, s2, tcTC( T, '/' ) );
#endif
		}

	template< typename T_STR >
		T_STR GetExtension( const T_STR &sFile )
		{	typedef typename T_STR::value_type T;
			T_STR sFilename = GetName( sFile );
			unsigned long pos = sFile.find_last_of( tcTT( T, "." ) );
			if ( T_STR::npos == pos )
				return T_STR();
			return T_STR( sFile, pos + 1 );
		}

	template < typename T_STR >
		T_STR GetMimeType( T_STR sFile )
		{	typedef typename T_STR::value_type T;
			T_STR sExt = disk::GetExtension( sFile );			
			if ( !sExt.length() )
				return tcTT( T, "application/octet-stream" );
				
			str::ToLower< T >( (T*)sExt.data(), sExt.length() );

			// +++ Add MIME types
			if ( sExt == tcTT( T, "jpg" ) ) return tcTT( T, "image/jpeg" );
			else if ( sExt == tcTT( T, "jpeg" ) ) return tcTT( T, "image/jpeg" );
			else if ( sExt == tcTT( T, "png" ) ) return tcTT( T, "image/png" );
			else if ( sExt == tcTT( T, "gif" ) ) return tcTT( T, "image/gif" );
			else if ( sExt == tcTT( T, "htm" ) ) return tcTT( T, "text/html" );
			else if ( sExt == tcTT( T, "html" ) ) return tcTT( T, "text/html" );
			else if ( sExt == tcTT( T, "css" ) ) return tcTT( T, "text/css" );
			else if ( sExt == tcTT( T, "txt" ) ) return tcTT( T, "text/plain" );
			else return tcTT( T, "application/octet-stream" );
		}

	// File flags
    enum
    {
        eFileAttribReadOnly = 0x00000001,
        eFileAttribHidden = 0x00000002,
        eFileAttribSystem = 0x00000004,
        eFileAttribDirectory = 0x00000010,
        eFileAttribArchive = 0x00000020,
        eFileAttribDevice = 0x00000040,
        eFileAttribNormal = 0x00000080,
        eFileAttribTemporary = 0x00000100,
        eFileAttribSparseFile = 0x00000200,
        eFileAttribReparsePoint = 0x00000400,
        eFileAttribCompressed = 0x00000800,
        eFileAttribOffline = 0x00001000,
        eFileAttribNotContentIndexed = 0x00002000,
        eFileAttribEncrypted = 0x00004000
    };

	/// Find data structure
	struct SFindData
	{
		str::tc_int64	uFileAttributes;
		str::tc_int64	ftCreated;
		str::tc_int64	ftLastAccess;
		str::tc_int64	ftLastModified;
		str::tc_int64	llSize;
		char			szName[ 1024 ];
		
		/// This is just a hack for linux atm
		const char		*orgPath;
	};

	/// Special folder id's
	enum
	{
		eFidNone = 0xff00,

		eFidTemp,

		eFidSystem,

		eFidUserOs,

		eFidCurrent,

		eFidRoot,

		eFidDefDrive,

		eFidFonts,
		
		eFidSettings,

		eFidRecycle,

		eFidDesktop,

		eFidDownloads,

		eFidTemplates,

		eFidPublic,

		eFidDocuments,

		eFidMusic,

		eFidPictures,

		eFidVideo,
		
		eFidFavorites,

		eFidHistory,

		eFidCookies,

		eFidNetwork,

		eFidPrinters,

		eFidStartMenu,

		eFidStartup,

		eFidRecent

	};
		
	enum
	{
		/// Set if size field is required
		eReqSize = 0x00000001
	
	};
	
	/// Returns the specified folder path
	str::t_string8 GetSysFolder( bool x_bShared, long x_nFolderId, long x_nMaxLength = MAXPATHLEN );

	/// Returns a type/description token for the specified drive
	str::t_string GetDriveTypeStr( const str::t_string &x_sDrive );
	
	/// Returns the file system type string
	str::t_string GetFsTypeStr( unsigned long type );

	/// Returns a property bag with information on the specified disk
	long GetDiskInfo( t_pb &pb, const str::t_string &x_sDrive );

	/// Returns a property bag filled with disk information
	long GetDisksInfo( t_pb &pb, bool bInfo );

	/// Find handle
	typedef void* HFIND;

	/// Invalid find handle value
	const HFIND c_invalid_hfind = (HFIND)-1;

	/// Returns non-zero if specified string is not a dot path
	bool isDotPath(  const char *p );
	
	/// Finds the first file matching the specified requirements
	/**
		@param[in]	x_pPath		- Root path for search
		@param[in]	x_pMask		- File mask to search for
		@param[in]	x_pFd		- Pointer to find data structure
		
		@return Returns a handle to an open find
	*/
	HFIND FindFirst( const char *x_pPath, const char *x_pMask, SFindData *x_pFd, unsigned long x_uReqFlags = 0 );

	/// Finds the next file matching the specified requirements
	/**
		@param[in]	x_hFind		- Find handle opened with ff_FindFirst()
		@param[in]	x_pFd		- Pointer to find data structure
		
		@return Returns a handle to an open find
	*/
	bool FindNext( HFIND x_hFind, SFindData *x_pFd );

	/// Closes a find handle and releases resources
	/**
		@param[in]	x_hFind		- Find handle opened with ff_FindFirst()
	*/
	bool FindClose( HFIND x_hFind );

	/// Opens the specified file and returns a handle
	HFILE Open( const char *x_pFile, const char *x_pMode );

	/// Opens the specified file and returns a handle
	t_size Write( const void *x_pData, t_size x_nSize, t_size x_nCount, HFILE x_hFile );

	/// Opens the specified file and returns a handle
	t_size Read( void *x_pData, t_size x_nSize, t_size x_nCount, HFILE x_hFile );

	/// Closes the specified file handle
	t_size Close( HFILE x_hFile );

	/// Returns the size of the file opened by the specified handle
	str::tc_int64 Size( HFILE hFile );

	/// Returns the size of the named file
	str::tc_int64 Size( const char *x_pFile );

	/// Creates the specified directory
	bool mkdir( const char *x_pPath );

	/// Returns non-zero if the specified path exists
	bool exists( const char *x_pPath );

	/// Deletes the specified file
	bool unlink( const char *x_pFile );

	/// Removes the sepecified directory
	bool rmdir( const char *x_pPath );

	/// Writes the data to the named file
	/**
		@param [in] sFile	- Name of file
		@param [in] sData	- Data to write to file
	*/
	template< typename T_STR >
		t_size WriteFile( T_STR sFile, T_STR sData )
		{	typedef typename T_STR::value_type T;
			HFILE hOut = Open( sFile.c_str(), "wb" );
			if ( c_invalid_hfile == hOut )
				return 0;
			t_size n = Write( sData.data(), sizeof( T ), sData.length(), hOut );
			Close( hOut );
			return n;
		}

	template< typename T_STR >
		t_size WriteFile( T_STR sFile, T_STR sData1, T_STR sData2 )
		{	typedef typename T_STR::value_type T;
			HFILE hOut = Open( sFile.c_str(), "wb" );
			if ( c_invalid_hfile == hOut )
				return 0;
			t_size n = Write( sData1.data(), sizeof( T ), sData1.length(), hOut )
					   + Write( sData2.data(), sizeof( T ), sData2.length(), hOut );
			Close( hOut );
			return n;
		}

	/// Appends the data to the named file
	/**
		@param [in] sFile	- Name of file
		@param [in] sData	- Data to append to file
	*/
	template< typename T_STR >
		t_size AppendFile( T_STR sFile, T_STR sData )
		{	typedef typename T_STR::value_type T;
			HFILE hOut = Open( sFile.c_str(), "ab" );
			if ( c_invalid_hfile == hOut )
				hOut = Open( sFile.c_str(), "wb" );
			if ( c_invalid_hfile == hOut )
				return 0;
			t_size n = Write( sData.data(), sizeof( T ), sData.length(), hOut );
			Close( hOut );
			return n;
		}

	/// Reads data from the specified file into a string 
	/**
		@param [in] sFile	- Name of file
		@param [in] sData	- Data to write to file
	*/
	template< typename T_STR >
		T_STR ReadFile( T_STR sFile, str::tc_int64 nMax = 0 )
		{
			typedef typename T_STR::value_type T;
			
			// Open the file
			HFILE hIn = Open( sFile.c_str(), "rb" );
			if ( c_invalid_hfile == hIn )
				return T_STR();

			// How much space do we need?
			str::tc_int64 sz = Size( hIn ) / sizeof( T );
			if ( !sz )
				return T_STR();

			// Is it too much?
			if ( 0 < nMax && nMax < sz )
				sz = nMax;

			// Attempt to allocate memory
			T_STR sData;

			// Set size
			try { sData.resize( sz );
			} catch( ... ) { return T_STR(); }

			// Read in the data
			sz = Read( (T*)sData.data(), sizeof( T ), sz, hIn );
			if ( 0 >= sz )
				return T_STR();

			// Close the file
			Close( hIn );

			// Return the data
			return sData;
		}

}; // namespace disk

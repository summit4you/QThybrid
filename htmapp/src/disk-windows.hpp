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

#if defined( _MSC_VER )
#	include <ShlObj.h>
#	include <Shellapi.h>
#else
#	define HTM_USE_DYNAMIC_SHELL32

#define CSIDL_ALTSTARTUP              29
#define CSIDL_APPDATA                 26
#define CSIDL_BITBUCKET               10
#define CSIDL_CDBURN_AREA             59
#define CSIDL_COMMON_ADMINTOOLS       47
#define CSIDL_COMMON_ALTSTARTUP       30
#define CSIDL_COMMON_APPDATA          35
#define CSIDL_COMMON_DESKTOPDIRECTORY 25
#define CSIDL_COMMON_DOCUMENTS        46
#define CSIDL_COMMON_FAVORITES        31
#define CSIDL_COMMON_MUSIC            53
#define CSIDL_COMMON_PICTURES         54
#define CSIDL_COMMON_PROGRAMS         23
#define CSIDL_COMMON_STARTMENU        22
#define CSIDL_COMMON_STARTUP          24
#define CSIDL_COMMON_TEMPLATES        45
#define CSIDL_COMMON_VIDEO            55
#define CSIDL_CONTROLS                 3
#define CSIDL_COOKIES                 33
#define CSIDL_DESKTOP                  0
#define CSIDL_DESKTOPDIRECTORY        16
#define CSIDL_DRIVES                  17
#define CSIDL_FAVORITES                6
#define CSIDL_FONTS                   20
#define CSIDL_HISTORY                 34
#define CSIDL_INTERNET                 1
#define CSIDL_INTERNET_CACHE          32
#define CSIDL_LOCAL_APPDATA           28
#define CSIDL_MYDOCUMENTS             12
#define CSIDL_MYMUSIC                 13
#define CSIDL_MYPICTURES              39
#define CSIDL_MYVIDEO                 14
#define CSIDL_NETHOOD                 19
#define CSIDL_NETWORK                 18
#define CSIDL_PERSONAL                 5
#define CSIDL_PRINTERS                 4
#define CSIDL_PRINTHOOD               27
#define CSIDL_PROFILE                 40
#define CSIDL_PROFILES                62
#define CSIDL_PROGRAM_FILES           38
#define CSIDL_PROGRAM_FILES_COMMON    43
#define CSIDL_PROGRAMS                 2
#define CSIDL_RECENT                   8
#define CSIDL_SENDTO                   9
#define CSIDL_STARTMENU               11
#define CSIDL_STARTUP                  7
#define CSIDL_SYSTEM                  37
#define CSIDL_TEMPLATES               21
#define CSIDL_WINDOWS                 36

#endif

HFILE Open( const char *x_pFile, const char *x_pMode )
{
	tHFILE res = fopen( x_pFile, x_pMode );
	if ( !res )
		return c_invalid_hfile;
	return (HFILE)res;
}

t_size Write( const void *x_pData, t_size x_nSize, t_size x_nCount, HFILE x_hFile )
{
	return fwrite( x_pData, x_nSize, x_nCount, (tHFILE)x_hFile );
}

t_size Read( void *x_pData, t_size x_nSize, t_size x_nCount, HFILE x_hFile )
{
	return fread( x_pData, x_nSize, x_nCount, (tHFILE)x_hFile );
}

t_size Close( HFILE x_hFile )
{
	return fclose( (tHFILE)x_hFile );
}

bool mkdir( const char *x_pPath )
{
	return ::CreateDirectory( x_pPath, NULL ) ? true : false;
}

bool exists( const char *x_pPath )
{
	return ( INVALID_FILE_ATTRIBUTES != ::GetFileAttributes( x_pPath ) ) ? true : false;
}

bool unlink( const char *x_pFile )
{
	::SetFileAttributes( x_pFile, FILE_ATTRIBUTE_NORMAL );
	return ::DeleteFile( x_pFile ) ? true : false;
}

bool rmdir( const char *x_pPath )
{
	::SetFileAttributes( x_pPath, FILE_ATTRIBUTE_NORMAL );
	return ::RemoveDirectory( x_pPath ) ? true : false;
}

str::tc_int64 Size( const char *x_pFile )
{
	if ( !x_pFile || !*x_pFile )
		return 0;

	// Get file info
    WIN32_FILE_ATTRIBUTE_DATA wfad; memset( &wfad, 0, sizeof( wfad ) );	
	if ( !GetFileAttributesEx( x_pFile, GetFileExInfoStandard, (LPVOID)&wfad ) )
		return 0;

	// Return the size
	return (str::tc_int64)wfad.nFileSizeLow | ( (str::tc_int64)wfad.nFileSizeHigh << 32 );
}

str::tc_int64 Size( HFILE hFile )
{
	// Ensure valid handle
	if ( c_invalid_hfile == hFile ) 
		return 0;

	// Save the current position and seek to the end of the file
	long pos = ftell( (tHFILE)hFile );
	fseek ( (tHFILE)hFile, 0, SEEK_END );

	// Read the file size
    long size = ftell( (tHFILE)hFile );

	// Restore position and return the size
	fseek( (tHFILE)hFile, pos, SEEK_SET );
	return size;
}


HFIND FindFirst( const char *x_pPath, const char *x_pMask, SFindData *x_pFd, unsigned long x_uReqFlags )
{
	// Sanity checks
	if ( !x_pPath || !x_pMask || !x_pFd )
		return c_invalid_hfind;

	WIN32_FIND_DATA wfd;
	ZeroMemory( &wfd, sizeof( wfd ) );

	// Where will we be looking?
	std::basic_string< char > sRoot 
		= FilePath( std::basic_string< char >( x_pPath ), std::basic_string< char >( x_pMask ) );

	// Find first file
	HANDLE hFind = ::FindFirstFile( sRoot.c_str(), &wfd );
	if ( INVALID_HANDLE_VALUE == hFind )
		return c_invalid_hfind;
	
	// Ignore . and ..
	while ( isDotPath( wfd.cFileName ) )
		if ( !::FindNextFile( hFind, &wfd ) )
		{	::FindClose( hFind );
			return c_invalid_hfind;
		} // end if

	// +++ Linux hack
	x_pFd->orgPath = 0;

	// Copy over data
	x_pFd->uFileAttributes = wfd.dwFileAttributes;
	x_pFd->ftCreated = (str::tc_int64)wfd.ftCreationTime.dwLowDateTime | ( (str::tc_int64)wfd.ftCreationTime.dwHighDateTime << 32 );
	x_pFd->ftLastAccess = (str::tc_int64)wfd.ftLastAccessTime.dwLowDateTime | ( (str::tc_int64)wfd.ftLastAccessTime.dwHighDateTime << 32 );
	x_pFd->ftLastModified = (str::tc_int64)wfd.ftLastWriteTime.dwLowDateTime | ( (str::tc_int64)wfd.ftLastWriteTime.dwHighDateTime << 32 );
	x_pFd->llSize = (str::tc_int64)wfd.nFileSizeLow | ( (str::tc_int64)wfd.nFileSizeHigh << 32 );
	zstr::Copy( x_pFd->szName, sizeof( x_pFd->szName ), wfd.cFileName );	

	// Return the file handle
	return (HFIND)hFind;
}

bool FindNext( HFIND x_hFind, SFindData *x_pFd )
{
	// Sanity checks
	if ( c_invalid_hfind == x_hFind || !x_pFd )
		return false;

	WIN32_FIND_DATA wfd;
	ZeroMemory( &wfd, sizeof( wfd ) );

	do
	{
		// Attempt to find the next file
		if ( !::FindNextFile( x_hFind, &wfd ) )
			return false;

	} while ( isDotPath( wfd.cFileName ) );

	// Copy over data
	x_pFd->uFileAttributes = wfd.dwFileAttributes;
	x_pFd->ftCreated = (str::tc_int64)wfd.ftCreationTime.dwLowDateTime | ( (str::tc_int64)wfd.ftCreationTime.dwHighDateTime << 32 );
	x_pFd->ftLastAccess = (str::tc_int64)wfd.ftLastAccessTime.dwLowDateTime | ( (str::tc_int64)wfd.ftLastAccessTime.dwHighDateTime << 32 );
	x_pFd->ftLastModified = (str::tc_int64)wfd.ftLastWriteTime.dwLowDateTime | ( (str::tc_int64)wfd.ftLastWriteTime.dwHighDateTime << 32 );
	x_pFd->llSize = (str::tc_int64)wfd.nFileSizeLow | ( (str::tc_int64)wfd.nFileSizeHigh << 32 );
	zstr::Copy( x_pFd->szName, sizeof( x_pFd->szName ), wfd.cFileName );

	return true;
}

bool FindClose( HFIND x_hFind )
{
	if ( c_invalid_hfind == x_hFind )
		return false;
	return ::FindClose( x_hFind ) ? true : false;
}

#if defined( HTM_USE_DYNAMIC_SHELL32 )
	
	// SH Types
	typedef IMalloc* t_LPMALLOC;
	typedef struct { USHORT cb; BYTE abID[ 1 ]; } t_SHITEMID;
	typedef struct { t_SHITEMID mkid; } t_ITEMIDLIST, *t_LPITEMIDLIST;
	
	// SH Functions
	typedef HRESULT (*pfn_SHGetMalloc)( t_LPMALLOC *ppMalloc );
	typedef BOOL (*pfn_SHGetPathFromIDList)( t_LPITEMIDLIST pidl, LPTSTR pszPath );
	typedef HRESULT (*pfn_SHGetSpecialFolderLocation)( HWND hwndOwner, int nFolder, t_LPITEMIDLIST *ppidl );

#else

#	define t_LPMALLOC LPMALLOC
#	define t_LPITEMIDLIST LPITEMIDLIST
#	define pSHGetMalloc SHGetMalloc
#	define pSHGetPathFromIDList SHGetPathFromIDList
#	define pSHGetSpecialFolderLocation SHGetSpecialFolderLocation

#endif

str::t_string GetSysFolder( bool x_bShared, long x_nFolderId, long x_nMaxLength )
{
	// Ensure at least MAX_PATH bytes
	if ( MAX_PATH > x_nMaxLength )
		x_nMaxLength = disk::MAXPATHLEN;

	long trim = 0;
	str::t_string s, sub;
	
	// Attempt to allocate space
	try { s.resize( x_nMaxLength ); } 
	catch( ... ) { return str::t_string(); }

	// Get writable pointer
	str::t_char8* pStr = &s[ 0 ];
		
	// Get the folder
	switch( x_nFolderId )
	{
		case eFidNone :
			return str::t_string8();

		case eFidTemp :
			s.resize( ::GetTempPath( x_nMaxLength, pStr ) );
			return s;

		case eFidSystem :
			s.resize( ::GetSystemDirectory( pStr, x_nMaxLength ) );
			return s;

		case eFidUserOs :
			s.resize( ::GetWindowsDirectory( pStr, x_nMaxLength ) );
			return s;

		case eFidCurrent :
			s.resize( ::GetCurrentDirectory( x_nMaxLength, pStr ) );
			return s;

		case eFidDefDrive :
			s.resize( cmn::Min( (UINT)3, ::GetWindowsDirectory( pStr, x_nMaxLength ) ) );
			return s;

		case eFidRoot :
			x_nFolderId = CSIDL_DRIVES;
			break;

		case eFidSettings :
			x_nFolderId = x_bShared ? CSIDL_COMMON_APPDATA : CSIDL_APPDATA;
			break;

		case eFidDesktop :
			x_nFolderId = x_bShared ? CSIDL_COMMON_DESKTOPDIRECTORY : CSIDL_DESKTOPDIRECTORY;
			break;

		case eFidDownloads :
			// +++ CSIDL_COMMON_DOCUMENTS is broken?
			trim = x_bShared ? 1 : 0;
			x_nFolderId = x_bShared ? CSIDL_COMMON_DESKTOPDIRECTORY : CSIDL_MYDOCUMENTS;
			sub = tcT( "Downloads" );
			break;

		case eFidRecycle :
			x_nFolderId = CSIDL_BITBUCKET;
			break;

		case eFidTemplates :
			x_nFolderId = x_bShared ? CSIDL_COMMON_TEMPLATES : CSIDL_TEMPLATES;
			break;

		case eFidPublic :
			// +++ CSIDL_COMMON_DOCUMENTS is broken?
			trim = 1; sub = tcT( "Public" );
			x_nFolderId = x_bShared ? CSIDL_COMMON_DESKTOPDIRECTORY : CSIDL_DESKTOPDIRECTORY;
			break;

		case eFidDocuments :
			// +++ CSIDL_COMMON_DOCUMENTS is broken?
			if ( x_bShared ) trim = 1, sub = tcT( "Documents" );
			x_nFolderId = x_bShared ? CSIDL_COMMON_DESKTOPDIRECTORY : CSIDL_MYDOCUMENTS;
			break;

		case eFidMusic :
			x_nFolderId = x_bShared ? CSIDL_COMMON_MUSIC : CSIDL_MYMUSIC;
			break;

		case eFidPictures :
			x_nFolderId = x_bShared ? CSIDL_COMMON_PICTURES : CSIDL_MYPICTURES;
			break;

		case eFidVideo :
			x_nFolderId = x_bShared ? CSIDL_COMMON_VIDEO : CSIDL_MYVIDEO;
			break;

		case eFidFavorites :
			x_nFolderId = x_bShared ? CSIDL_COMMON_FAVORITES : CSIDL_FAVORITES;
			break;

		case eFidStartMenu :
			x_nFolderId = x_bShared ? CSIDL_COMMON_STARTMENU : CSIDL_STARTMENU;
			break;

		case eFidStartup :
			x_nFolderId = x_bShared ? CSIDL_COMMON_STARTUP : CSIDL_STARTUP;
			break;

		case eFidCookies :
			x_nFolderId = CSIDL_COOKIES;
			break;

		case eFidNetwork :
			x_nFolderId = CSIDL_NETWORK;
			break;

		case eFidPrinters :
			x_nFolderId = CSIDL_PRINTERS;
			break;

		case eFidRecent :
			x_nFolderId = CSIDL_RECENT;
			break;

		case eFidHistory :
			x_nFolderId = CSIDL_HISTORY;
			break;

		case eFidFonts :
			x_nFolderId = CSIDL_FONTS;
			break;

		default :
			break;

	} // end switch

#if defined( HTM_USE_DYNAMIC_SHELL32 )

	// Functions
	pfn_SHGetMalloc pSHGetMalloc = tcNULL;
	pfn_SHGetPathFromIDList pSHGetPathFromIDList = tcNULL;
	pfn_SHGetSpecialFolderLocation pSHGetSpecialFolderLocation = tcNULL;

	// Load shell32.dll
	HMODULE hShell32 = LoadLibrary( tcT( "shell32.dll" ) );
	if ( !hShell32 )
		return str::t_string();

	// Load functions
	pSHGetMalloc = (pfn_SHGetMalloc)GetProcAddress( hShell32, tcT( "SHGetMalloc" ) );
	pSHGetPathFromIDList = (pfn_SHGetPathFromIDList)GetProcAddress( hShell32, tcT( "SHGetPathFromIDList" ) );
	pSHGetSpecialFolderLocation = (pfn_SHGetSpecialFolderLocation)GetProcAddress( hShell32, tcT( "SHGetSpecialFolderLocation" ) );

	// Did we get the functions?
	if ( !pSHGetMalloc || !pSHGetPathFromIDList || !pSHGetSpecialFolderLocation )
	{	FreeLibrary( hShell32 );
		return str::t_string();
	} // end if

#endif

	// +++ Add support for SHGetKnownFolderPath()

	t_LPITEMIDLIST pidl = tcNULL;
	if ( pSHGetSpecialFolderLocation( tcNULL, x_nFolderId, &pidl ) == NOERROR && pidl )
	{
		// Get the path name
		if ( !pSHGetPathFromIDList( pidl, pStr ) )
			s.clear();

		// Free the memory
		t_LPMALLOC pMalloc;
		if ( pSHGetMalloc( &pMalloc ) == NOERROR )
			pMalloc->Free( pidl );

	} // end if


#if defined( HTM_USE_DYNAMIC_SHELL32 )

	// Unload shell lib
	FreeLibrary( hShell32 );

#endif

	// Ensure length is valid
	s.resize( str::Length( pStr, x_nMaxLength ) );

	// Trim path
	while ( trim )
		s = GetPath( s ), trim--;

	// Is there a sub directory?
	if ( sub.length() )
		return FilePath( s, sub );

	return s;
}


str::t_string GetDriveTypeStr( const str::t_string &x_sDrive )
{
	switch( GetDriveType( x_sDrive.c_str() ) )
	{	case DRIVE_NO_ROOT_DIR : 	// return tcT( "noroot" ); break;
		case DRIVE_REMOVABLE : 		return tcT( "removable" ); break;
		case DRIVE_FIXED : 			return tcT( "fixed" ); break;
		case DRIVE_REMOTE :			return tcT( "remote" ); break;
		case DRIVE_CDROM :			return tcT( "cdrom" ); break;
		case DRIVE_RAMDISK :		return tcT( "ramdisk" ); break;
		default : break;	
	} // end switch

	return tcT( "unknown" );
}


long GetDiskInfo( t_pb &pb, const str::t_string &x_sDrive )
{
	// Sanity check
	if ( !x_sDrive.length() ) 
		return 0;

	pb[ tcT( "drive" ) ] = x_sDrive;
	pb[ tcT( "drive_type" ) ] = GetDriveTypeStr( x_sDrive.c_str() );
	pb[ tcT( "drive_type_os" ) ] = GetDriveTypeStr( x_sDrive.c_str() );
	
	// Get volume information
	DWORD dwSn = 0, dwMax = 0, dwFlags = 0;
	char szVolume[ 1024 * 8 ] = { 0 }, szFileSystem[ 1024 * 8 ] = { 0 };
	if ( GetVolumeInformation(	x_sDrive.c_str(), szVolume, sizeof( szVolume ),
								&dwSn, &dwMax, &dwFlags,
								szFileSystem, sizeof( szFileSystem ) ) )
	{	pb[ tcT( "volume" ) ] = tcMb2Str( szVolume );
		pb[ tcT( "serial" ) ] = dwSn;
		pb[ tcT( "max_filename" ) ] = dwMax;
		pb[ tcT( "flags" ) ] = dwFlags;
		pb[ tcT( "file_system" ) ] = tcMb2Str( szFileSystem );
	} // end if

	// More disk info
	DWORD dwSectorsPerCluster = 0, dwBytesPerSector = 0, dwFreeClusters = 0, dwClusters = 0;
	if ( GetDiskFreeSpace( x_sDrive.c_str(), &dwSectorsPerCluster, &dwBytesPerSector, &dwFreeClusters, &dwClusters ) )
	{	pb[ tcT( "sectors_per_cluster" ) ] = dwSectorsPerCluster;
		pb[ tcT( "bytes_per_sector" ) ] = dwBytesPerSector;
		pb[ tcT( "clusters_free" ) ] = dwFreeClusters;
		pb[ tcT( "clusters" ) ] = dwClusters;
	} // end if
	
	// Get disk space
	ULARGE_INTEGER liFreeBytesAvailable, liTotalNumberOfBytes, liTotalNumberOfBytesFree;
	if ( GetDiskFreeSpaceEx( x_sDrive.c_str(), &liFreeBytesAvailable, &liTotalNumberOfBytes, &liTotalNumberOfBytesFree ) )
	{	pb[ tcT( "bytes" ) ] = liTotalNumberOfBytes.QuadPart;
		pb[ tcT( "bytes_free" ) ] = liTotalNumberOfBytesFree.QuadPart;
		pb[ tcT( "bytes_used" ) ] = liTotalNumberOfBytes.QuadPart - liTotalNumberOfBytesFree.QuadPart;
		pb[ tcT( "bytes_available" ) ] = liFreeBytesAvailable.QuadPart;
		pb[ tcT( "bytes_unavailable" ) ] = liTotalNumberOfBytes.QuadPart - liFreeBytesAvailable.QuadPart;
	} // end if
	
	// Get the dos name
	TCHAR buf[ MAX_PATH ] = { 0 };
	DWORD dw = QueryDosDevice( x_sDrive.c_str(), buf, MAX_PATH );
	if ( dw && dw < sizeof( buf ) )
		buf[ dw ] = 0, pb[ tcT( "dos_name" ) ] = buf;
	
	return 1;
}

long GetDisksInfo( t_pb &pb, bool bInfo )
{
	long lTotal = 0;
	TCHAR szDrive[ 16 ] = tcT( "A:" );
	DWORD dwDrives = GetLogicalDrives(), dw = 1;

	// Build a list of drives
	while ( dwDrives && dw && szDrive[ 0 ] <= tcT( 'Z' ) )
	{
		// Get info on this drive if it exists
		if ( dwDrives & dw )
		{
			lTotal++;
		
			if ( bInfo )
				GetDiskInfo( pb[ szDrive ], szDrive );
			else
				pb[ szDrive ] = GetDriveTypeStr( szDrive );

		} // end if

		// Next drive position
		dwDrives &= ~dw; 
		dw <<= 1;
		szDrive[ 0 ]++;

	} // end while
	
	return lTotal;
}


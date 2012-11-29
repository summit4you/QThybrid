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

#include <stdlib.h>

#include <fcntl.h>
#include <stropts.h>

#if !defined( HTM_NOSTATFS )
#	include <sys/statfs.h>
#elif !defined( HTM_NOSTATVFS )
#	include <sys/statvfs.h>
#endif

#if !defined( HTM_NOPWD )
#	include <pwd.h>
#endif

#if !defined( HTM_NOHDREG )
#	include <linux/hdreg.h> 
#endif

#if !defined( HTM_NOMNTENT )
#	include <mntent.h>
#endif

HFILE Open( const char *x_pFile, const char *x_pMode )
{
#if defined( CII_NOSTAT64 )
	tHFILE res = fopen( x_pFile, x_pMode );
#else
	tHFILE res = fopen64( x_pFile, x_pMode );
#endif
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
	if ( !x_pPath || !*x_pPath )
		return false;

	bool bRet = !::mkdir( x_pPath, 0755 ) ? true : false;
	if ( bRet )
	{	::chmod( x_pPath, 0755 );
		return true;
	} // end if

	return false;
}

bool exists( const char *x_pPath )
{
#if defined( CII_NOSTAT64 )
	struct stat s;
	return ::stat( x_pPath, &s ) ? false : true;
#else
	struct stat64 s64;
	return	::stat64( x_pPath, &s64 ) ? false : true;
#endif
}

bool unlink( const char *x_pFile )
{
	if ( !x_pFile )
		return false;

	::chmod( x_pFile, 0xffff );
	return ::unlink( x_pFile ) ? false : true;
}

bool rmdir( const char *x_pPath )
{
	if ( !x_pPath )
		return false;

	::chmod( x_pPath, 0xffff );
	return ::rmdir( x_pPath ) ? false : true;
}

str::tc_int64 Size( const char *x_pFile )
{
	if ( !x_pFile || !*x_pFile )
		return 0;

#if defined( CII_NOSTAT64 )

	struct stat s;
	if ( stat( x_pFile, &s ) )
		return 0;

	return s.st_size;

#else

	// +++ Ensure this works correctly
	struct stat64 s64;
	if ( stat64( x_pFile, &s64 ) )
		return 0;

	return s64.st_size;

#endif
}

str::tc_int64 Size( HFILE x_hFile )
{
	// Ensure valid handle
	if ( c_invalid_hfile == x_hFile ) 
		return 0;

#if defined( CII_NOSTAT64 )

	int size = 0;
	int pos = ftell( (tHFILE)x_hFile );
	if ( !fseek( (tHFILE)x_hFile, 0, SEEK_END ) )
		size = ftell( (tHFILE)x_hFile ),
		fseek( (tHFILE)x_hFile, pos, SEEK_SET );
	return size;	

#else

	off64_t size = 0;
	off64_t pos = ftello64( (tHFILE)x_hFile );
	if ( !fseeko64( (tHFILE)x_hFile, 0, SEEK_END ) )
		size = ftello64( (tHFILE)x_hFile ),
		fseeko64( (tHFILE)x_hFile, pos, SEEK_SET );
	return (str::tc_int64)size;	

#endif
}


static void disk_InitFindData( SFindData *x_pFd )
{	x_pFd->llSize = 0;
    *x_pFd->szName = 0;
    x_pFd->uFileAttributes = 0;
    x_pFd->ftCreated = 0;
    x_pFd->ftLastAccess = 0;
    x_pFd->ftLastModified = 0;
}

static void disk_SetFindData( SFindData *x_pFd, const dirent *x_pD )
{
	if ( ! x_pFd || !x_pD )
		return;

	// Init structure
	disk_InitFindData( x_pFd );

	// Save file name
	zstr::Copy( x_pFd->szName, sizeof( x_pFd->szName ), x_pD->d_name );

	// Is it a directory?
	if ( DT_DIR & x_pD->d_type )
		x_pFd->uFileAttributes |= disk::eFileAttribDirectory;
		
	// If size is required
	if ( x_pFd->orgPath )
	{
		// Doing this instead of str::FilePath() for speed, 
		// Not happy size isn't in dirent anyway :(
		char path[ 1024 ] = { 0 };
		t_size sz = zstr::Copy( path, sizeof( path ), x_pFd->orgPath );
		path[ sz++ ] = '/';
		if ( sz < sizeof( path ) )
		{	zstr::Copy( &path[ sz ], sizeof( path ), x_pD->d_name );
			x_pFd->llSize = Size( path );
		} // end if

	} // end if
}

HFIND FindFirst( const char *x_pPath, const char *x_pMask, SFindData *x_pFd, unsigned long x_uReqFlags )
{
#if defined( CII_NODIRENT )
	return c_invalid_hfind;
#else

    // Sanity checks
    if ( !x_pPath || ! x_pMask || ! x_pFd )
        return c_invalid_hfind;

	// Open find
	DIR *hDir = opendir( x_pPath );
	if ( !hDir )
		return c_invalid_hfind;

	// Skip . and ..
	struct dirent *pD;
	do
	{	errno = 0;
		pD = readdir( hDir );
		if ( !pD || errno )
		{	closedir( hDir );
			return c_invalid_hfind;
		} // end if

	} while ( isDotPath( pD->d_name ) );

	// Set file data
	disk_SetFindData( x_pFd, pD );

	// +++ Hack to get file size
	x_pFd->orgPath = ( 0 != ( x_uReqFlags & eReqSize ) ) ? x_pPath : 0;

    return (HFIND)hDir;
#endif
}

bool FindNext( HFIND x_hFind, SFindData *x_pFd )
{
#if defined( CII_NODIRENT )
	return false;
#else
	DIR *hDir = (DIR*)x_hFind;

	struct dirent *pD;
	do
	{
		errno = 0;
		pD = readdir( hDir );
		if ( !pD || errno )
			return false;

	} while ( isDotPath( pD->d_name ) );
		
	disk_SetFindData( x_pFd, pD );

    return true;
#endif
}

bool FindClose( HFIND x_hFind )
{
#if defined( CII_NODIRENT )
	return false;
#else
	return closedir( (DIR*)x_hFind ) ? false : true;
#endif
}

static str::t_string disk_GetHome()
{
	// First attempt to user environment
	const char *pHome = getenv( "HOME" );
	if ( pHome && *pHome )
		return tcMb2Str( pHome );
		
#if !defined( HTM_NOPWD )
	// Attempt to read user home directory
	struct passwd *pw = getpwuid( getuid() );
	if ( pw && pw->pw_dir && *pw->pw_dir )
		return tcMb2Str( pw->pw_dir );
#endif

	return tcT( "~/" );
}

#define disk_FROMHOME( s ) disk::FilePath< str::t_string >( disk_GetHome(), tcT( s ) )

str::t_string GetSysFolder( bool x_bShared, long x_nFolderId, long x_nMaxLength )
{
	// Get the folder
	switch( x_nFolderId )
	{
		case eFidNone :
			break;

		case eFidTemp :
		{
			const char *pTmp;
			if ( !( pTmp = getenv( "TMPDIR" ) ) 
				 && !( pTmp = getenv( "TEMP" ) )
				 && !( pTmp = getenv( "TMP" ) ) 
			   )
				return tcT( "/tmp" );

			return tcMb2Str( pTmp );

		} break;

		case eFidSystem :
			return disk_GetHome();

		case eFidUserOs :
			return tcT( "/sys" );

		case eFidCurrent :
		{
			// Allocate space
			str::t_string8 s;
			try { s.resize( disk::MAXPATHLEN ); }
			catch( ... ) { return str::t_string(); }
			
			// Ask for the current working directory
			if ( !getcwd( &s[ 0 ], disk::MAXPATHLEN ) || !s[ 0 ] )
				return str::t_string();
			
			// Set string length
			s.resize( str::Length( s.c_str(), disk::MAXPATHLEN ) );

			return tcMb2Str( s );

		} break;

		case eFidRoot :
		case eFidDefDrive :
			return tcT( "/" );

		case eFidFonts :
			return tcT( "/usr/share/fonts/truetype/msttcorefonts" );

		case eFidSettings :
			if ( x_bShared )
				return tcT( "/var/lib" );
			return disk_FROMHOME( ".config" );

		case eFidDesktop :
			return disk_FROMHOME( "Desktop" );

		case eFidDownloads :
			return disk_FROMHOME( "Downloads" );

		case eFidTemplates :
			return disk_FROMHOME( "Templates" );

		case eFidPublic :
			return disk_FROMHOME( "Public" );

		case eFidDocuments :
			return disk_FROMHOME( "Documents" );

		case eFidMusic :
			return disk_FROMHOME( "Music" );

		case eFidPictures :
			return disk_FROMHOME( "Pictures" );

		case eFidVideo :
			return disk_FROMHOME( "Videos" );

		default :
			break;

	} // end switch

	return tcT( "" );
}

str::t_string GetDriveTypeStr( const str::t_string &x_sDrive )
{
	if ( !x_sDrive.length() )	 			return tcT( "noroot" );
	
	if ( x_sDrive == tcT( "fd" ) ) 			return tcT( "removable" );
	
	if ( x_sDrive == tcT( "hd" ) ) 			return tcT( "fixed" );
	if ( x_sDrive == tcT( "ext" ) ) 		return tcT( "fixed" );
	if ( x_sDrive == tcT( "ext2" ) ) 		return tcT( "fixed" );
	if ( x_sDrive == tcT( "ext3" ) ) 		return tcT( "fixed" );
	if ( x_sDrive == tcT( "fuseblk" ) ) 	return tcT( "fixed" );
	if ( x_sDrive == tcT( "ecryptfs" ) ) 	return tcT( "fixed" );	
	
	if ( x_sDrive == tcT( "cdrom" ) ) 		return tcT( "cdrom" );
	
	if ( x_sDrive == tcT( "ram" ) ) 		return tcT( "ramdisk" );
	if ( x_sDrive == tcT( "tmpfs" ) ) 		return tcT( "ramdisk" );
	if ( x_sDrive == tcT( "tempfs" ) ) 		return tcT( "ramdisk" );
	if ( x_sDrive == tcT( "devtmpfs" ) ) 	return tcT( "ramdisk" );
	
	if ( x_sDrive == tcT( "devpts" ) )	 	return tcT( "remote" );
	if ( x_sDrive == tcT( "subst" ) ) 		return tcT( "remote" );
	if ( x_sDrive == tcT( "join" ) ) 		return tcT( "remote" );
	if ( x_sDrive == tcT( "net" ) ) 		return tcT( "remote" );

	return tcT( "unknown" );
}

long GetDiskInfo( t_pb &pb, const str::t_string &x_sDrive )
{
	// Sanity check
	if ( !x_sDrive.length() )
		return 0;

	pb[ tcT( "drive" ) ] = x_sDrive;
//	pb[ tcT( "drive_type" ) ] = GetDriveTypeStr( x_sDrive.c_str() );

#if !defined( HTM_NOSTATFS )

	struct statfs di;
    if ( 0 > statfs( tcStr2Mb( x_sDrive ).c_str(), &di ) )
    	return 0;

	pb[ tcT( "file_system_type" ) ] = (unsigned int)di.f_type;
	pb[ tcT( "file_system_str" ) ] = GetFsTypeStr( di.f_type );
	pb[ tcT( "file_system_id32" ) ] = (unsigned int)di.f_fsid.__val[ 0 ];
	pb[ tcT( "file_system_id64" ) ] = *(str::tc_uint64*)&di.f_fsid;
	
#elif !defined( HTM_NOSTATVFS )

	struct statvfs di;
    if ( 0 > statvfs( x_sDrive.c_str(), &di ) )
    	return 0;
	
	pb[ tcT( "flags" ) ] = di.f_flag;
	pb[ tcT( "max_filename" ) ] = di.f_namemax;
//	pb[ tcT( "file_system_type" ) ] = di.f_type;
//	pb[ tcT( "file_system_str" ) ] = GetFsTypeStr( di.f_type );
	pb[ tcT( "file_system_id32" ) ] = (unsigned int)di.f_fsid;
//	pb[ tcT( "file_system_id64" ) ] = (str::tc_uint64)di.f_fsid64;
	
#endif
   	
    // Space info
	pb[ tcT( "bytes" ) ] = (str::tc_uint64)di.f_blocks * (str::tc_uint64)di.f_bsize;
	pb[ tcT( "bytes_free" ) ] = (str::tc_uint64)di.f_bfree * (str::tc_uint64)di.f_bsize;
	pb[ tcT( "bytes_used" ) ] = (str::tc_uint64)( di.f_blocks - di.f_bfree ) * (str::tc_uint64)di.f_bsize;
	pb[ tcT( "bytes_available" ) ] = (str::tc_uint64)di.f_bavail * (str::tc_uint64)di.f_bsize;
	pb[ tcT( "bytes_unavailable" ) ] = (str::tc_uint64)( di.f_blocks - di.f_bavail ) * (str::tc_uint64)di.f_bsize;
	
	pb[ tcT( "inodes" ) ] = di.f_files;	
	pb[ tcT( "block_size" ) ] = di.f_bsize;	
	pb[ tcT( "fragment_size" ) ] = di.f_frsize;	

	// +++ Hmmmm
	if ( di.f_blocks && di.f_bsize
		 && pb[ tcT( "drive_type" ) ].ToString() == "unknown" 
		 && pb[ tcT( "file_system_str" ) ].ToString() != "TMPFS" )
		pb[ tcT( "drive_type" ) ] = tcT( "fixed" );

	return 1;
}

long GetDisksInfo( t_pb &pb, bool bInfo )
{
	long lTotal = 0;
#if !defined( HTM_NOMNTENT )

	struct mntent *m;
	FILE *f = setmntent( "/proc/mounts", "r" );
	while ( ( m = getmntent( f ) ) )
		if ( m )
		{
			str::t_string sPath = m->mnt_dir 
								  ? tcMb2Str( m->mnt_dir ) 
								  : str::ToString< str::t_string >( lTotal );
			
			
			if ( bInfo )
			{
				t_pb &r = pb[ sPath ];
				r[ tcT( "volume" ) ] = tcMb2Str( m->mnt_fsname ? m->mnt_fsname : "" );
				r[ tcT( "drive_type" ) ] = GetDriveTypeStr( tcMb2Str( m->mnt_type ? m->mnt_type : "" ) );
				r[ tcT( "drive_type_os" ) ] = tcMb2Str( m->mnt_type ? m->mnt_type : "" );
				r[ tcT( "drive_path" ) ] = tcMb2Str( m->mnt_dir ? m->mnt_dir : "" );
				r[ tcT( "drive_fs" ) ] = tcMb2Str( m->mnt_fsname ? m->mnt_fsname : "" );
				r[ tcT( "drive_opts" ) ] = tcMb2Str( m->mnt_opts ? m->mnt_opts : "" );
				r[ tcT( "drive_freq" ) ] = m->mnt_freq;
				r[ tcT( "drive_passno" ) ] = m->mnt_passno;

				// Get extended info				
				GetDiskInfo( r, sPath );

			} // end if
			
			else
				pb[ sPath ] = GetDriveTypeStr( tcMb2Str( m->mnt_type ? m->mnt_type : "" ) );

			lTotal++;
		
		} // end while
	
	endmntent( f );

#endif

	return lTotal;
}




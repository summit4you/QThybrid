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

#include <stdio.h>
#include <string>
#include <string.h>
#include "htmapp.h"

#if defined( _WIN32 )
#	include <windows.h>
#else
#	include <errno.h>
#	include <sys/stat.h>
#	include <sys/types.h>
#	if !defined( CII_NODIRENT )
#		include <dirent.h>
#	endif
#endif

namespace disk
{
	typedef FILE* tHFILE;

#if defined( _WIN32 )
#	include "disk-windows.hpp"
#else
#	include "disk-posix.hpp"
#endif

bool isDotPath(  const char *p )
{
	return p[ 0 ] == '.' && ( !p[ 1 ] || ( p[ 1 ] == '.' && !p[ 2 ] ) );
}

str::t_string GetFsTypeStr( unsigned long type )
{
	switch( type )
	{
		case 0x0000ADF5	: return tcT( "ADFS" );
		case 0x0000ADFF	: return tcT( "AFFS" );
		case 0x42465331	: return tcT( "BEFS" );
		case 0x1badface : return tcT( "BFS" );
		case 0xFF534D42	: return tcT( "CIFS" );
		case 0x73757245	: return tcT( "CODA" );
		case 0x012FF7B7 : return tcT( "COH" );
		case 0x00001373 : return tcT( "DEVFS" );
		case 0x00414A53 : return tcT( "EFS" );
		case 0x0000137D : return tcT( "EXT" );
		case 0x0000EF51 : return tcT( "EXT2 old" );
		case 0x0000EF52 : return tcT( "EXT2" );
		case 0x0000EF53 : return tcT( "EXT3" );
		case 0x00004244 : return tcT( "HFS" );
		case 0xF995E849 : return tcT( "HPFS" );
		case 0x958458f6 : return tcT( "HUGETLBFS" );
		case 0x00009660 : return tcT( "ISOFS" );
		case 0x000072b6 : return tcT( "JFFS2" );
		case 0x3153464a : return tcT( "JFS" );
		case 0x0000137F : return tcT( "MINIX" );
		case 0x0000138F : return tcT( "MINIX-30" );
		case 0x00002468 : return tcT( "MINIX2" );
		case 0x00002478 : return tcT( "MINIX2-30" );
		case 0x00004d44 : return tcT( "MSDOS" );
		case 0x0000564c : return tcT( "NCP" );
		case 0x00006969 : return tcT( "NFS" );
		case 0x5346544e : return tcT( "NTFS" );
		case 0x00009fa1 : return tcT( "OPENPROM" );
		case 0x00009fa0 : return tcT( "PROC" );
		case 0x0000002f : return tcT( "QNX4" );
		case 0x52654973 : return tcT( "REISERFS" );
		case 0x00007275 : return tcT( "ROMFS" );
		case 0x0000517B : return tcT( "SMB" );
		case 0x012FF7B6 : return tcT( "SYSV2" );
		case 0x012FF7B5 : return tcT( "SYSV4" );
		case 0x01021994 : return tcT( "TMPFS" );
		case 0x15013346 : return tcT( "UDF" );
		case 0x00011954 : return tcT( "UFS" );
		case 0x00009fa2 : return tcT( "USBDEV" );
		case 0xa501FCF5 : return tcT( "VXFS" );
		case 0x012FF7B4 : return tcT( "XENIX" );
		case 0x58465342 : return tcT( "XFS" );
		case 0x012FD16D : return tcT( "XIAFS" );

	} // end switch	

	return tcT( "unknown" );
}

}; // namespace disk


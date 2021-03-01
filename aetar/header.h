//
//	aegis - project change supervisor
//	Copyright (C) 2002, 2004-2006, 2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#ifndef AETAR_HEADER_H
#define AETAR_HEADER_H

#include <common/nstring.h>

#define TBLOCK 512
#define NAMSIZ 100

//
// The magic field is filled with this if uname and gname are valid.
//
#define	TMAGIC		"ustar  "	// 7 chars and a null

//
// The linkflag defines the type of file
//
#define	LF_OLDNORMAL	'\0'		// Normal disk file, Unix compat
#define	LF_NORMAL	'0'		// Normal disk file
#define	LF_LINK		'1'		// Link to previously dumped file
#define	LF_SYMLINK	'2'		// Symbolic link
#define	LF_CHR		'3'		// Character special file
#define	LF_BLK		'4'		// Block special file
#define	LF_DIR		'5'		// Directory
#define	LF_FIFO		'6'		// FIFO special file
#define	LF_CONTIG	'7'		// Contiguous file
#define LF_LONGNAME	'L'		// File is actually long name
					// for next file in the archive.
#define LF_LONGLINK	'K'		// File is actually long link
					// for next file in the archive.
// Further link types may be defined later.


struct header_ty
{
    char	name[NAMSIZ];
    char	mode[8];
    char	uid[8];
    char	gid[8];
    char	size[12];
    char	mtime[12];
    char	checksum[8];
    char	linkflag;
    char	linkname[NAMSIZ];
    char	magic[8];
    char	uname[32];
    char	gname[32];
    char	devmajor[8];
    char	devminor[8];
};

nstring header_name_get(header_ty *);
void header_name_set(header_ty *hp, const nstring &arg);
void header_name_set(header_ty *, const nstring &);
long header_mode_get(header_ty *);
void header_mode_set(header_ty *, long);
long header_uid_get(header_ty *);
void header_uid_set(header_ty *, long);
long header_gid_get(header_ty *);
void header_gid_set(header_ty *, long);
long header_size_get(header_ty *);
void header_size_set(header_ty *, long);
long header_mtime_get(header_ty *);
void header_mtime_set(header_ty *, long);
long header_checksum_get(header_ty *);
void header_checksum_set(header_ty *, long);
int header_linkflag_get(header_ty *);
void header_linkflag_set(header_ty *, int);
nstring header_linkname_get(header_ty *);
void header_linkname_set(header_ty *hp, const nstring &arg);
nstring header_uname_get(header_ty *);
void header_uname_set(header_ty *hp, const nstring &arg);
nstring header_gname_get(header_ty *);
void header_gname_set(header_ty *hp, const nstring &arg);
long header_devmajor_get(header_ty *);
void header_devmajor_set(header_ty *, long);
long header_devminor_get(header_ty *);
void header_devminor_set(header_ty *, long);
long header_checksum_calculate(header_ty *);
void header_dump(header_ty *);

#endif // AETAR_HEADER_H

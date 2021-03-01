//
//	aegis - project change supervisor
//	Copyright (C) 2002, 2004, 2005 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: interface definition for aetar/header.c
//

#ifndef AETAR_HEADER_H
#define AETAR_HEADER_H

#include <str.h>

class nstring; // forward

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

string_ty *header_name_get(header_ty *);
void header_name_set(header_ty *, string_ty *);
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
string_ty *header_linkname_get(header_ty *);
void header_linkname_set(header_ty *, string_ty *);
void header_linkname_set(header_ty *, const nstring &);
string_ty *header_uname_get(header_ty *);
void header_uname_set(header_ty *, string_ty *);
void header_uname_set(header_ty *, const nstring &);
string_ty *header_gname_get(header_ty *);
void header_gname_set(header_ty *, string_ty *);
void header_gname_set(header_ty *, const nstring &);
long header_devmajor_get(header_ty *);
void header_devmajor_set(header_ty *, long);
long header_devminor_get(header_ty *);
void header_devminor_set(header_ty *, long);
long header_checksum_calculate(header_ty *);
void header_dump(header_ty *);

#endif // AETAR_HEADER_H

/*
 *	aegis - project change supervisor
 *	Copyright (C) 1998 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: interface definition for libaegis/fopen_nfs.c
 */

#ifndef LIBAEGIS_FOPEN_NFS_H
#define LIBAEGIS_FOPEN_NFS_H

#include <ac/stdio.h>

#include <main.h>

FILE *fopen_with_stale_nfs_retry _((const char *, const char *));

#endif /* LIBAEGIS_FOPEN_NFS_H */

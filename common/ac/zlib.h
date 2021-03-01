/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2004 Peter Miller;
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
 * MANIFEST: interface definition for common/ac/zlib.c
 */

#ifndef COMMON_AC_ZLIB_H
#define COMMON_AC_ZLIB_H

#include <config.h>

#if HAVE_ZLIB_H
#include <zlib.h>
#else
#error "You must have zlib installed in order to build Aegis."
#endif

/* there is ERR_MSG in zutil.h, but we aren't allowed to see it. */
#include <main.h>
const char *z_error(int);

#ifndef DEF_MEM_LEVEL
#define DEF_MEM_LEVEL 8 /* in zutil.h, but we aren't allowed to see it. */
#endif

#endif /* COMMON_AC_ZLIB_H */

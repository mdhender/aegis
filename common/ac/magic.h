/*
 *	aegis - project change supervisor
 *	Copyright (C) 2003, 2004 Peter Miller;
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
 * MANIFEST: interface definition for common/ac/magic.c
 */

#ifndef COMMON_AC_MAGIC_H
#define COMMON_AC_MAGIC_H

#include <config.h>

#if HAVE_MAGIC_H
extern "C" {
#include <magic.h>
}
#else
typedef void *magic_t;
magic_t magic_open(int flags);
void magic_close(magic_t cookie);
const char *magic_file(magic_t cookie, const char *filename);
const char *magic_error(magic_t cookie);
#define MAGIC_MIME 0
#endif

#endif /* COMMON_AC_MAGIC_H */

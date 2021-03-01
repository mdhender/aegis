/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996 Peter Miller;
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
 * MANIFEST: interface definition for aegis/io.c
 */

#ifndef AEGIS_IO_H
#define AEGIS_IO_H

#include <ac/time.h>

#include <main.h>
#include <type.h>

struct sub_context_ty; /* existence */

#define INTEGER_NOT_SET 0
#define TIME_NOT_SET (time_t)0

void string_write _((char *, string_ty *));
void integer_write _((char *, long));
void time_write _((char *, time_t));
void io_comment_append _((struct sub_context_ty *, char *));
void io_comment_emit _((void));

#endif /* AEGIS_IO_H */

/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993 Peter Miller.
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
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * MANIFEST: interface definition for common/error.c
 */

#ifndef ERROR_H
#define ERROR_H

#include <main.h>

void error _((char *, ...));
void fatal _((char *, ...));
void verbose _((char *, ...));

void nerror _((char *, ...));
void nfatal _((char *, ...));

typedef void (*quit_ty)_((int));
void quit_register _((quit_ty));
void quit _((int));

char *signal_name _((int));

int assert_failed _((char *condition, char *file, int line));
#ifdef DEBUG
# if defined(__STDC__) || defined(__stdc__)
#  define assert(c) ((void)((c) != 0 || assert_failed(#c, __FILE__, __LINE__)))
# else
#  define assert(c) ((void)((c) != 0 || assert_failed("c", __FILE__, __LINE__)))
# endif
#else
# define assert(c)
#endif

typedef void (*error_id_ty)_((int *uid, int *gid));

void error_set_id_func _((error_id_ty));

#endif /* ERROR_H */

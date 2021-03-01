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
 * MANIFEST: interface definition for common/mem.c
 */

#ifndef MEM_H
#define MEM_H

#include <stddef.h>
#include <main.h>

char *mem_alloc _((size_t));
char *mem_alloc_clear _((size_t));
void mem_change_size _((char **, size_t));
void mem_free _((char *));
char *enlarge _((size_t *, char **, size_t));
char *mem_copy_string _((char *));

#endif /* MEM_H */

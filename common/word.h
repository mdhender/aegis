/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1994 Peter Miller.
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
 * MANIFEST: interface definition for common/word.c
 */

#ifndef WORD_H
#define WORD_H

#include <str.h>

typedef struct wlist wlist;
struct wlist
{
	size_t		wl_nwords;
	size_t		wl_nwords_max;
	string_ty	**wl_word;
};

int wl_member _((wlist *, string_ty *));
string_ty *wl2str _((wlist *, int, int, char *));
void str2wl _((wlist *, string_ty *, char *, int));
void wl_prepend _((wlist *, string_ty *));
void wl_append _((wlist *, string_ty *));
void wl_append_unique _((wlist *, string_ty *));
void wl_copy _((wlist *, wlist *));
void wl_delete _((wlist *, string_ty *));
void wl_free _((wlist *));
void wl_zero _((wlist *));

int wl_equal _((wlist *, wlist *));
int wl_subset _((wlist *, wlist *));

#endif /* WORD_H */

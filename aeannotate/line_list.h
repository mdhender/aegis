/*
 *	aegis - project change supervisor
 *	Copyright (C) 2002 Peter Miller;
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
 * MANIFEST: interface definition for aeannotate/line_list.c
 */

#ifndef AEANNOTATE_LINE_LIST_H
#define AEANNOTATE_LINE_LIST_H

#include <line.h>

typedef struct line_list_t line_list_t;
struct line_list_t
{
    size_t	    maximum;
    size_t	    start1;
    size_t	    length1;
    size_t	    start2;
    size_t	    length2;
    line_t	    *item;
};

void line_list_constructor _((line_list_t *));
void line_list_destructor _((line_list_t *));
void line_list_clear _((line_list_t *));
void line_list_delete _((line_list_t *, size_t, size_t));
void line_list_insert _((line_list_t *, size_t, struct change_ty *,
    struct string_ty *));

#endif /* AEANNOTATE_LINE_LIST_H */

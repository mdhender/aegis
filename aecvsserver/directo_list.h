/*
 *	aegis - project change supervisor
 *	Copyright (C) 2004 Peter Miller;
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
 * MANIFEST: interface definition for aecvsserver/directo_list.c
 */

#ifndef AECVSSERVER_DIRECTO_LIST_H
#define AECVSSERVER_DIRECTO_LIST_H

#include <directory.h>

typedef struct directory_list_ty directory_list_ty;
struct directory_list_ty
{
    size_t          length;
    size_t          maximum;
    directory_ty    *item;
};

void directory_list_constructor(directory_list_ty *);
void directory_list_destructor(directory_list_ty *);
void directory_list_rewind(directory_list_ty *);
void directory_list_append(directory_list_ty *, string_ty *, string_ty *);

#endif /* AECVSSERVER_DIRECTO_LIST_H */
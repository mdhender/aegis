/*
 *	aegis - project change supervisor
 *	Copyright (C) 2003 Peter Miller;
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
 * MANIFEST: interface definition for libaegis/change/list.c
 */

#ifndef LIBAEGIS_CHANGE_LIST_H
#define LIBAEGIS_CHANGE_LIST_H

#include <main.h>

typedef struct change_list_ty change_list_ty;
struct change_list_ty
{
    size_t          length;
    size_t          maximum;
    struct change_ty **item;
};


/**
  * The change_list_new function is used to create a new change list is
  * dynamic memory.
  */
change_list_ty *change_list_new(void);

/**
  * The change_list_delete function is used to release a change
  * list dynamic memory resources.  The changes themselves are NOT
  * change_free()ed.
  */
void change_list_delete(change_list_ty *clp);

/**
  * The change_list_append function is used to append a change to a
  * change list.
  */
void change_list_append(change_list_ty *clp, change_ty *cp);

/**
  * The change_list_member_p function is used to determine if a given
  * change is a member of the given change list.
  */
int change_list_member_p(change_list_ty *clp, change_ty *cp1);

#endif /* LIBAEGIS_CHANGE_LIST_H */

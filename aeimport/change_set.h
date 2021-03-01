/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001, 2002 Peter Miller;
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
 * MANIFEST: interface definition for aeimport/change_set.c
 */

#ifndef AEIMPORT_CHANGE_SET_H
#define AEIMPORT_CHANGE_SET_H

#include <ac/time.h>

#include <str_list.h>
#include <change_set/file_list.h>

typedef struct change_set_ty change_set_ty;
struct change_set_ty
{
    string_ty       *who;
    time_t	    when;
    string_ty       *description;
    change_set_file_list_ty file;
    string_list_ty  tag;
};

change_set_ty *change_set_new(void);
void change_set_delete(change_set_ty *);
void change_set_validate(change_set_ty *);

#endif /* AEIMPORT_CHANGE_SET_H */

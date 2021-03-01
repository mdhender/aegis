/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001 Peter Miller;
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
 * MANIFEST: interface definition for aeimport/change_set/file_list.c
 */

#ifndef AEIMPORT_CHANGE_SET_FILE_LIST_H
#define AEIMPORT_CHANGE_SET_FILE_LIST_H

#include <change_set/file.h>

typedef struct change_set_file_list_ty change_set_file_list_ty;
struct change_set_file_list_ty
{
	size_t		length;
	size_t		maximum;
	change_set_file_ty *item;
};

void change_set_file_list_constructor _((change_set_file_list_ty *));
void change_set_file_list_destructor _((change_set_file_list_ty *));
void change_set_file_list_append _((change_set_file_list_ty *, string_ty *,
	string_ty *, change_set_file_action_ty, string_list_ty *));
void change_set_file_list_validate _((change_set_file_list_ty *));

#endif /* AEIMPORT_CHANGE_SET_FILE_LIST_H */
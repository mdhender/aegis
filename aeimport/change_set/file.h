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
 * MANIFEST: interface definition for aeimport/change_set/file.c
 */

#ifndef AEIMPORT_CHANGE_SET_FILE_H
#define AEIMPORT_CHANGE_SET_FILE_H

#include <str_list.h>

enum change_set_file_action_ty
{
	change_set_file_action_create,
	change_set_file_action_modify,
	change_set_file_action_remove
};
typedef enum change_set_file_action_ty change_set_file_action_ty;

typedef struct change_set_file_ty change_set_file_ty;
struct change_set_file_ty
{
	string_ty	*filename;
	string_ty	*edit;
	change_set_file_action_ty action;
	string_list_ty	tag;
};

void change_set_file_constructor _((change_set_file_ty *, string_ty *,
	string_ty *, change_set_file_action_ty, string_list_ty *));
void change_set_file_destructor _((change_set_file_ty *));
const char *change_set_file_action_name _((change_set_file_action_ty));
void change_set_file_validate _((change_set_file_ty *));

#endif /* AEIMPORT_CHANGE_SET_FILE_H */

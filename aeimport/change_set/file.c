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
 * MANIFEST: functions to manipulate files
 */

#include <change_set/file.h>


void
change_set_file_constructor(change_set_file_ty *csfp, string_ty *filename,
    string_ty *edit, change_set_file_action_ty action, string_list_ty *tag)
{
    csfp->filename = str_copy(filename);
    csfp->edit = str_copy(edit);
    csfp->action = action;
    string_list_copy(&csfp->tag, tag);
}


void
change_set_file_destructor(change_set_file_ty *csfp)
{
    str_free(csfp->filename);
    csfp->filename = 0;
    str_free(csfp->edit);
    csfp->edit = 0;
    string_list_destructor(&csfp->tag);
}


#ifdef DEBUG

void
change_set_file_validate(change_set_file_ty *csfp)
{
    str_validate(csfp->filename);
    str_validate(csfp->edit);
    string_list_validate(&csfp->tag);
}

#endif


const char *
change_set_file_action_name(change_set_file_action_ty n)
{
    switch (n)
    {
    case change_set_file_action_create:
	return "create";

    case change_set_file_action_modify:
	return "modify";

    case change_set_file_action_remove:
	return "remove";
    }
    return "unknown";
}

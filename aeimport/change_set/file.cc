//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2002, 2004 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to manipulate files
//

#include <change_set/file.h>
#include <error.h> // assert


change_set_file_ty::change_set_file_ty(string_ty *arg1, string_ty *arg2,
	change_set_file_action_ty arg3, string_list_ty *arg4) :
    filename(str_copy(arg1)),
    edit(str_copy(arg2)),
    action(arg3),
    tag(*arg4)
{
}


change_set_file_ty::~change_set_file_ty()
{
    str_free(filename);
    filename = 0;
    str_free(edit);
    edit = 0;
}


#ifdef DEBUG

void
change_set_file_ty::validate()
    const
{
    assert(str_validate(filename));
    assert(str_validate(edit));
    assert(tag.validate());
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


void
change_set_file_ty::merge(const change_set_file_ty &arg)
{
    str_free(edit);
    edit = str_copy(arg.edit);
    action = arg.action;
    tag.push_back_unique(arg.tag);
}

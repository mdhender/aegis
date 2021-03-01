//
//	aegis - project change supervisor
//	Copyright (C) 2006 Peter Miller;
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
// MANIFEST: implementation of the validation_text class
//

#include <libaegis/change/file.h>
#include <libaegis/fstate.h>
#include <libaegis/input/file.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>

#include <aede-policy/validation/text.h>


validation_text::~validation_text()
{
}


validation_text::validation_text()
{
}


bool
validation_text::run(change_ty *cp)
{
    bool result = true;
    for (size_t j = 0; ; ++j)
    {
	fstate_src_ty *src = change_file_nth(cp, j, view_path_first);
	if (!src)
	    break;
	switch (src->action)
	{
	case file_action_create:
	case file_action_modify:
	    if (!check(cp, src))
		result = false;
	    break;

	case file_action_remove:
	case file_action_insulate:
	case file_action_transparent:
	    break;
	}
    }
    return result;
}


bool
validation_text::check(change_ty *cp, fstate_src_ty *src)
    const
{
    //
    // Note: we return true if the file is acceptable to the policy
    // (i.e. has no NUL characters).  We return false if the file is
    // unacceptable (is binary).
    //
    string_ty *path = change_file_path(cp, src);
    // assert(path);
    if (!path)
	return true;

    os_become_orig();
    bool result = true;
    input ip = input_file_open(path);
    for (;;)
    {
	int c = ip->getch();
	if (c < 0)
	    break;
	if (c == 0)
	{
	    result = false;
	    break;
	}
    }
    ip.close();
    os_become_undo();
    if (!result)
    {
	sub_context_ty sc;
	sc.var_set_string("File_Name", src->file_name);
	change_error(cp, &sc, i18n("$filename: is binary"));
    }
    return result;
}

//
//	aegis - project change supervisor
//	Copyright (C) 2005, 2006 Peter Miller;
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
// MANIFEST: implementation of the validation_crlf class
//

#include <libaegis/change/file.h>
#include <libaegis/fstate.h>
#include <libaegis/input/file.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>

#include <aede-policy/validation/crlf.h>


validation_crlf::~validation_crlf()
{
}


validation_crlf::validation_crlf()
{
}


bool
validation_crlf::run(change_ty *cp)
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
validation_crlf::check(change_ty *cp, fstate_src_ty *src)
    const
{
    string_ty *path = change_file_path(cp, src);
    // assert(path);
    if (!path)
	return true;

    os_become_orig();
    bool crlf_seen = false;
    bool nul_seen = false;
    input ip = input_file_open(path);
    int state = 0;
    for (;;)
    {
	int c = ip->getch();
	if (c < 0)
	    break;
	if (c == 0)
	{
	    //
            // These days, the only difference between text files
            // and binary files is the presence of NUL characters.
            // All other characters can be part of an international
            // character set encoding.
	    //
	    nul_seen = true;
	}
	if (state == '\r' && c == '\n')
	    crlf_seen = true;
	state = c;
    }
    ip.close();
    os_become_undo();
    bool result = nul_seen || !crlf_seen;
    if (!result)
    {
	sub_context_ty sc;
	sc.var_set_string("File_Name", src->file_name);
	change_error(cp, &sc, i18n("$filename: dos line termination"));
    }
    return result;
}

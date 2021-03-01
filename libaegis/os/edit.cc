//
//	aegis - project change supervisor
//	Copyright (C) 2002-2004 Peter Miller;
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
// MANIFEST: functions to manipulate edits
//

#include <ac/string.h>

#include <arglex2.h>
#include <error.h> // for assert
#include <os.h>
#include <sub.h>
#include <user.h>


void
os_edit(const nstring &filename, edit_ty et)
{
    string_ty       *cmd;
    string_ty       *cwd;
    string_ty       *editor;

    //
    // find the editor to use
    //
    os_become_must_not_be_active();
    if (et == edit_background)
    {
	editor = user_editor_command((user_ty*)0);
    }
    else
    {
	char	*cp;
	editor = user_visual_command((user_ty*)0);

	cp = strrchr(editor->str_text, '/');
	if (!cp)
		cp = editor->str_text;
	if (0 == strcmp(cp, "ed"))
		et = edit_background;
    }

    //
    // make sure we are in a position to edit
    //
    if (et != edit_background && os_background())
    {
	sub_context_ty sc;
	sc.var_set_charstar("Name", arglex_token_name(arglex_token_edit));
	sc.fatal_intl(i18n("may not use $name in the background"));
	// NOTREACHED
    }

    //
    // edit the file
    //
    // Please note: we ignore the exit status on purpose.
    // This is because vi (amongst others) returns a silly exit status.
    //
    os_become_orig();
    cmd = str_format("%s %s", editor->str_text, filename.c_str());
    cwd = os_curdir();
    assert(cwd);
    os_execute(cmd, OS_EXEC_FLAG_INPUT | OS_EXEC_FLAG_ERROK, cwd);
    os_become_undo();
    str_free(cmd);
    str_free(cwd);
}


void
os_edit(string_ty *filename, edit_ty et)
{
    os_edit(nstring(filename), et);
}

//
//	aegis - project change supervisor
//	Copyright (C) 2006 Peter Miller
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
// MANIFEST: implementation of the project_ty::get_the_owner method
//

#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/sub.h>


void
project_ty::get_the_owner()
{
    //
    // If the project owner is already established,
    // don't do anything here.
    //
    if (uid >= 0 && gid >= 0)
	return;

    //
    // When checking the project home path owner, we append "/."
    // to force the automounter to mount it.  Some automounters
    // don't actually trigger until the directory lookup happens,
    // which gives nasty results when we stat the directory to see
    // who ownes it (the automounter frequently returns 0,0 which
    // gives a "tampering" error).
    //
    os_become_orig();
    string_ty *s = str_format("%s/.", home_path_get()->str_text);
    os_owner_query(s, &uid, &gid);
    str_free(s);
    os_become_undo();

    //
    // Make sure the project UID and GID are acceptable.  This mirrors
    // the tests in aegis/aenpr.c when the project is first created.
    //
    if (uid < AEGIS_MIN_UID)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_string(scp, "File_Name", home_path_get());
	sub_var_set_long(scp, "Number1", uid);
	sub_var_set_long(scp, "Number2", AEGIS_MIN_UID);
	fatal_intl
	(
	    scp,
	    i18n("$filename: uid $number1 invalid, must be >= $number2")
	);
	// NOTREACHED
	sub_context_delete(scp);
    }
    if (gid < AEGIS_MIN_GID)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_string(scp, "File_Name", home_path_get());
	sub_var_set_long(scp, "Number1", gid);
	sub_var_set_long(scp, "Number2", AEGIS_MIN_GID);
	fatal_intl
	(
	    scp,
	    i18n("$filename: gid $number1 invalid, must be >= $number2")
	);
	// NOTREACHED
	sub_context_delete(scp);
    }
}

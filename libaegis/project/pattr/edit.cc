//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2001-2006, 2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <common/error.h> // for assert
#include <libaegis/commit.h>
#include <libaegis/pattr.h>
#include <libaegis/project/pattr/edit.h>
#include <libaegis/sub.h>
#include <libaegis/undo.h>


void
project_pattr_edit(pattr_ty **dp, edit_ty et)
{
    sub_context_ty  *scp;
    pattr_ty	    *d;
    string_ty       *filename;
    string_ty       *msg;

    //
    // write attributes to temporary file
    //
    d = *dp;
    assert(d);
    filename = os_edit_filename(1);
    os_become_orig();
    pattr_write_file(filename, d, 0);
    pattr_type.free(d);

    //
    // error message to issue if anything goes wrong
    //
    scp = sub_context_new();
    sub_var_set_string(scp, "File_Name", filename);
    msg = subst_intl(scp, i18n("attributes in $filename"));
    sub_context_delete(scp);
    undo_message(msg);
    str_free(msg);
    os_become_undo();

    //
    // edit the file
    //
    os_edit(filename, et);

    //
    // read it in again
    //
    os_become_orig();
    d = pattr_read_file(filename);
    commit_unlink_errok(filename);
    os_become_undo();
    str_free(filename);
    *dp = d;
}

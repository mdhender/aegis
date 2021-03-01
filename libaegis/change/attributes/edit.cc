//
//	aegis - project change supervisor
//	Copyright (C) 2000-2008 Peter Miller
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

#include <common/error.h>
#include <common/language.h>
#include <libaegis/cattr.h>
#include <libaegis/change/attributes.h>
#include <libaegis/commit.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>
#include <libaegis/undo.h>


void
change_attributes_edit(cattr_ty **dp, int et)
{
    //
    // write attributes to temporary file
    //
    cattr_ty *d = *dp;
    assert(d);
    string_ty *filename = os_edit_filename(1);
    os_become_orig();
    language_human();
    cattr_write_file(filename, d, 0);
    language_C();
    cattr_type.free(d);

    //
    // an error message to issue if anything goes wrong
    //
    sub_context_ty sc;
    sc.var_set_string("File_Name", filename);
    string_ty *msg = sc.subst_intl(i18n("attributes in $filename"));
    undo_message(msg);
    str_free(msg);
    os_become_undo();

    //
    // edit the file
    //
    os_edit(filename, (edit_ty)et);

    //
    // read it in again
    //
    os_become_orig();
    d = cattr_read_file(filename);
    commit_unlink_errok(filename);
    os_become_undo();
    change_attributes_fixup(d);
    change_attributes_verify(filename, d);
    str_free(filename);
    *dp = d;
}

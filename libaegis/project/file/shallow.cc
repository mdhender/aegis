//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2001-2008 Peter Miller
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

#include <libaegis/change/file.h>
#include <common/error.h> // for assert
#include <libaegis/project/file.h>
#include <common/trace.h>


//
// If you change the logic in this function, make sure you also change
// project_file_shallow_check to match, see libaegis/project/file/shallow_chk.c
//

void
project_file_shallow(project_ty *pp, string_ty *file_name, long cn)
{
    change::pointer pcp;
    fstate_src_ty   *src1_data;
    fstate_src_ty   *src2_data;

    //
    // Will never be zero, because already magic zero encoded.
    //
    trace(("project_file_shallow(pp = %08lX, file_name = \"%s\", cn=%ld)\n{\n",
	(long)pp, file_name->str_text, cn));
    assert(cn);

    //
    // If there is no parent project,
    // nothing more needs to be done.
    //
    if (pp->is_a_trunk())
    {
	trace(("shallowing \"%s\" no grandparent\n", file_name->str_text));
	trace(("}\n"));
	return;
    }

    //
    // Look for the file in the project.
    // If it is there, nothing more needs to be done.
    //
    pcp = pp->change_get();
    src1_data = change_file_find(pcp, file_name, view_path_first);
    if (src1_data)
    {
	trace(("shallowing \"%s\" already in %s\n", file_name->str_text,
	    project_name_get(pp)->str_text));
	trace(("}\n"));
	return;
    }

    //
    // The file is not part of the immediate project, see if
    // we can find it in one of the grand*parent projects.
    // If it is not there, nothing more needs to be done.
    //
    src2_data = project_file_find(pp, file_name, view_path_simple);
    if (!src2_data)
    {
	trace(("shallowing \"%s\" not in %s ancestor\n", file_name->str_text,
	    project_name_get(pp)->str_text));
	trace(("}\n"));
	return;
    }

    //
    // Create a new file in the project, and mark it "about to be
    // copied".  That way we can throw it away again, if the
    // review fails or the integration fails.
    //
    trace(("shallowing \"%s\"\n", file_name->str_text));
    src1_data = pcp->file_new(src2_data);
    src1_data->action = file_action_transparent;
    src1_data->about_to_be_copied_by = cn;

    //
    // The value here is bogus (can't use the old one, it refers to
    // the wrong branch).  The values only lasts long enough to be
    // replaced (ipass) or removed (ifail).
    //
    if (src2_data->deleted_by)
	src1_data->deleted_by = cn;

    //
    // As a branch advances, the edit field tracks the
    // history, but the edit_origin field is the number when
    // the file was first created or copied into the branch.
    // By definition, a file in a change is out of date when
    // it's edit_origin field does not equal the edit field
    // of its project.
    //
    // In order to merge branches, this must be done as a
    // cross branch merge in a change to that branch; the
    // edit_origin_new field of the change is copied
    // into the edit_origin field of the branch.
    //
    // branch's edit
    //     The head revision of the branch.
    // branch's edit_origin
    //     The version originally copied.
    //
    assert(src2_data->edit);
    if (src1_data->edit)
	history_version_type.free(src1_data->edit);
    if (src2_data->edit)
	src1_data->edit = history_version_copy(src2_data->edit);
    else
    {
	src1_data->edit = (history_version_ty *)history_version_type.alloc();
	src1_data->edit->revision = str_from_c("bogus");
    }
    if (src1_data->edit_origin)
	history_version_type.free(src1_data->edit_origin);
    src1_data->edit_origin = history_version_copy(src1_data->edit);

    //
    // pull the testing correlations across
    //
    if (src2_data->test && src2_data->test->length)
    {
	size_t          j;

	src1_data->test =
            (fstate_src_test_list_ty *)fstate_src_test_list_type.alloc();
	for (j = 0; j < src2_data->test->length; ++j)
	{
	    string_ty       **addr_p;
	    meta_type *type_p = 0;

	    addr_p =
		(string_ty **)
		fstate_src_test_list_type.list_parse(src1_data->test, &type_p);
	    assert(type_p = &string_type);
	    *addr_p = str_copy(src2_data->test->list[j]);
	}
    }

    //
    // Note: locked_by is deliberatey dropped,
    // because it refers to a different branch.
    //
    trace(("}\n"));
}

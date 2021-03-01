//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2001-2006, 2008 Peter Miller
//	Copyright (C) 2006 Walter Franzini
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
#include <libaegis/project.h>
#include <libaegis/project/file.h>
#include <common/trace.h>


int
change_file_up_to_date(project_ty *pp, fstate_src_ty *c_src_data)
{
    fstate_src_ty   *p_src_data;
    int             result;

    //
    // No edit origin implies a new file, and is always
    // up-to-date.
    //
    trace(("change_file_up_to_date(pp = %08lX)\n{\n", (long)pp));
    trace(("filename = \"%s\";\n", c_src_data->file_name->str_text));
    assert(!c_src_data->edit || c_src_data->edit->revision);
    assert(!c_src_data->edit_origin || c_src_data->edit_origin->revision);
    if (!c_src_data->edit_origin)
    {
	trace(("return 1;\n"));
	trace(("}\n"));
	return 1;
    }

    //
    // Look for the file in the project.  If it is not there, it
    // implies a new file, which is always up-to-date.
    //
    p_src_data = project_file_find(pp, c_src_data, view_path_extreme);
    if (!p_src_data || !p_src_data->edit)
    {
	trace(("return 1;\n"));
	trace(("}\n"));
	return 1;
    }

    //
    // The file is out-of-date if the edit number of the file in the
    // project is not the same as the edit number of the file when
    // originally copied from the project.
    //
    // p_src_data->edit
    //     The head revision of the branch.
    // p_src_data->edit_origin
    //     The version originally copied.
    //
    // c_src_data->edit
    //     Not meaningful until after integrate pass.
    // c_src_data->edit_origin
    //     The version originally copied.
    // c_src_data->edit_origin_new
    //     Updates branch edit_origin on integrate pass.
    //
    assert(p_src_data->edit);
    assert(p_src_data->edit->revision);
    assert(c_src_data->edit_origin);
    assert(c_src_data->edit_origin->revision);
    result =
	(
	    (
		// backwards compatibility
		!p_src_data->uuid
	    ||
		// backwards compatibility
		!c_src_data->uuid
	    ||
		// make sure the file hasn't been renamed in the mean time
		str_equal(p_src_data->uuid, c_src_data->uuid)
	    )
	&&
	    str_equal
	    (
		p_src_data->edit->revision,
		c_src_data->edit_origin->revision
	    )
	);
    trace(("return %d;\n", result));
    trace(("}\n"));
    return result;
}

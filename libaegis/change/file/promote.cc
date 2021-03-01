//
//	aegis - project change supervisor
//	Copyright (C) 2005 Peter Miller;
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
// MANIFEST: implementation of the change_file_promote class
//

#include <libaegis/change/file.h>
#include <common/error.h> // for assert
#include <common/nstring/list.h>
#include <libaegis/project/file.h>
#include <libaegis/sub.h>
#include <common/trace.h>


bool
change_file_promote(change_ty *cp)
{
    trace(("change_file_promote(cp = %08lX)\n{\n", (long)cp));
    bool changed = false;
    nstring_list remove_later;
    for (size_t j = 0; ; ++j)
    {
	//
	// find the relevant change src data
	//
	fstate_src_ty *c_src_data = change_file_nth(cp, j, view_path_first);
	if (!c_src_data)
	    break;
	trace(("c_src_data = %08lX\n", (long)c_src_data));
	trace(("action = %s\n", file_action_ename(c_src_data->action)));
	trace(("usage = %s\n", file_usage_ename(c_src_data->usage)));

	//
	// locate the equivalent project file
	//
	fstate_src_ty *p_src_data =
	    project_file_find(cp->pp, c_src_data->file_name, view_path_extreme);
	trace(("p_src_data = %08lX\n", (long)p_src_data));

	////////////////////////////////////////////////////////////////
        //                                                            //
        // FIXME: we have to cope with the case where files have      //
        // been MOVED in the baseline, not just created and deleted.  //
        // A moved file we are modifying means the file in the        //
        // development directory must be physicall moved to reflect   //
        // the change in the baseline.                                //
        //                                                            //
        // There is a similar issue with removing a file that has     //
        // been renamed from under the change, you have to switch to  //
        // removing the new name.                                     //
        //                                                            //
	////////////////////////////////////////////////////////////////

	//
	// Look for inconsistencies.
	//
	switch (c_src_data->action)
	{
	case file_action_create:
	    //
	    // check if someone created it ahead of you
	    //
	    if (p_src_data)
	    {
		trace(("action = %s\n", file_action_ename(c_src_data->action)));
		trace(("usage = %s\n", file_usage_ename(c_src_data->usage)));
		switch (p_src_data->action)
		{
		case file_action_create:
		case file_action_modify:
		case file_action_insulate:
		    {
			trace(("create -> modify\n"));
			changed = true;

			sub_context_ty sc;
			sc.var_set_string("File_Name", c_src_data->file_name);
			change_verbose
			(
			    cp,
			    &sc,
			    i18n("warning: $filename in baseline, copying")
			);

			c_src_data->action = file_action_modify;
			if (p_src_data->edit_origin)
			{
			    c_src_data->edit_origin =
				history_version_copy(p_src_data->edit);
			}
			if (!c_src_data->uuid && p_src_data->uuid)
			    c_src_data->uuid = str_copy(p_src_data->uuid);
			if (!str_equal(c_src_data->uuid, p_src_data->uuid))
			{
			    // fixme: we come here if an UUID clash is
			    // present (the previous test failed).  Must
			    // think a better way to handle it.
			    assert(0);
			}

			//
			// must also remove ,D fingerprint, to force a new diff
			//
			if (c_src_data->diff_file_fp)
			{
			    fingerprint_type.free(c_src_data->diff_file_fp);
			    c_src_data->diff_file_fp = 0;
			}
		    }
		    break;

		case file_action_transparent:
		case file_action_remove:
		    // Do nothing.
		    break;
		}
	    }
	    break;

	case file_action_remove:
	    //
	    // check if someone deleted it ahead of you
	    //
	    if (!p_src_data)
	    {
		sub_context_ty sc;
		sc.var_set_string("File_Name", c_src_data->file_name);
		change_verbose(cp, &sc, i18n("no $filename in baseline"));
		remove_later.push_back(nstring(c_src_data->file_name));
	    }
	    else if
    	    (
		c_src_data->edit_origin
	    &&
		c_src_data->edit_origin->revision
	    &&
		p_src_data->edit_origin
	    &&
		p_src_data->edit_origin->revision
	    &&
		!str_equal
		(
		    c_src_data->edit_origin->revision,
		    p_src_data->edit_origin->revision
		)
	    )
	    {
		//
		// Update the version number and other attributes.
		// The file will still be removed, of course.
		//
		trace(("updating removed version number\n"));
		changed = true;
		change_file_copy_basic_attributes(c_src_data, p_src_data);
		assert(p_src_data->edit);
		assert(p_src_data->edit->revision);
		if (c_src_data->edit_origin)
		    history_version_type.free(c_src_data->edit_origin);
		c_src_data->edit_origin =
		    history_version_copy(p_src_data->edit);
	    }
	    break;

	case file_action_modify:
	case file_action_insulate:
	    //
	    // Check that nobody has
	    // deleted it from under you.
	    //
	    if (!p_src_data)
	    {
		trace(("modify -> create\n"));
		sub_context_ty sc;
		sc.var_set_string("File_Name", c_src_data->file_name);
		change_verbose
		(
		    cp,
		    &sc,
		    i18n("warning: no $filename in baseline, creating")
		);

		c_src_data->action = file_action_create;
		changed = true;

                //
                // Must also invalidate the ,D fingerprint, to force a
                // new difference to be performed.
		//
		if (c_src_data->diff_file_fp)
		{
		    fingerprint_type.free(c_src_data->diff_file_fp);
		    c_src_data->diff_file_fp = 0;
		}
	    }
	    break;

	case file_action_transparent:
	    break;
	}
    }
    for (size_t k = 0; k < remove_later.size(); ++k)
    {
	const nstring &file_name = remove_later[k];
	trace(("remove \"%s\"\n", file_name.c_str()));
	change_file_remove(cp, file_name.get_ref());
	changed = true;
    }
    trace(("return %s;\n", (changed ? "true" : "false")));
    trace(("}\n"));
    return changed;
}

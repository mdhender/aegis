//
//	aegis - project change supervisor
//	Copyright (C) 2005-2007 Peter Miller
//	Copyright (C) 2006 Walter Franzini
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
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//
// MANIFEST: implementation of the change_file_unchanged function
//

#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>
#include <common/error.h> // for assert
#include <libaegis/file.h>
#include <common/nstring.h>
#include <libaegis/os.h>
#include <libaegis/project/file.h>
#include <libaegis/user.h>


bool
change_file_unchanged(change::pointer cp, fstate_src_ty *src_data,
    user_ty::pointer up)
{
    //
    // The policy only applies to simple change sets, not to branches.
    //
    if (change_was_a_branch(cp))
	return false;

    //
    // Leave cross branch merges alone, even if they
    // don't change anything.  The information is
    // about the merge, not the file contents.
    //
    if (src_data->edit_origin_new)
    {
	assert(src_data->edit_origin_new->revision);
	return false;
    }

    //
    // Only modified files are of interest.
    //
    switch (src_data->action)
    {
    case file_action_modify:
    case file_action_insulate:
	break;

    case file_action_create:
	if (src_data->move)
	    break;
	return false;

    case file_action_remove:
    case file_action_transparent:
	return false;
    }
    switch (src_data->usage)
    {
    case file_usage_build:
	return false;

    case file_usage_config:
    case file_usage_source:
    case file_usage_test:
    case file_usage_manual_test:
	break;
    }

    //
    // The file could have vanished from under us,
    // so make sure this is sensable.
    //
    fstate_src_ty *psrc_data =
	project_file_find(cp->pp, src_data, view_path_extreme);
    if (!psrc_data)
	return false;

    //
    // Get the path of the project file to be compared against.
    //
    int blf_unlink = 0;
    nstring blf(project_file_path(cp->pp, psrc_data->file_name));
    assert(!blf.empty());
    up->become_begin();
    int blf_exists = os_exists(blf);
    up->become_end();
    assert(blf_exists);
    if (!blf_exists)
    {
	blf = nstring(project_file_version_path(cp->pp, src_data, &blf_unlink));
	assert(!blf.empty());
    }

    //
    // Get the path of the change file to be compared against.
    //
    nstring cfp(change_file_path(cp, src_data->file_name));
    assert(cfp);
    if (!cfp)
	return false;

    //
    // If any of the file's attributes have changed, the file is
    // different, too.
    //
    if (!str_equal(src_data->file_name, psrc_data->file_name))
	return false;
    if (src_data->usage != psrc_data->usage)
	return false;
    if
    (
	(src_data->attribute ? src_data->attribute->length : 0)
    !=
	(psrc_data->attribute ? psrc_data->attribute->length : 0)
    )
	return false;
    if (src_data->attribute && src_data->attribute->length)
    {
	assert(psrc_data->attribute);
	for (size_t j = 0; j < src_data->attribute->length; ++j)
	{
	    attributes_ty *p1 = src_data->attribute->list[j];
	    attributes_ty *p2 = psrc_data->attribute->list[j];
	    if (p1 && p2)
	    {
		if (p1->name && p2->name && !str_equal(p1->name, p2->name))
		    return false;
		if (p1->value && p2->value && !str_equal(p1->value, p2->value))
		    return false;
	    }
	}
    }

    //
    // Perform the comparison.
    //
    up->become_begin();
    bool different = files_are_different(cfp, blf);
    up->become_end();
    if (blf_unlink)
    {
	os_become_orig();
	os_unlink(blf);
	os_become_undo();
    }
    return !different;
}
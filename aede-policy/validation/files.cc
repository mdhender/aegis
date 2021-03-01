//
//	aegis - project change supervisor
//	Copyright (C) 2007, 2008 Peter Miller
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

#include <common/nstring.h>
#include <libaegis/attribute.h>
#include <libaegis/change.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>

#include <aede-policy/validation/files.h>


validation_files::~validation_files()
{
}


validation_files::validation_files()
{
}


bool
validation_files::check_branches()
    const
{
    return true;
}


bool
validation_files::check_downloaded()
    const
{
    return true;
}


bool
validation_files::check_foreign_copyright()
    const
{
    return true;
}


bool
validation_files::check_binaries()
    const
{
    return true;
}


bool
validation_files::run(change::pointer cp)
{
    //
    // Don't check branches, only individual changes have control over
    // the copyright year in their source files.
    //
    if (!check_branches() && change_was_a_branch(cp))
	return true;

    //
    // Don't perform this check for changes downloaded and applied by
    // aedist, because the original developer is no longer in control.
    //
    if (!check_downloaded() && was_downloaded(cp))
	return true;

    //
    // Don't perform this check for change sets marked as owning a
    // foreign copyright.
    //
    if
    (
        !check_foreign_copyright()
    &&
        cp->attributes_get_boolean("foreign-copyright")
    )
	return true;

    //
    // Check each file in the change set.
    //
    bool result = true;
    for (size_t j = 0; ; ++j)
    {
	fstate_src_ty *src = change_file_nth(cp, j, view_path_first);
	if (!src)
	    break;

	//
	// Don't perform this check for files marked as owning a
	// foreign copyright.
	//
	if
        (
            !check_foreign_copyright()
        &&
            attributes_list_find_boolean(src->attribute, "foreign-copyright")
        )
	    continue;

	switch (src->action)
	{
	case file_action_create:
	case file_action_modify:
            //
            // Don't look at binary files unless this validation can
            // cope with them.
            //
            if (check_binaries() || is_a_text_file(src, cp))
            {
                if (!check(cp, src))
                    result = false;
            }
	    break;

	case file_action_remove:
	case file_action_insulate:
	case file_action_transparent:
	    break;
	}
    }
    return result;
}

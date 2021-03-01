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
// MANIFEST: implementation of the validation_description class
//

#include <common/ac/string.h>

#include <common/nstring.h>
#include <libaegis/change.h>
#include <libaegis/change/branch.h>
#include <libaegis/cstate.h>
#include <libaegis/sub.h>

#include <aede-policy/validation/description.h>


validation_description::~validation_description()
{
}


validation_description::validation_description()
{
}


bool
validation_description::run(change_ty *cp)
{
    bool result = true;
    cstate_ty *cstate_data = change_cstate_get(cp);

    //
    // Check for the default brief description used by both aenc and tkaenc.
    //
    nstring brief_desc(cstate_data->brief_description);
    if
    (
	brief_desc.empty()
    ||
	brief_desc.starts_with("none")
    ||
	strstr(brief_desc.c_str(), "(clone of")
    )
    {
	change_error(cp, 0, i18n("brief_description not set"));
	result = false;
    }

    nstring desc(cstate_data->description);
    if
    (
        desc.empty()
    ||
	// check the aenc default
	desc.starts_with("none")
    ||
	// check the tkaenc default
	desc.starts_with("This change ...")
    ||
	desc.starts_with("This change...")
    ||
	// check for gross laziness
	(!change_was_a_branch(cp) && desc.starts_with(brief_desc))
    )
    {
	change_error(cp, 0, i18n("description not set"));
	result = false;
    }
    return result;
}

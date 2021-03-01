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
// MANIFEST: implementation of the change_identifi_sub_get_proj_fil class
//

#include <change/identifi_sub.h>
#include <error.h> // for assert
#include <nstring/list.h>
#include <project/file.h>
#include <project/file/roll_forward.h>


void
change_identifier_subset::get_project_file_names(nstring_list &results)
{
    if (need_historical_perspective())
    {
	project_file_roll_forward *hp = get_historian();
	nstring_list file_name_list;
	hp->keys(file_name_list);
	for (size_t j = 0; j < file_name_list.size(); ++j)
	{
	    nstring file_name = file_name_list[j];
	    assert(file_name.length());
	    file_event_ty *fep = hp->get_last(file_name);
	    assert(fep);
	    if (!fep)
		continue;
	    assert(fep->src);
	    switch (fep->src->usage)
	    {
	    case file_usage_build:
		switch (fep->src->action)
		{
		case file_action_modify:
		    continue;

		case file_action_create:
		case file_action_remove:
		case file_action_insulate:
		case file_action_transparent:
		    break;
		}
		// fall through...

	    case file_usage_source:
	    case file_usage_config:
	    case file_usage_test:
	    case file_usage_manual_test:
		switch (fep->src->action)
		{
		case file_action_create:
		case file_action_modify:
		case file_action_remove:
		    break;

		case file_action_insulate:
		case file_action_transparent:
		    // can't happen
		    assert(0);
		    continue;
		}
		break;
	    }
	    results.push_back(nstring(str_copy(fep->src->file_name)));
	}
    }
    else
    {
	for (size_t k = 0;; ++k)
	{
	    fstate_src_ty *src_data =
		project_file_nth(get_pp(), k, view_path_simple);
	    if (!src_data)
		break;
	    switch (src_data->usage)
	    {
	    case file_usage_build:
		switch (src_data->action)
		{
		case file_action_modify:
		    continue;

		case file_action_create:
		case file_action_remove:
		case file_action_insulate:
		case file_action_transparent:
		    break;
		}
		// fall through...

	    case file_usage_source:
	    case file_usage_config:
	    case file_usage_test:
	    case file_usage_manual_test:
		switch (src_data->action)
		{
		case file_action_create:
		case file_action_modify:
		case file_action_remove:
		    break;

		case file_action_insulate:
		case file_action_transparent:
		    // can't happen
		    assert(0);
		    continue;
		}
		break;
	    }
	    results.push_back(nstring(str_copy(src_data->file_name)));
	}
    }
}

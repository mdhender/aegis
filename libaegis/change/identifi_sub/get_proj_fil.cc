//
// aegis - project change supervisor
// Copyright (C) 2005, 2006, 2008, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published
// by the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>

#include <common/nstring/list.h>
#include <libaegis/change/identifi_sub.h>
#include <libaegis/file/event.h>
#include <libaegis/project/file.h>
#include <libaegis/project/file/roll_forward.h>


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
            file_event *fep = hp->get_last(file_name);
            assert(fep);
            if (!fep)
                continue;
            assert(fep->get_src());
            switch (fep->get_src()->usage)
            {
            case file_usage_build:
                switch (fep->get_src()->action)
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
                switch (fep->get_src()->action)
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
            results.push_back(nstring(fep->get_src()->file_name));
        }
    }
    else
    {
        for (size_t k = 0;; ++k)
        {
            fstate_src_ty *src_data = get_pp()->file_nth(k, view_path_simple);
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
            results.push_back(nstring(src_data->file_name));
        }
    }
}


// vim: set ts=8 sw=4 et :

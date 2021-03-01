//
// aegis - project change supervisor
// Copyright (C) 2008 Walter Franzini
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/trace.h>

#include <libaegis/change.h>
#include <libaegis/change/file.h>
#include <libaegis/change/functor/file_find.h>


change_functor_file_find::~change_functor_file_find()
{
}


change_functor_file_find::change_functor_file_find(const nstring_list &arg1)
    : change_functor(true, true)
{
    file_to_uuid.set_reaper();

    //
    // Store in the symtab an empty string for each file name.  This
    // is to decide faster if a name is to be retained or not.  Using
    // the nstring_list::member method has a O(n*m) complexity where n
    // is the size of arg1 and m is the total number of files in the
    // project.
    //
    for (size_t j = 0; j < arg1.size(); ++j)
        file_to_uuid.assign(arg1[j], new nstring());
}


void
change_functor_file_find::operator()(change::pointer cp)
{
    //
    // We only need to consult OPEN changes and branches, since files
    // added by an already closed change are also included by the
    // parent.
    //
    if (cp->is_completed())
        return;

    for (size_t j = 0; ; ++j)
    {
        fstate_src_ty *src_data = change_file_nth(cp, j, view_path_first);

        if (!src_data)
            break;
        if (!src_data->uuid)
            continue;

        switch (src_data->action)
        {
        case file_action_remove:
        case file_action_transparent:
        case file_action_insulate:
        case file_action_modify:
            continue;

        case file_action_create:
            break;
        }

        trace_string(src_data->file_name->str_text);

        //
        //
        nstring *s = file_to_uuid.query(src_data->file_name);
        if (!s)
            continue;

        if (s->empty())
            file_to_uuid.assign
            (
                src_data->file_name,
                new nstring(src_data->uuid)
            );

        trace(("%s => %s\n", src_data->file_name->str_text,
               src_data->uuid->str_text));
    }
}

nstring*
change_functor_file_find::query(const nstring &fn) const
{
    nstring *uuid;
    uuid = file_to_uuid.query(fn);

    //
    // The symtab contains an empty string for files not already
    // present in the repository.
    //
    if (!uuid || uuid->empty())
        return 0;
    return uuid;
}

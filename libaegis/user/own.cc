//
//      aegis - project change supervisor
//      Copyright (C) 2007, 2008, 2011, 2012 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>

#include <common/trace.h>
#include <libaegis/gonzo.h>
#include <libaegis/lock.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/ustate.fmtgen.h>
#include <libaegis/user.h>


ustate_ty *
user_ty::ustate_get(project *pp)
{
    trace(("user_ty::ustate_get(this = %p, pp = %p)\n{\n", this,
        pp));
    lock_sync();
    if (!ustate_path)
    {
        project *ppp = pp->trunk_get();
        nstring pn(project_name_get(ppp));
        ustate_path = gonzo_ustate_path(pn, name());
    }
    if (!ustate_data)
    {
        gonzo_become();
        if (os_exists(ustate_path))
        {
            ustate_data = ustate_read_file(ustate_path);
        }
        else
        {
            ustate_data = (ustate_ty *)ustate_type.alloc();
            ustate_is_new = true;
        }
        gonzo_become_undo();
        if (!ustate_data->own)
        {
            ustate_data->own =
                (ustate_own_list_ty *)ustate_own_list_type.alloc();
        }
    }
    trace(("return %p;\n", ustate_data));
    trace(("}\n"));
    return ustate_data;
}


void
user_ty::own_add(project *pp, long change_number)
{
    trace(("user_ty::own_add(this = %p, project_name = \"%s\", "
        "change_number = %ld)\n{\n", this, project_name_get(pp).c_str(),
        change_number));
    ustate_ty *usp = ustate_get(pp);
    assert(usp->own);

    //
    // See if the project is already known.
    //
    ustate_own_ty *own_data = 0;
    size_t j = 0;
    for (j = 0; j < usp->own->length; ++j)
    {
        own_data = usp->own->list[j];
        if (nstring(own_data->project_name) == project_name_get(pp))
            break;
    }

    //
    // If the project isn't known, append it to the list.
    //
    if (j >= usp->own->length)
    {
        meta_type *type_p = 0;
        ustate_own_ty **own_data_p =
            (ustate_own_ty **)
            ustate_own_list_type.list_parse(usp->own, &type_p);
        assert(type_p == &ustate_own_type);
        own_data = (ustate_own_ty *)ustate_own_type.alloc();
        *own_data_p = own_data;
        own_data->project_name = project_name_get(pp).get_ref_copy();
    }
    assert(own_data);

    //
    // Create a changes for the project, if necessary.
    //
    if (!own_data->changes)
    {
        own_data->changes =
            (ustate_own_changes_list_ty *)ustate_own_changes_list_type.alloc();
    }

    //
    // Add another item to the changes list for the project.
    //
    meta_type *type_p = 0;
    long *change_p =
        (long int *)
        ustate_own_changes_list_type.list_parse(own_data->changes, &type_p);
    assert(type_p == &integer_type);
    *change_p = change_number;
    ustate_modified = true;
    trace(("}\n"));
}


bool
user_ty::own_nth(project *pp, long n, long &change_number)
{
    trace(("user_ty::own_nth(this = %p, project_name = \"%s\", "
        "n = %ld)\n{\n", this, project_name_get(pp).c_str(), n));
    assert(n >= 0);
    if (n < 0)
    {
        trace(("return false;\n"));
        trace(("}\n"));
        return false;
    }
    ustate_ty *usp = ustate_get(pp);
    assert(usp->own);
    if (!usp->own)
    {
        trace(("return false;\n"));
        trace(("}\n"));
        return false;
    }

    //
    // find the relevant project
    //      and extract the n'th change
    //
    for (size_t j = 0; j < usp->own->length; ++j)
    {
        ustate_own_ty *own_data = usp->own->list[j];
        if (project_name_get(pp) == nstring(own_data->project_name))
        {
            bool result = false;
            if (own_data->changes && n < (long)own_data->changes->length)
            {
                change_number = own_data->changes->list[n];
                result = true;
            }
            trace(("return %d;\n", result));
            trace(("}\n"));
            return result;
        }
    }

    trace(("return false;\n"));
    trace(("}\n"));
    return false;
}


void
user_ty::own_remove(project *pp, long change_number)
{
    trace(("user_ty::own_remove(this = %p, pp = %p, cn = %ld)\n{\n",
        this, pp, change_number));
    ustate_ty *usp = ustate_get(pp);
    assert(usp->own);

    //
    // Search for the project in the "own" list.
    //
    ustate_own_ty *own_data = 0;
    size_t j = 0;
    for (j = 0;; ++j)
    {
        if (j >= usp->own->length)
        {
            trace(("}\n"));
            return;
        }
        own_data = usp->own->list[j];
        if (nstring(own_data->project_name) == project_name_get(pp))
            break;
    }

    //
    // Create the "changes" list for the project, if necessary.
    //
    if (!own_data->changes)
    {
        own_data->changes =
            (ustate_own_changes_list_ty *)ustate_own_changes_list_type.alloc();
    }

    //
    // Search for the change in the "changes" list.
    //
    for (size_t k = 0; k < own_data->changes->length; ++k)
    {
        if (own_data->changes->list[k] == change_number)
        {
            own_data->changes->list[k] =
                own_data->changes->list[own_data->changes->length - 1];
            own_data->changes->length--;
            ustate_modified = true;
            break;
        }
    }

    //
    // If the changes list for the project is now empty,
    // remove the project from the "own" list.
    //
    if (!own_data->changes->length)
    {
        ustate_own_type.free(own_data);
        usp->own->list[j] =
            usp->own->list[usp->own->length - 1];
        usp->own->length--;
        ustate_modified = true;
    }
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :

//
// aegis - project change supervisor
// Copyright (C) 1999, 2002-2006, 2008, 2009, 2011, 2012 Peter Miller
// Copyright (C) 2007-2009 Walter Franzini
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
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>

#include <common/nstring.h>
#include <common/symtab.h>
#include <common/trace.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>
#include <libaegis/project.h>
#include <libaegis/view_path/next_change.h>


fstate_src_ty *
change::file_find(const nstring &file_name)
{
    return file_find(file_name, view_path_first);
}


fstate_src_ty *
change::file_find(const nstring &file_name, view_path_ty as_view_path)
{
    trace(("change::file_find(this = %p, file_name = \"%s\", "
        "as_view_path = %s)\n{\n", this, file_name.c_str(),
        view_path_ename(as_view_path)));

    fstate_src_ty *result = 0;
    bool xpar = false;
    bool top_level = !is_a_branch();

    //
    // In order to make Aegis time safe we need to exclude files
    // created in the future with respect to the change cp points to.
    //
    // There is no need to set the limit if as_view_path ==
    // view_path_first because we only need to consult the change
    // record without looking in up to parent branches.
    //
    // There is no need to set the limit if the change is not
    // completed.
    //
    time_t limit = time_limit_get();
    trace_time(limit);

    //
    // To avoid memory exaustion problems we handle all change_ty
    // pointers in the same manner (change_free) so we need to take a
    // copy also of cp.
    //
    change::pointer cp2 = change_copy(this);
    for (;;)
    {
        fstate_src_ty   *fsp;

        trace(("project \"%s\": change %ld\n",
               project_name_get(cp2->pp).c_str(), cp2->number));
        if (cp2->bogus)
            goto next;

        if (as_view_path != view_path_first && cp2->pfstate_get())
        {
            assert(cp2->pfstate_stp);
            fsp = (fstate_src_ty*)cp2->pfstate_stp->query(file_name);
        }
        else
        {
            cp2->fstate_get();
            assert(cp2->fstate_stp);
            fsp = (fstate_src_ty*)cp2->fstate_stp->query(file_name);
        }

        if (fsp)
        {
            trace(("%s \"%s\" %s\n", file_action_ename(fsp->action),
                fsp->file_name->str_text,
                (fsp->edit && fsp->edit->revision ?
                fsp->edit->revision->str_text : "")));

            //
            // If there is a top-level transparent file,
            // it hides the next instance of the file.
            //
            if (xpar)
            {
                fstate_src_type.free(result);
                result = 0;
                xpar = 0;
                goto next;
            }

            //
            // If we already have a "hit" for this file, ignore any
            // deeper records.
            //
            if (result)
                break;

            switch (as_view_path)
            {
            case view_path_first:
                //
                // This means that transparent and removed files
                // are returned.  No project file union, either.
                //
                break;

            case view_path_none:
                //
                // This means that transparent files are returned
                // as transparent, no processing or filtering of
                // the list is performed.
                //
                break;

            case view_path_simple:
            case view_path_extreme:
                switch (fsp->action)
                {
                case file_action_transparent:
                    //
                    // These cases both mean that transparent files
                    // are resolved (the underlying file is shown).
                    //
                    if (top_level)
                        xpar = true;
                    goto next;

                case file_action_remove:
                    //
                    // For now, we keep removed files.
                    //
                    break;

                case file_action_create:
                case file_action_modify:
                case file_action_insulate:
#ifndef DEBUG
                default:
#endif
                    // should be file_action_remove
                    assert(!fsp->deleted_by);
                    if (fsp->deleted_by)
                        break;
                    // should be file_action_transparent
                    assert(!fsp->about_to_be_created_by);
                    if (fsp->about_to_be_created_by)
                        goto next;
                    // should be file_action_transparent
                    assert(!fsp->about_to_be_copied_by);
                    if (fsp->about_to_be_copied_by)
                        goto next;
                    break;
                }
                break;
            }
            fstate_src_type.free(result);
            result = fstate_src_copy(fsp);
        }

        next:
        if (as_view_path == view_path_first)
        {
            change_free(cp2);
            break;
        }
        if (cp2->number == TRUNK_CHANGE_NUMBER)
        {
            change_free(cp2);
            break;
        }
        if
        (
            limit != TIME_NOT_SET
        &&
            cp2->pfstate_get()
        &&
            cp2->pp->is_a_trunk()
        )
        {
            change_free(cp2);
            break;
        }

        change::pointer next_change = view_path_next_change(cp2, limit);
        change_free(cp2);
        cp2 = next_change;

        top_level = false;
    }

    if
    (
        result
    &&
        as_view_path == view_path_extreme
    &&
        result->action == file_action_remove
    )
    {
        fstate_src_type.free(result);
        result = 0;
    }

    assert (!result || result->reference_count > 0);
    trace(("return %p;\n", result));
    trace(("}\n"));
    return result;
}


// vim: set ts=8 sw=4 et :

//
//      aegis - project change supervisor
//      Copyright (C) 2004-2008, 2011, 2012 Peter Miller
//      Copyright (C) 2008 Walter Franzini
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

#include <common/itab.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/list.h>
#include <libaegis/project.h>


void
change_branch_uuid_find(change::pointer cp, string_ty *uuid,
    change_list_ty &result)
{
    cstate_ty *cstate_data = cp->cstate_get();
    if (cstate_data->uuid && str_leading_prefix(cstate_data->uuid, uuid))
    {
        result.append(change_copy(cp));
        if (uuid->str_length == 36)
            return;
    }
    if (cstate_data->branch)
    {
        project *pp2 = cp->pp->bind_branch(cp);
        itab_ty *skip = itab_alloc();

        //
        // If a branch has completed changes then scan that list for a
        // match.
        //
        if (cstate_data->branch->history)
            for (size_t i = 0; i < cstate_data->branch->history->length; ++i)
            {
                cstate_branch_history_ty *bh =
                    cstate_data->branch->history->list[i];

                if (!bh->uuid)
                    continue;

                if (str_leading_prefix(bh->uuid, uuid))
                {
                    change::pointer cp2 = change_alloc(pp2, bh->change_number);
                    change_bind_existing(cp2);
                    result.append(cp2);

                    if (uuid->str_length == 36)
                    {
                        itab_free(skip);
                        return;
                    }

                    //
                    // The change match the requested UUID so there is
                    // no need to check it again in the next loop.
                    //
                    cstate_ty *cstate_data2 = cp2->cstate_get();
                    itab_assign
                    (
                        skip,
                        bh->change_number,
                        (void*)!cstate_data2->branch
                    );
                }
                else
                {
                    switch (bh->is_a_branch)
                    {
                    case cstate_branch_history_is_a_branch_yes:
                    case cstate_branch_history_is_a_branch_unknown:
                        break;

                    case cstate_branch_history_is_a_branch_no:
                        //
                        // We can safely skip the change in the next
                        // loop since we know the UUID will not match.
                        //
                        itab_assign
                        (
                            skip,
                            bh->change_number,
                            (void*)true
                        );
                        break;
                    }
                }
            }

        for (size_t j = 0; j < cstate_data->branch->change->length; ++j)
        {
            long change_number = cstate_data->branch->change->list[j];

            //
            // The change has already been processed, because it's
            // complete, and was *not* a branch, then we can skip it
            // without the need to check the UUID.
            //
            if (itab_query(skip, change_number))
                continue;

            change::pointer cp2 = change_alloc(pp2, change_number);
            change_bind_existing(cp2);
            change_branch_uuid_find(cp2, uuid, result);
            change_free(cp2);
            if (uuid->str_length == 36 && result.size())
            {
                itab_free(skip);
                return;
            }
        }
        itab_free(skip);
    }
}


// vim: set ts=8 sw=4 et :

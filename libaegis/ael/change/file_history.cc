//
// aegis - project change supervisor
// Copyright (C) 2001-2009, 2011, 2012 Peter Miller
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

#include <common/str_list.h>
#include <common/trace.h>
#include <libaegis/ael/change/file_history.h>
#include <libaegis/ael/column_width.h>
#include <libaegis/ael/formeditnum.h>
#include <libaegis/change.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>
#include <libaegis/change/identifier.h>
#include <libaegis/col.h>
#include <libaegis/file/event.h>
#include <libaegis/file/event/list.h>
#include <libaegis/option.h>
#include <libaegis/output.h>
#include <libaegis/project.h>
#include <libaegis/project/file.h>
#include <libaegis/project/file/roll_forward.h>
#include <libaegis/user.h>


#define VERSION_WIDTH 10


static bool
is_a_rename_with_uuid(change::pointer cp, fstate_src_ty *src)
{
    if (src->action != file_action_create)
        return false;
    if (!src->move)
        return false;
    if (!src->uuid)
        return false;
    fstate_src_ty *other = cp->file_find(nstring(src->move), view_path_first);
    if (!other)
        return false;
    if (!other->uuid)
        return false;
    return str_equal(src->uuid, other->uuid);
}


void
list_change_file_history(change_identifier &cid, string_list_ty *)
{
    output::pointer usage_col;
    output::pointer action_col;
    output::pointer when_col;
    output::pointer file_name_col;
    int             j;
    string_ty       *line1;
    output::pointer delta_col;
    output::pointer change_col;
    output::pointer description_col;

    trace(("list_change_file_history()\n{\n"));

    assert(change_file_nth(cid.get_cp(), 0, view_path_none));

    //
    // Reconstruct the project file history.
    //
    time_t when = cid.get_cp()->completion_timestamp();
    project_file_roll_forward historian
    (
        cid.get_pp(),
        when,
        option_verbose_get()
    );

    //
    // create the columns
    //
    col::pointer colp = col::open((string_ty *)0);
    line1 =
        str_format
        (
            "Project \"%s\"  Change %ld",
            project_name_get(cid.get_pp()).c_str(),
            cid.get_change_number()
        );
    colp->title(line1->str_text, "Change File History");
    str_free(line1);

    file_name_col = colp->create(0, 0, "File Name\n-----------");

    int left = 2;
    usage_col = colp->create(left, left + USAGE_WIDTH, 0);
    left += USAGE_WIDTH + 1;
    action_col = colp->create(left, left + ACTION_WIDTH, 0);
    left += ACTION_WIDTH + 1;

    delta_col =
        colp->create(left, left + VERSION_WIDTH, "Delta\n---------");
    left += VERSION_WIDTH + 1;

    when_col =
        colp->create(left, left + WHEN_WIDTH, "Date and Time\n---------------");
    left += WHEN_WIDTH + 1;

    change_col = colp->create(left, left + CHANGE_WIDTH, "Change\n-------");
    left += CHANGE_WIDTH + 1;

    description_col = colp->create(left, 0, "Description\n-------------");

    //
    // list the change's files' histories
    //
    for (j = 0;; ++j)
    {
        fstate_src_ty   *src_data;
        file_event_list::pointer felp;
        size_t          k;
        int             usage_track;
        int             action_track;

        src_data = change_file_nth(cid.get_cp(), j, view_path_first);
        if (!src_data)
            break;
        assert(src_data->file_name);
        colp->need(4);

        string_ty *file_name_track = 0;
        usage_track = -1;
        action_track = -1;

        felp = historian.get(src_data);
        if (felp)
        {
            for (k = 0; k < felp->size(); ++k)
            {
                file_event      *fep;

                fep = felp->get(k);
                assert(fep->get_src());

                //
                // See if the file's name changed.
                //
                if (!str_equal(file_name_track, fep->get_src()->file_name))
                {
                    file_name_col->fputs(fep->get_src()->file_name->str_text);
                    colp->eoln();
                    file_name_track = fep->get_src()->file_name;
                }

                nstring s = fep->get_change()->version_get();
                delta_col->fputs(s.c_str());
                change_col->fprintf("%4ld", fep->get_change()->number);

                if (usage_track != fep->get_src()->usage)
                {
                    usage_col->fputs(file_usage_ename(fep->get_src()->usage));
                    usage_track = fep->get_src()->usage;
                    action_track = -1;
                }
                if (action_track != fep->get_src()->action)
                {
                    if
                    (
                        is_a_rename_with_uuid(fep->get_change(), fep->get_src())
                    )
                    {
                        action_col->fputs("rename");
                        action_track = -1;
                    }
                    else
                    {
                        action_col->fputs
                        (
                            file_action_ename(fep->get_src()->action)
                        );
                        action_track = fep->get_src()->action;
                    }
                }
                time_t when3 = fep->get_when();
                when_col->fputs(ctime(&when3));
                description_col->fputs
                (
                    fep->get_change()->brief_description_get()
                );
                colp->eoln();
            }
        }

        //
        // Now output details of this change, as the "end"
        // of the history.
        //
        if (!cid.get_cp()->is_completed())
        {
            //
            // See if the file's name changed.
            //
            if (!str_equal(file_name_track, src_data->file_name))
            {
                file_name_col->fputs(src_data->file_name->str_text);
                colp->eoln();
            }

            delta_col->fputs(cid.get_cp()->version_get());
            change_col->fprintf("%4ld", cid.get_change_number());
            if (usage_track != src_data->usage)
            {
                usage_col->fputs(file_usage_ename(src_data->usage));
                action_track = -1;
            }
            if (action_track != src_data->action)
            {
                if (is_a_rename_with_uuid(cid.get_cp(), src_data))
                {
                    action_col->fputs("rename");
                }
                else
                {
                    action_col->fputs(file_action_ename(src_data->action));
                }
            }
            description_col->fputs(cid.get_cp()->brief_description_get());
            colp->eoln();
        }
    }
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :

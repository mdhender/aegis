//
//      aegis - project change supervisor
//      Copyright (C) 2003-2009, 2012 Peter Miller
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
#include <common/ac/stdio.h>

#include <libaegis/change.h>
#include <libaegis/change/branch.h>
#include <libaegis/emit/edit_number.h>
#include <libaegis/file/event.h>
#include <libaegis/file/event/list.h>
#include <libaegis/http.h>
#include <libaegis/project/file/roll_forward.h>


static file_event *
file_event_from_revision(string_ty *revision,
    const file_event_list::pointer &felp)
{
    if (!felp)
        return 0;
    for (size_t j = 0; j < felp->size(); ++j)
    {
        file_event *fep = felp->get(j);
        assert(fep);
        if (!fep)
            continue;
        fstate_src_ty *src = fep->get_src();
        assert(src);
        if (!src)
            continue;
        assert(src->edit);
        if (!src->edit)
            continue;
        assert(src->edit->revision);
        if (!src->edit->revision)
            continue;
        if (str_equal(revision, src->edit->revision))
            return fep;
    }
    return 0;
}


static void
emit_edit_number_inner(change::pointer cp, fstate_src_ty *src_data,
    const file_event_list::pointer &felp = file_event_list::pointer())
{
    if (src_data->edit_origin && src_data->edit)
    {
        //
        // We have both the original version copied, and the
        // current head revision.  Print them both, with a
        // notation implying "from the old one to the new one"
        // if they differ.  Only print one if thay are the same.
        //
        assert(src_data->edit->revision);
        assert(src_data->edit_origin->revision);
        if
        (
            !str_equal
            (
                src_data->edit->revision,
                src_data->edit_origin->revision
            )
        )
        {
            file_event *fep =
                file_event_from_revision
                (
                    src_data->edit_origin->revision,
                    felp
                );
            if (fep)
            {
                emit_file_href(fep->get_change(), fep->get_src()->file_name, 0);
                printf("%s</a>\n", src_data->edit_origin->revision->str_text);
                nstring mod =
                    nstring::format
                    (
                        "diff+unified+rhs=%s",
                        cp->version_get().c_str()
                    );
                emit_file_href
                (
                    fep->get_change(),
                    fep->get_src()->file_name,
                    mod.c_str()
                );
                printf("&rarr;</a>\n");
            }
            else
            {
                printf
                (
                    "%s\n&rarr;\n",
                    src_data->edit_origin->revision->str_text
                );
            }
        }
        emit_file_href(cp, src_data->file_name, 0);
        printf("%s", src_data->edit->revision->str_text);
        printf("</a>\n");
        return;
    }

    if (src_data->edit_origin)
    {
        //
        // The "original version" copied.
        //
        assert(src_data->edit_origin->revision);
        file_event *fep =
            file_event_from_revision(src_data->edit_origin->revision, felp);
        if (fep)
        {
            emit_file_href(fep->get_change(), fep->get_src()->file_name, 0);
            printf("%s</a>\n", fep->get_src()->edit->revision->str_text);
        }
        else
            printf("%s\n", src_data->edit_origin->revision->str_text);
    }
    if (src_data->edit)
    {
        //
        // For active branches, the current
        // head revision.  For completed changes
        // and branches, the revision at aeipass.
        //
        assert(src_data->edit->revision);
        file_event *fep =
            file_event_from_revision(src_data->edit->revision, felp);
        if (fep)
        {
            emit_file_href(fep->get_change(), fep->get_src()->file_name, 0);
            printf("%s</a>\n", fep->get_src()->edit->revision->str_text);
        }
        else
            printf("%4s\n", src_data->edit->revision->str_text);
    }
    if (!cp->bogus && src_data->edit_origin_new)
    {
        printf("<br>{cross ");

        assert(src_data->edit_origin_new->revision);
        file_event *fep =
            file_event_from_revision(src_data->edit_origin_new->revision, felp);
        if (fep)
        {
            emit_file_href(fep->get_change(), fep->get_src()->file_name, 0);
            printf("%s</a>", fep->get_src()->edit->revision->str_text);
        }
        else
            printf("%4s", src_data->edit_origin_new->revision->str_text);
        printf("}\n");
    }
}


void
emit_edit_number(change::pointer cp, fstate_src_ty *src,
    project_file_roll_forward * hp)
{
    switch (src->action)
    {
    case file_action_remove:
    case file_action_insulate:
    case file_action_transparent:
        emit_edit_number_inner(cp, src);
        break;

    case file_action_create:
    case file_action_modify:
        if (hp && hp->is_set())
            emit_edit_number_inner(cp, src, hp->get(src));
        else
            emit_edit_number_inner(cp, src);
        break;
    }
}


// vim: set ts=8 sw=4 et :

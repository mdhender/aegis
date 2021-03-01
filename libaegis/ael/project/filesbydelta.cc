//
// aegis - project change supervisor
// Copyright (C) 2008, 2009, 2011, 2012 Peter Miller
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

#include <common/ac/assert.h>

#include <common/trace.h>

#include <libaegis/ael/attribu_list.h>
#include <libaegis/ael/build_header.h>
#include <libaegis/ael/column_width.h>
#include <libaegis/ael/project/filesbydelta.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/identifier.h>
#include <libaegis/col.h>
#include <libaegis/file/event.h>
#include <libaegis/option.h>
#include <libaegis/project.h>
#include <libaegis/project/file/roll_forward.h>


void
list_project_files_by_delta(change_identifier &cid, string_list_ty *)
{
    trace(("list_project_files()\n{\n"));

    //
    // create the columns
    //
    col::pointer colp = col::open((string_ty *)0);
    nstring line1 =
        nstring::format
        (
            "Project \"%s\", Change %ld",
            cid.get_pp()->name_get()->str_text,
            cid.get_change_number()
        );
    if (cid.get_cp()->is_completed())
    {
        line1 +=
            nstring::format
            (
                ", Delta %ld",
                cid.get_cp()->cstate_get()->delta_number
            );
    }
    nstring line2 = "Historical List of Project's Files";
    colp->title(line1, line2);

    //
    // Perform historical reconstruction.
    //
    project_file_roll_forward *h = cid.get_historian(option_verbose_get());
    nstring_list filenames;
    h->keys(filenames);
    filenames.sort();

    int left = 0;
    output::pointer usage_col;
    output::pointer action_col;
    output::pointer version_col;
    symtab_ty *attr_col_stp = 0;
    if (!option_terse_get())
    {
        usage_col = colp->create(left, left + USAGE_WIDTH, "Type\n-------");
        left += USAGE_WIDTH + 1;

        action_col =
            colp->create(left, left + ACTION_WIDTH, "Action\n--------");
        left += ACTION_WIDTH + 1;

        attr_col_stp = new symtab_ty(5);
        for (size_t j = 0; j < filenames.size(); ++j)
        {
            nstring filename = filenames[j];
            file_event *fep = h->get_last(filename);
            assert(fep);
            fstate_src_ty *src_data = fep->get_src();
            assert(src_data);
            if (!src_data->attribute)
                continue;
            if (src_data->deleted_by && !option_verbose_get())
                continue;
            for (size_t k = 0; k < src_data->attribute->length; ++k)
            {
                attributes_ty *ap = src_data->attribute->list[k];
                if (ael_attribute_listable(ap))
                {
                    string_ty *lc_name = str_downcase(ap->name);
                    void *p = attr_col_stp->query(lc_name);
                    if (!p)
                    {
                        string_ty *s = ael_build_header(ap->name);
                        output::pointer op =
                            colp->create(left, left + ATTR_WIDTH, s->str_text);
                        str_free(s);
                        attr_col_stp->assign(lc_name, new output::pointer(op));
                        left += ATTR_WIDTH + 1;
                    }
                    str_free(lc_name);
                }
            }
        }

        version_col =
            colp->create(left, left + VERSION_WIDTH, "Version\n---------");
        left += VERSION_WIDTH + 1;
    }
    output::pointer file_name_col =
        colp->create(left, 0, "File Name\n-----------");

    //
    // list the project's files
    //
    for (size_t j = 0; j < filenames.size(); ++j)
    {
        trace(("j = %ld\n", long(j)));
        nstring filename = filenames[j];
        file_event *fep = h->get_last(filename);
        fstate_src_ty *src_data = fep->get_src();
        assert(src_data);
        trace(("src_data = %p\n", src_data));
        if (src_data->deleted_by && !option_verbose_get())
            continue;
        if (option_terse_get())
        {
            if (src_data->deleted_by)
                continue;
        }
        if (usage_col)
            usage_col->fputs(file_usage_ename(src_data->usage));
        if (action_col)
            action_col->fputs(file_action_ename(src_data->action));
        if (version_col)
            version_col->fputs(fep->get_change()->version_get());
        assert(src_data->file_name);
        file_name_col->fputs(src_data->file_name);
        if (src_data->deleted_by)
        {
            file_name_col->end_of_line();
            file_name_col->fprintf
            (
                "Deleted by change %ld.",
                magic_zero_decode(src_data->deleted_by)
            );
        }
        if (src_data->move)
        {
            switch (src_data->action)
            {
            case file_action_create:
                file_name_col->end_of_line();
                file_name_col->fputs("Moved from ");
                file_name_col->fputs(src_data->move->str_text);
                break;

            case file_action_remove:
                file_name_col->end_of_line();
                file_name_col->fputs("Moved to ");
                file_name_col->fputs(src_data->move->str_text);
                break;

            case file_action_modify:
            case file_action_insulate:
            case file_action_transparent:
                assert(0);
                break;
            }
        }
        if (attr_col_stp && src_data->attribute)
        {
            for (size_t k = 0; k < src_data->attribute->length; ++k)
            {
                attributes_ty *ap = src_data->attribute->list[k];
                assert(ap);
                assert(ap->name);
                assert(ap->value);
                if (ap->name && ap->value)
                {
                    string_ty *lc_name = str_downcase(ap->name);
                    void *vp = attr_col_stp->query(lc_name);
                    if (vp)
                    {
                        output::pointer op = *(output::pointer *)vp;
                        assert(op);
                        if (op)
                            op->fputs(ap->value);
                    }
                    str_free(lc_name);
                }
            }
        }
        colp->eoln();
    }

    //
    // clean up and go home
    //
    if (attr_col_stp)
        delete attr_col_stp;
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :

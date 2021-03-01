//
//      aegis - project change supervisor
//      Copyright (C) 1999, 2001-2008 Peter Miller
//      Copyright (C) 2006 Walter Franzini
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

#include <common/error.h> // for assert
#include <common/mem.h>
#include <common/str_list.h>
#include <common/symtab.h>
#include <common/trace.h>
#include <libaegis/ael/attribu_list.h>
#include <libaegis/ael/build_header.h>
#include <libaegis/ael/change/files.h>
#include <libaegis/ael/column_width.h>
#include <libaegis/ael/formeditnum.h>
#include <libaegis/change.h>
#include <libaegis/change/file.h>
#include <libaegis/change/identifier.h>
#include <libaegis/col.h>
#include <libaegis/option.h>
#include <libaegis/output.h>
#include <libaegis/project.h>
#include <libaegis/project/file.h>
#include <libaegis/user.h>


static void
output_reaper(void *p)
{
    output::pointer *opp = (output::pointer *)p;
    delete opp;
}


void
list_change_files(change_identifier &cid, string_list_ty *)
{
    //
    // create the columns
    //
    col::pointer colp = col::open((string_ty *)0);
    string_ty *line1 =
        (
            cid.get_cp()->number == TRUNK_CHANGE_NUMBER
        ?
            str_format
            (
                "Project \"%s\", Trunk",
                cid.get_pp()->name_get()->str_text
            )
        :
            str_format
            (
                "Project \"%s\"  Change %ld",
                cid.get_pp()->name_get()->str_text,
                cid.get_cp()->number
            )
        );
    colp->title(line1->str_text, "List of Change's Files");
    str_free(line1);

    int left = 0;
    output::pointer usage_col;
    output::pointer action_col;
    output::pointer edit_col;
    symtab_ty *attr_col_stp = 0;
    if (!option_terse_get())
    {
        usage_col = colp->create(left, left + USAGE_WIDTH, "Type\n-------");
        left += USAGE_WIDTH + 1;

        action_col =
            colp->create(left, left + ACTION_WIDTH, "Action\n--------");
        left += ACTION_WIDTH + 1;

        edit_col = colp->create(left, left + EDIT_WIDTH, "Edit\n-------");
        left += EDIT_WIDTH + 1;

        if (option_verbose_get())
        {
            //
            // Only print the file attributes in the verbose listing.
            // This is because the number of columns can vary depending
            // on which changes have which attributes.  This is not good
            // for scripting.
            //
            attr_col_stp = new symtab_ty(5);
            attr_col_stp->set_reap(output_reaper);
            for (int j = 0;; ++j)
            {
                fstate_src_ty *src_data =
                    change_file_nth(cid.get_cp(), j, view_path_first);
                if (!src_data)
                    break;
                assert(src_data->file_name);
                if (src_data->attribute)
                {
                    for (size_t k = 0; k < src_data->attribute->length; ++k)
                    {
                        attributes_ty *ap = src_data->attribute->list[k];
                        if (ael_attribute_listable(ap))
                        {
                            string_ty *lc_name = str_downcase(ap->name);
                            void *p = attr_col_stp->query(lc_name);
                            if (!p)
                            {
                                string_ty       *s;
                                output::pointer op;

                                s = ael_build_header(ap->name);
                                op =
                                    colp->create
                                    (
                                        left,
                                        left + ATTR_WIDTH,
                                        s->str_text
                                    );
                                str_free(s);
                                attr_col_stp->assign
                                (
                                    lc_name,
                                    new output::pointer(op)
                                );
                                left += ATTR_WIDTH + 1;
                            }
                            str_free(lc_name);
                        }
                    }
                }
            }
        }
    }
    output::pointer file_name_col =
        colp->create(left, 0, "File Name\n-----------");

    //
    // list the change's files
    //
    for (int j = 0;; ++j)
    {
        fstate_src_ty *src_data =
            change_file_nth(cid.get_cp(), j, view_path_first);
        if (!src_data)
            break;
        assert(src_data->file_name);
        fstate_src_ty *psrc_data = 0;
        if (option_terse_get())
        {
            switch (src_data->action)
            {
            case file_action_remove:
                continue;

            case file_action_create:
            case file_action_modify:
            case file_action_insulate:
            case file_action_transparent:
                break;
            }
        }
        else
        {
            usage_col->fputs(file_usage_ename(src_data->usage));
            action_col->fputs(file_action_ename(src_data->action));
            list_format_edit_number(edit_col, src_data);

            //
            // We use view_path_none because we want the transparent
            // files which exist simply to host the locked_by field.
            // But if the file has been removed, toss it.
            //
            psrc_data =
                project_file_find(cid.get_pp(), src_data, view_path_none);
            if (psrc_data && psrc_data->action == file_action_remove)
                psrc_data = 0;

            if
            (
                cid.get_cp()->is_being_developed()
            &&
                // watch out for ael cf -bl
                (cid.get_cp() != cid.get_pp()->change_get())
            &&
                !change_file_up_to_date(cid.get_pp(), src_data)
            )
            {
                //
                // The current head revision of the branch may not equal
                // the version "originally" copied.
                //
                if (psrc_data && psrc_data->edit)
                {
                    assert(psrc_data->edit->revision);
                    edit_col->fprintf
                    (
                        " (%s)",
                        psrc_data->edit->revision->str_text
                    );
                }
            }
            if (src_data->edit_origin_new)
            {
                //
                // The "cross branch merge" version.
                //
                assert(src_data->edit_origin_new->revision);
                edit_col->end_of_line();
                edit_col->fprintf
                (
                    "{cross %4s}",
                    src_data->edit_origin_new->revision->str_text
                );
            }
        }
        file_name_col->fputs(src_data->file_name);
        if
        (
            cid.get_cp()->is_being_developed()
        &&
            psrc_data
        &&
            psrc_data->locked_by
        &&
            psrc_data->locked_by != cid.get_cp()->number
        )
        {
            file_name_col->end_of_line();
            file_name_col->fprintf
            (
                "Locked by change %ld.",
                magic_zero_decode(psrc_data->locked_by)
            );
        }
        if (src_data->about_to_be_created_by)
        {
            file_name_col->end_of_line();
            file_name_col->fprintf
            (
                "About to be created by change %ld.",
                magic_zero_decode(src_data->about_to_be_created_by)
            );
        }
        if (src_data->deleted_by)
        {
            file_name_col->end_of_line();
            file_name_col->fprintf
            (
                "Deleted by change %ld.",
                magic_zero_decode(src_data->deleted_by)
            );
        }
        if (src_data->locked_by)
        {
            file_name_col->end_of_line();
            file_name_col->fprintf
            (
                "Locked by change %ld.",
                magic_zero_decode(src_data->locked_by)
            );
        }
        if (src_data->about_to_be_copied_by)
        {
            file_name_col->end_of_line();
            file_name_col->fprintf
            (
                "About to be copied by change %ld.",
                magic_zero_decode(src_data->about_to_be_copied_by)
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

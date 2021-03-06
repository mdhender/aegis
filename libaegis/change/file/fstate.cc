//
// aegis - project change supervisor
// Copyright (C) 1999, 2001-2009, 2011, 2012 Peter Miller
// Copyright (C) 2007, 2008 Walter Franzini
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

#include <common/symtab.h>
#include <common/symtab_iter.h>
#include <common/symtab/template.h>
#include <common/trace.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>
#include <libaegis/project.h>
#include <libaegis/sub.h>


void
change::fimprove(fstate_ty *fsd)
{
    trace(("fimprove(fsd = %p)\n{\n", fsd));

    if (!fsd->src)
    {
        fsd->src = (fstate_src_list_ty *)fstate_src_list_type.alloc();
    }

    //
    // Migrate file state information.
    //
    symtab<fstate_src_ty> create_half;
    symtab<fstate_src_ty> remove_half;
    for (size_t j = 0; j < fsd->src->length; ++j)
    {
        fstate_src_ty *src = fsd->src->list[j];

        //
        // Fix an ugly inconsistency in the file action.
        //
        if (src->deleted_by)
            src->action = file_action_remove;

        //
        // Historical 4.9 -> 4.10 transition.
        //
        // This is here to cope with cases where the users upgrade with
        // changes between 'being reviewed' and 'being integrated' states.
        //
        // This code must agree with the corresponding code in
        // libaegis/project.c
        //
        switch (src->action)
        {
        case file_action_transparent:
            break;

        case file_action_remove:
            //
            // There was once a bug in aeclone which caused the
            // edit_origin field to be omitted for removed files.  This,
            // in turn, led to project file entries with no edit_origin
            // field, which hauses assert failues and segfaults all over
            // the place.
            //
            if (!src->edit_origin && !src->edit_number_origin)
            {
                //
                // The problem is , what the heck to we replace it with?
                // Probably not important, since it will never be passed
                // to the history_get_command.
                //
                src->edit_origin =
                    (history_version_ty *)history_version_type.alloc();
                src->edit_origin->errpos = str_copy(src->errpos);
                src->edit_origin->revision = str_from_c("0.0");
                src->edit_origin->encoding = history_version_encoding_none;
            }
            break;

        case file_action_create:
        case file_action_modify:
        case file_action_insulate:
#ifndef DEBUG
        default:
#endif
            if (src->about_to_be_created_by || src->about_to_be_copied_by)
                src->action = file_action_transparent;
            break;
        }

        //
        // Historical 2.3 -> 3.0 transition.
        //
        // This covers a transitional glitch in the edit number
        // semantics.  Very few installed sites will ever
        // need this.
        //
        if (src->edit_number && !src->edit_number_origin)
            src->edit_number_origin = str_copy(src->edit_number);

        //
        // Historical 3.24 to 3.25 transition.
        //
        // This was when history file contents encoding was added,
        // so that we could cope with binary files transparently,
        // even for ascii-only history tools.
        //
        if (src->edit_number)
        {
            src->edit = (history_version_ty *)history_version_type.alloc();
            src->edit->errpos = str_copy(src->errpos);
            src->edit->revision = src->edit_number;
            src->edit->encoding = history_version_encoding_none;
            src->edit_number = 0;
        }
        if (src->edit_number_origin)
        {
            src->edit_origin =
                (history_version_ty *)history_version_type.alloc();
            src->edit_origin->errpos = str_copy(src->errpos);
            src->edit_origin->revision = src->edit_number_origin;
            src->edit_origin->encoding = history_version_encoding_none;
            src->edit_number_origin = 0;
        }
        if (src->edit_number_origin_new)
        {
            src->edit_origin_new =
                (history_version_ty *)history_version_type.alloc();
            src->edit_origin_new->errpos = str_copy(src->errpos);
            src->edit_origin_new->revision = src->edit_number_origin_new;
            src->edit_origin_new->encoding = history_version_encoding_none;
            src->edit_number_origin_new = 0;
        }

        //
        // Make sure things are where they are meant to be.
        //
        if (src->edit && !src->edit->revision)
        {
            sub_context_ty  *scp;

            assert(src->edit->errpos);
            scp = sub_context_new();
            sub_var_set_string(scp, "File_Name", src->edit->errpos);
            sub_var_set_charstar(scp, "FieLD_Name", "edit.revision");
            change_fatal
            (
                this,
                scp,
                i18n("$filename: contains no \"$field_name\" field")
            );
            // NOTREACHED
            sub_context_delete(scp);
        }
        if (src->edit_origin && !src->edit_origin->revision)
        {
            sub_context_ty  *scp;

            assert(src->edit_origin->errpos);
            scp = sub_context_new();
            sub_var_set_string(scp, "File_Name", src->edit_origin->errpos);
            sub_var_set_charstar(scp, "FieLD_Name", "edit_origin.revision");
            change_fatal
            (
                this,
                scp,
                i18n("$filename: contains no \"$field_name\" field")
            );
            // NOTREACHED
            sub_context_delete(scp);
        }
        if (src->edit_origin_new && !src->edit_origin_new->revision)
        {
            sub_context_ty  *scp;

            assert(src->edit_origin_new->errpos);
            scp = sub_context_new();
            sub_var_set_string(scp, "File_Name", src->edit_origin_new->errpos);
            sub_var_set_charstar(scp, "FieLD_Name", "edit_origin_new.revision");
            change_fatal
            (
                this,
                scp,
                i18n("$filename: contains no \"$field_name\" field")
            );
            // NOTREACHED
            sub_context_delete(scp);
        }

        //
        // Set the edit.uuid field if appropriate.  This helps
        // populating the edit_origin.uuid field even if the previous
        // revision is missing the uuid.
        //
        if
        (
            !was_a_branch()
        &&
            !uuid_get().empty()
        &&
            src->edit
        &&
            !src->edit->uuid
        )
            src->edit->uuid = uuid_get().get_ref_copy();

        //
        // Adjust the move field in order to cope with a bug in the
        // aeipass code.  We need another pass to fix the removed
        // files.
        //
        switch (src->action)
        {
        case file_action_remove:
            if (src->move && was_a_branch())
            {
                trace(("remember %s;\n", src->file_name->str_text));
                remove_half.assign (nstring(src->file_name), src);
            }
            break;

        case file_action_create:
            if (src->move && was_a_branch())
                create_half.assign(nstring(src->file_name), src);
            break;

        case file_action_insulate:
        case file_action_modify:
        case file_action_transparent:
#ifndef DEBUG
        default:
#endif
            if (src->move)
            {
                trace(("reset move;\n"));
                str_free(src->move);
                src->move = 0;
            }
            break;
        }
    }

    //
    // Adjust the move field, we now fix the deleted files.
    //
    if (remove_half.size() > 0)
    {
        assert(was_a_branch());
        nstring_list candidate;

        remove_half.keys(candidate);
        trace(("candidate.size() = %ld;\n", (long)candidate.size()));

        for (size_t i = 0; i < remove_half.size(); ++i)
        {
            trace_nstring(candidate[i]);
            fstate_src_ty *src2 = remove_half.query(candidate[i]);
            //
            // Skip over if src2 is NULL, it means the file has already
            // been removed from the list.
            //
            if (!src2)
                continue;
            assert(src2->move);

            trace_string(src2->move->str_text);

            fstate_src_ty *src3 = remove_half.query(src2->move);
            trace_pointer(src3);

            //
            // Skip over is src3 is NULL, it means the file has already
            // been removed from the list.
            //
            if (!src3)
                continue;
            assert(src3->move);

            //
            // Skip over if src3->move is the create half of a rename.
            //
            if (create_half.query(src3->move))
                continue;

            //
            // If the deletion comes from the same branch we need to go
            // down one step to retrieve the right value.
            // Otherwise we compare the integration_timestamp of the changes.
            //
            project *ppp = pp->bind_branch(this);
            assert(ppp);
            trace(("ppp->version = \"%s\";\n",
                project_version_get(ppp).c_str()));
            if (src2->deleted_by == src3->deleted_by)
            {
                change::pointer cp2 = change_alloc(ppp, src2->deleted_by);
                change_bind_existing(cp2);

                cp2->fstate_get();
                assert(cp2->fstate_stp);

                fstate_src_ty *src2_tmp =
                    (fstate_src_ty*)cp2->fstate_stp->query(src2->file_name);
                fstate_src_ty *src3_tmp =
                    (fstate_src_ty*)cp2->fstate_stp->query(src3->file_name);

                assert
                (
                    (src3_tmp->move && !src2_tmp->move)
                ||
                    (src2_tmp->move && !src3_tmp->move)
                );

                if (!src2_tmp->move)
                {
                    str_free(src2->move);
                    src2->move = 0;
                    remove_half.remove(nstring(src2->file_name));
                }
                else
                {
                    assert(str_equal(src2->move, src2_tmp->move));
                }

                if (!src3_tmp->move)
                {
                    str_free(src3->move);
                    src3->move = 0;
                    remove_half.remove(nstring(src3->file_name));
                }
                else
                {
                    assert(str_equal(src3->move, src3_tmp->move));
                }
            }
            else
            {
                //
                // The files are modified by two *different* changes.
                // Since the only possibile scenario is to have a
                // rename followed by a delete (and not the other
                // way), the most recent change is a deletion and
                // it cannot have the move field set.
                //
                time_t completion_timestamp2 =
                    ppp->change_completion_timestamp(src2->deleted_by);
                time_t completion_timestamp3 =
                    ppp->change_completion_timestamp(src3->deleted_by);
                assert(completion_timestamp2 != completion_timestamp3);
                if (completion_timestamp2 < completion_timestamp3)
                {
                    trace(("reset move;\n"));
                    str_free(src3->move);
                    src3->move = 0;
                    remove_half.remove(nstring(src3->file_name));
                }
                else
                {
                    trace(("reset move;\n"));
                    str_free(src2->move);
                    src2->move = 0;
                    remove_half.remove(candidate[i]);
                }
            }
            // project_free(ppp);
        }
    }

    trace(("return;\n"));
    trace(("}\n"));
}


fstate_ty *
change::fstate_get()
{
    trace(("change::fstate_get()\n{\n"));

    //
    // make sure the change state has been read in,
    // in case its src field needed to be converted.
    // (also to ensure lock_sync has been called for both)
    //
    cstate_get();

    if (!fstate_data)
    {
        string_ty *fn = change_fstate_filename_get(this);
        change_become(this);
        fstate_data = fstate_read_file(fn);
        change_become_undo(this);
        fimprove(fstate_data);
    }
    if (!fstate_data->src)
    {
        fstate_data->src = (fstate_src_list_ty *)fstate_src_list_type.alloc();
    }

    //
    // Create a couple of O(1) indexes.
    // This speeds up just about everything.
    //
    if (!fstate_stp)
    {
        fstate_stp = new symtab_ty(fstate_data->src->length);
        fstate_uuid_stp = new symtab_ty(fstate_data->src->length);
        for (size_t j = 0; j < fstate_data->src->length; ++j)
        {
            fstate_src_ty *p = fstate_data->src->list[j];
            fstate_stp->assign(p->file_name, p);
            if (p->uuid && (!p->move || p->action != file_action_remove))
                fstate_uuid_stp->assign(p->uuid, p);
        }
    }
    trace(("return %p;\n", fstate_data));
    trace(("}\n"));
    return fstate_data;
}


// vim: set ts=8 sw=4 et :

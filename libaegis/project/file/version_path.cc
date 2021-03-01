//
//      aegis - project change supervisor
//      Copyright (C) 2002-2008, 2011, 2012 Peter Miller
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

#include <common/error.h>
#include <common/trace.h>
#include <libaegis/change/file.h>
#include <libaegis/change.h>
#include <libaegis/fstate.fmtgen.h>
#include <libaegis/os.h>
#include <libaegis/project/file.h>
#include <libaegis/project.h>
#include <libaegis/undo.h>
#include <libaegis/user.h>


string_ty *
project_file_version_path(project *pp, fstate_src_ty *src, int *unlink_p)
{
    project      *ppp;
    change::pointer cp;
    string_ty       *filename;
    history_version_ty *ed;
    fstate_src_ty   *reconstruct;

    trace(("project_file_version_path(pp = %p, src = %p, "
        "unlink_p = %p)\n{\n", pp, src, unlink_p));
    assert(src);
    assert(src->file_name);
    trace(("fn \"%s\"\n", src->file_name->str_text));
    assert(src->edit || src->edit_origin);
    ed = src->edit ? src->edit : src->edit_origin;
    assert(ed->revision);
    trace(("rev \"%s\"\n", ed->revision->str_text));
    if (unlink_p)
        *unlink_p = 0;
    for (ppp = pp; ppp; ppp = (ppp->is_a_trunk() ? 0 : ppp->parent_get()))
    {
        cp = ppp->change_get();
        if (cp->is_completed())
            continue;
        fstate_src_ty *old_src =
            cp->file_find(nstring(src->file_name), view_path_first);
        if (!old_src)
            continue;
        switch (old_src->action)
        {
        case file_action_remove:
        case file_action_transparent:
            continue;

        case file_action_create:
        case file_action_modify:
        case file_action_insulate:
#ifndef DEBUG
        default:
#endif
            // should be file_action_remove
            assert(!old_src->deleted_by);
            if (old_src->deleted_by)
                continue;

            // should be file_action_transparent
            assert(!old_src->about_to_be_created_by);
            if (old_src->about_to_be_created_by)
                continue;

            // should be file_action_transparent
            assert(!old_src->about_to_be_copied_by);
            if (old_src->about_to_be_copied_by)
                continue;
            break;
        }
        assert(old_src->edit);
        assert(old_src->edit->revision);
        if (str_equal(old_src->edit->revision, ed->revision))
        {
            filename = cp->file_path(src->file_name);
            //
            // The following check is needed to make aegis work even
            // when filename does'not exists.
            //
            os_become_orig();
            int file_exists = os_exists(filename);
            os_become_undo();
            assert(file_exists);
            if (!file_exists)
                break;
            trace(("return \"%s\";\n", filename->str_text));
            trace(("}\n"));
            return filename;
        }
    }

    filename = os_edit_filename(0);
    os_become_orig();
    undo_unlink_errok(filename);
    os_become_undo();
    if (unlink_p)
        *unlink_p = 1;

    reconstruct = (fstate_src_ty *)fstate_src_type.alloc();
    reconstruct->file_name = str_copy(src->file_name);
    reconstruct->edit = history_version_copy(ed);
    change_file_copy_basic_attributes(reconstruct, src);

    cp = pp->change_get();
    change_run_history_get_command
    (
        cp,
        reconstruct,
        filename,
        user_ty::create()
    );
    fstate_src_type.free(reconstruct);
    trace(("return \"%s\";\n", filename->str_text));
    trace(("}\n"));
    return filename;
}


// vim: set ts=8 sw=4 et :

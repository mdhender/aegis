//
//      aegis - project change supervisor
//      Copyright (C) 2001-2008, 2011, 2012 Peter Miller
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

#include <common/debug.h>
#include <common/trace.h>
#include <libaegis/change/file.h>
#include <libaegis/change.h>
#include <libaegis/commit.h>
#include <libaegis/fstate.fmtgen.h>
#include <libaegis/lock.h>
#include <libaegis/os.h>
#include <libaegis/project/file.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>

#include <aeimport/reconstruct.h>


static void
process(change::pointer cp, fstate_src_ty *src, user_ty::pointer up)
{
    project         *pp;
    string_ty       *bl;
    string_ty       *path;
    int             mode;

    trace(("reconstruct::process(filename = \"%s\")\n{\n",
        src->file_name->str_text));
    if (src->usage == file_usage_config)
    {
        trace(("}\n"));
        return;
    }
    pp = cp->pp;
    bl = pp->baseline_path_get();
    mode = 0755 & ~project_umask_get(pp);
    project_become(pp);
    os_mkdir_between(bl, src->file_name, mode);
    project_become_undo(pp);

    path = os_path_cat(bl, src->file_name);
    trace(("src->action = %s;\n", file_action_ename(src->action)));
    switch (src->action)
    {
    case file_action_remove:
        break;

    case file_action_insulate:
    case file_action_transparent:
        assert(0);
        break;

    case file_action_create:
    case file_action_modify:
#ifndef DEBUG
    default:
#endif
        //
        // Extract the file
        //
        trace_string(path->str_text);
        trace(("extract %s\n", src->edit->revision->str_text));
        change_run_history_get_command(cp, src, path, up);

        //
        // Fingerprint the file.
        //
        src->file_fp = (fingerprint_ty *)fingerprint_type.alloc();
        user_ty::become scoped(pp->get_user());
        change_fingerprint_same(src->file_fp, path, 0);
    }

    if (!pp->is_a_trunk())
    {
        string_ty       *path_in = 0;
        string_ty       *path_d;
        static string_ty *dev_null;

        //
        // Difference the file.
        //
        if (!dev_null)
            dev_null = str_from_c("/dev/null");
        path_d = str_format("%s,D", path->str_text);
        trace_string(path_d->str_text);
        switch (src->action)
        {
        case file_action_insulate:
        case file_action_transparent:
            assert(0);
            // fall through...

        case file_action_remove:
            path_in = dev_null;
            break;

        case file_action_create:
        case file_action_modify:
#ifndef DEBUG
        default:
#endif
            path_in = path;
            break;
        }
        change_run_diff_command(cp, up, dev_null, path_in, path_d);

        //
        // Fingerprint the difference file.
        //
        src->diff_file_fp = (fingerprint_ty *)fingerprint_type.alloc();
        user_ty::become scoped(pp->get_user());
        change_fingerprint_same(src->diff_file_fp, path_d, 0);
        str_free(path_d);
    }

    str_free(path);
    trace(("}\n"));
}


void
reconstruct(string_ty *project_name)
{
    project         *pp;
    size_t          j;
    change::pointer cp_bogus;
    string_ty       *bl;
    user_ty::pointer up;

    //
    // Take some locks.
    //
    trace(("reconstruct()\n{\n"));
    pp = project_alloc(project_name);
    pp->bind_existing();
    project_error(pp, 0, i18n("reconstruct baseline"));
    pp->pstate_lock_prepare();
    project_baseline_write_lock_prepare(pp);
    lock_take();

    //
    // Process each file.
    //
    cp_bogus = change_alloc(pp, project_next_change_number(pp, 1));
    change_bind_new(cp_bogus);
    change_architecture_from_pconf(cp_bogus);
    cp_bogus->bogus = 1;
    bl = pp->baseline_path_get();
    change_integration_directory_set(cp_bogus, bl);
    up = project_user(pp);
    for (j = 0; ; ++j)
    {
        fstate_src_ty   *src;

        src = pp->file_nth(j, view_path_simple);
        if (!src)
            break;
        process(cp_bogus, src, up);
    }
    change_free(cp_bogus);

    //
    // Write it all back out.
    //
    pp->pstate_write();
    commit();
    lock_release();
    project_verbose(pp, 0, i18n("import complete"));
    project_free(pp);
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :

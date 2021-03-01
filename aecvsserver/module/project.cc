//
//      aegis - project change supervisor
//      Copyright (C) 2004-2008, 2011, 2012 Peter Miller
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

#include <common/symtab.h>
#include <libaegis/input/file.h>
#include <libaegis/os.h>
#include <libaegis/project/file.h>
#include <libaegis/project/history.h>

#include <aecvsserver/file_info.h>
#include <aecvsserver/module/project.h>
#include <aecvsserver/module/project_bogu.h>
#include <aecvsserver/response/clear_sticky.h>
#include <aecvsserver/response/clearstatdir.h>
#include <aecvsserver/response/created.h>
#include <aecvsserver/response/update_exist.h>
#include <aecvsserver/server.h>


module_project::~module_project()
{
    project_free(pp);
    pp = 0;
}


module_project::module_project(project *arg) :
    pp(arg)
{
}


void
module_project::groan(server_ty *sp, const char *request_name)
{
    server_error
    (
        sp,
        "%s: project \"%s\": you may not alter project files directly, you "
            "must use an Aegis change instead, and the corresponding module",
        request_name,
        project_name_get(pp).c_str()
    );
}


void
module_project::modified(server_ty *sp, string_ty *, file_info_ty *, input &)
{
    //
    // It is an error to try to write a project file.
    //
    groan(sp, "Modified");
}


string_ty *
module_project::calculate_canonical_name()
    const
{
    return project_name_get(pp).get_ref_copy();
}


bool
module_project::update(server_ty *sp, string_ty *, string_ty *server_side_0,
    const options &opt)
{
    //
    // Form a list of files by unioning the source files of the project
    // and all the ancestor branches together.
    //
    // For each of these files, send information about their contents.
    //
    for (size_t j = 0; ; ++j)
    {
        fstate_src_ty   *src;
        string_ty       *client_side;
        string_ty       *server_side;
        string_ty       *path;
        int             need_to_unlink;
        int             mode;
        string_ty       *version;
        file_info_ty    *fip;

        src = pp->file_nth(j, view_path_extreme);
        if (!src)
            break;
        switch (src->usage)
        {
        case file_usage_build:
            continue;

        case file_usage_source:
        case file_usage_config:
        case file_usage_test:
        case file_usage_manual_test:
            break;
        }
        switch (src->action)
        {
        case file_action_remove:
            continue;

        case file_action_transparent:
            assert(0);
            continue;

        case file_action_create:
        case file_action_modify:
        case file_action_insulate:
            break;
        }

        //
        // Make sure the client creates the directories for us.
        //
        server_side = os_path_cat(name(), src->file_name);
        if (!is_update_prefix(server_side_0, server_side, opt.d))
        {
            //
            // don't create files which are not under one of the
            // Directories specified by the client.
            //
            str_free(server_side);
            continue;
        }
        client_side = server_directory_calc_client_side(sp, server_side);

        //
        // Make sure the directory exists.
        //
        server_mkdir_above(sp, client_side, server_side);
        server_updating_verbose(sp, client_side);

        //
        // Determine where to get the file from.
        //
        path = project_file_version_path(pp, src, &need_to_unlink);
        os_become_orig();
        input ip = input_file_open(path, need_to_unlink);
        os_become_undo();
        str_free(path);

        //
        // Determine the file mode.
        //
        mode = 0666;
        if (src->executable)
            mode |= 0111;
        mode &= ~project_umask_get(pp);

        //
        // Determine the version string to send to the client.
        // Special cases:
        //      ""   - no user file,
        //      "0"  - new user file,
        //      "-"  - user file to be removed
        //
        version = 0;
        switch (src->action)
        {
        case file_action_remove:
            version = str_from_c("-");
            break;

        case file_action_transparent:
            version = str_from_c("0");
            break;

        case file_action_create:
        case file_action_modify:
        case file_action_insulate:
            if (src->edit && src->edit->revision)
                version = str_copy(src->edit->revision);
            else
                version = str_from_c("0");
            break;
        }
        if (!version)
            version = str_from_c("");

        //
        // Queue the response to be sent.
        //
        fip = server_file_info_find(sp, server_side, 0);
        if (!fip)
        {
            server_response_queue
            (
                sp,
                new response_created
                (
                    client_side,
                    server_side,
                    ip,
                    mode,
                    version
                )
            );
        }
        else if (str_equal(fip->version, version))
        {
            //
            // What they copied the first time is still the current
            // version on the server side.
            //
            if (fip->modified > 0)
            {
                string_ty       *sub;

                //
                // Remind them they need to commit it.
                // Except they can't, bwah ha ha ha.
                //
                sub = os_entryname(client_side);
                server_m(sp, "M %s (oops)\n", sub->str_text);
                str_free(sub);
            }
        }
        else
        {
            server_response_queue
            (
                sp,
                new response_update_existing
                (
                    client_side,
                    server_side,
                    ip,
                    mode,
                    version
                )
            );
        }
        str_free(server_side);
        str_free(client_side);
        str_free(version);

        os_become_orig();
        ip.close();
        os_become_undo();
    }
    return true;
}


bool
module_project::checkin(server_ty *sp, string_ty *, string_ty *)
{
    groan(sp, "ci");
    return false;
}


bool
module_project::add(server_ty *sp, string_ty *, string_ty *, const options &)
{
    groan(sp, "add");
    return false;
}


bool
module_project::remove(server_ty *sp, string_ty *, string_ty *, const options &)
{
    groan(sp, "remove");
    return false;
}


module_ty *
module_project_new(string_ty *project_name)
{
    project *pp = project_alloc(project_name);
    if (!pp->bind_existing_errok())
    {
        project_free(pp);
        return new module_project_bogus(project_name);
    }

    return new module_project(pp);
}


// vim: set ts=8 sw=4 et :

//
// aegis - project change supervisor
// Copyright (C) 1991-2009, 2011, 2012 Peter Miller
// Copyright (C) 2006, 2008-2010 Walter Franzini
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
#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/fcntl.h>
#include <common/ac/unistd.h>

#include <common/nstring/list.h>
#include <common/progname.h>
#include <common/quit.h>
#include <common/sizeof.h>
#include <common/str_list.h>
#include <common/trace.h>
#include <common/uuidentifier.h>
#include <libaegis/ael/project/files.h>
#include <libaegis/arglex/change.h>
#include <libaegis/arglex/project.h>
#include <libaegis/arglex2.h>
#include <libaegis/attribute.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>
#include <libaegis/change/functor/file_find.h>
#include <libaegis/change/identifier.h>
#include <libaegis/col.h>
#include <libaegis/commit.h>
#include <libaegis/dir.h>
#include <libaegis/file.h>
#include <libaegis/help.h>
#include <libaegis/lock.h>
#include <libaegis/log.h>
#include <libaegis/os.h>
#include <libaegis/pconf.fmtgen.h>
#include <libaegis/project.h>
#include <libaegis/project/file.h>
#include <libaegis/project/invento_walk.h>
#include <libaegis/search_path/base_get.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>

#include <aegis/aenf.h>


static void
new_file_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf
    (
        stderr,
        "usage: %s -New_File <filename>... [ <option>... ]\n",
        progname
    );
    fprintf(stderr, "       %s -New_File -List [ <option>... ]\n", progname);
    fprintf(stderr, "       %s -New_File -Help\n", progname);
    quit(1);
}


static void
new_file_help(void)
{
    help("aenf", new_file_usage);
}


static void
new_file_list(void)
{
    trace(("new_file_list()\n{\n"));
    arglex();
    change_identifier cid;
    cid.command_line_parse_rest(new_file_usage);
    list_project_files(cid, 0);
    trace(("}\n"));
}


struct walker_ty
{
    user_ty::pointer up;
    change::pointer cp;
    string_list_ty  *slp;
    string_ty       *dd;
    int             found;
    int             used;
};


static void
walker(void *p, dir_walk_message_ty msg, string_ty *path, const struct stat *)
{
    walker_ty       *aux;
    string_ty       *s;
    string_ty       *check_msg;

    aux = (walker_ty *)p;
    switch (msg)
    {
    case dir_walk_dir_before:
    case dir_walk_dir_after:
        break;

    case dir_walk_special:
    case dir_walk_symlink:
        //
        // Ignore special files and symlinks.
        // If they need them, they can create them at build time.
        //
        break;

    case dir_walk_file:
        //
        // Ignore files we already know about.
        // Ignore files the change already knows about.
        // Ignore files the project already knows about.
        //
        aux->found++;
        s = os_below_dir(aux->dd, path);
        aux->up->become_end();
        if
        (
            !aux->slp->member(s)
        &&
            !aux->cp->file_find(nstring(s), view_path_first)
        &&
            !aux->cp->pp->file_find(s, view_path_extreme)
        )
        {
            check_msg = change_filename_check(aux->cp, s);
            if (!check_msg)
            {
                aux->slp->push_back(s);
                aux->used++;
            }
            else
                str_free(check_msg);
        }
        aux->up->become_begin();
        str_free(s);
        break;
    }
}


static void
new_file_independent(void)
{
    trace(("new_file_independent()\n{\n"));
    change_identifier cid;
    arglex();
    nstring_list wl;
    log_style_ty log_style = log_style_append_default;
    int nerrs = 0;
    int use_template = -1;
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(new_file_usage);
            continue;

        case arglex_token_file:
            if (arglex() != arglex_token_string)
                new_file_usage();
            // fall through...

        case arglex_token_string:
            wl.push_back(nstring(arglex_value.alv_string));
            break;

        case arglex_token_branch:
        case arglex_token_delta:
        case arglex_token_delta_date:
        case arglex_token_grandparent:
        case arglex_token_project:
        case arglex_token_trunk:
            cid.command_line_parse(new_file_usage);
            continue;

        case arglex_token_nolog:
            if (log_style == log_style_none)
                duplicate_option(new_file_usage);
            log_style = log_style_none;
            break;

        case arglex_token_wait:
        case arglex_token_wait_not:
            user_ty::lock_wait_argument(new_file_usage);
            break;

        case arglex_token_template:
        case arglex_token_template_not:
            if (use_template >= 0)
                duplicate_option(new_file_usage);
            use_template = (arglex_token == arglex_token_template);
            break;
        }
        arglex();
    }
    if (nerrs)
    {
        sub_context_ty sc;
        sc.var_set_long("Number", nerrs);
        sc.var_optional("Number");
        fatal_intl(&sc, i18n("no new files"));
        // NOTREACHED
    }
    if (wl.empty())
    {
        error_intl(0, i18n("no file names"));
        new_file_usage();
    }

    //
    // Create each file in the current directory,
    // if it does not exist already.
    // Create any necessary directories along the way.
    //
    change::pointer cp = cid.get_bogus_cp();
    os_become_orig();
    cp->cstate_get()->development_directory = os_curdir();
    os_become_undo();
    for (size_t j = 0; j < wl.size(); ++j)
    {
        change_file_template(cp, wl[j].get_ref(), cid.get_up(), use_template);
    }
    trace(("}\n"));
}


static void
new_file_main(void)
{
    trace(("new_file_main()\n{\n"));
    arglex();
    string_ty *project_name = 0;
    long change_number = 0;
    file_usage_ty file_usage_value = file_usage_source;
    string_list_ty wl;
    log_style_ty log_style = log_style_append_default;
    int nerrs = 0;
    int use_template = -1;
    int auto_config_allowed = 1;
    string_ty *uuid = 0;
    bool as_needed = false;
    bool use_uuid = true;
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(new_file_usage);
            continue;

        case arglex_token_file:
            if (arglex() != arglex_token_string)
                new_file_usage();
            // fall through...

        case arglex_token_string:
            {
                string_ty *s2 = str_from_c(arglex_value.alv_string);
                wl.push_back(s2);
                str_free(s2);
            }
            break;

        case arglex_token_change:
            arglex();
            // fall through...

        case arglex_token_number:
            arglex_parse_change(&project_name, &change_number, new_file_usage);
            continue;

        case arglex_token_project:
            arglex();
            arglex_parse_project(&project_name, new_file_usage);
            continue;

        case arglex_token_nolog:
            if (log_style == log_style_none)
                duplicate_option(new_file_usage);
            log_style = log_style_none;
            break;

        case arglex_token_build:
            if (file_usage_value != file_usage_source)
                duplicate_option(new_file_usage);
            file_usage_value = file_usage_build;
            auto_config_allowed = 0;
            break;

        case arglex_token_configured:
            if (file_usage_value != file_usage_source)
                duplicate_option(new_file_usage);
            file_usage_value = file_usage_config;
            auto_config_allowed = 0;
            break;

        case arglex_token_configured_not:
            if (file_usage_value != file_usage_source)
                duplicate_option(new_file_usage);
            file_usage_value = file_usage_source;
            auto_config_allowed = 0;
            break;

        case arglex_token_wait:
        case arglex_token_wait_not:
            user_ty::lock_wait_argument(new_file_usage);
            break;

        case arglex_token_base_relative:
        case arglex_token_current_relative:
            user_ty::relative_filename_preference_argument(new_file_usage);
            break;

        case arglex_token_template:
        case arglex_token_template_not:
            if (use_template >= 0)
                duplicate_option(new_file_usage);
            use_template = (arglex_token == arglex_token_template);
            break;

        case arglex_token_uuid:
            {
                if (uuid)
                    duplicate_option(new_file_usage);
                if (arglex () != arglex_token_string)
                    option_needs_string(arglex_token_uuid, new_file_usage);
                string_ty *s2 = str_from_c(arglex_value.alv_string);
                if (!universal_unique_identifier_valid(s2))
                    option_needs_string(arglex_token_uuid, new_file_usage);
                uuid = str_downcase(s2);
                str_free(s2);
            }
            break;

        case arglex_token_uuid_not:
            use_uuid = false;
            break;

        case arglex_token_keep:
        case arglex_token_interactive:
        case arglex_token_keep_not:
            user_ty::delete_file_argument(new_file_usage);
            break;

        case arglex_token_as_needed:
            if (as_needed)
                duplicate_option(new_file_usage);
            as_needed = true;
            break;
        }
        arglex();
    }

    //
    // no-uuid conflicts with uuid
    //
    if (uuid && !use_uuid)
    {
        mutually_exclusive_options
        (
            arglex_token_uuid,
            arglex_token_uuid_not,
            new_file_usage
        );
    }

    if (nerrs)
    {
        sub_context_ty sc;
        sc.var_set_long("Number", nerrs);
        sc.var_optional("Number");
        fatal_intl(&sc, i18n("no new files"));
        // NOTREACHED
    }
    if (!wl.nstrings)
    {
        error_intl(0, i18n("no file names"));
        new_file_usage();
    }

    //
    // It is an error if the -uuid switch is used and more than one file
    // is nominated on the command line.
    //
    if (uuid && wl.nstrings > 1)
    {
        error_intl(0, i18n("too many files"));
        new_file_usage();
    }

    //
    // locate project data
    //
    if (!project_name)
    {
        nstring n = user_ty::create()->default_project();
        project_name = str_copy(n.get_ref());
    }
    project *pp = project_alloc(project_name);
    str_free(project_name);
    project_name = 0;
    pp->bind_existing();

    //
    // locate user data
    //
    user_ty::pointer up = user_ty::create();

    //
    // locate change data
    //
    if (!change_number)
        change_number = up->default_change(pp);
    change::pointer cp = change_alloc(pp, change_number);
    change_bind_existing(cp);

    //
    // lock everything, we need this to avoid race conditions while
    // searching the whole project for UUID already assigned to files
    // with the same name.
    //
    pp->trunk_get()->lock_prepare_everything();
    lock_take();

    log_open(change_logfile_get(cp), up, log_style);

    if (cp->file_promote())
    {
        //
        // Write out the file state, and then let go of the locks
        // and take them again.  This ensures the data is consistent
        // for the next stage of processing.
        //
        trace(("Write out what we've done so far.\n"));
        cp->cstate_write();
        commit();
        lock_release();

        trace(("Take the locks again.\n"));
        change_cstate_lock_prepare(cp);
        lock_take();
    }

    //
    // It is an error if the change is not in the being_developed state.
    // It is an error if the change is not assigned to the current user.
    //
    if (!cp->is_being_developed())
        change_fatal(cp, 0, i18n("bad nf state"));
    if (cp->is_a_branch())
        change_fatal(cp, 0, i18n("bad nf branch"));
    if (nstring(cp->developer_name()) != up->name())
        change_fatal(cp, 0, i18n("not developer"));

    //
    // It is an error if the UUID is already is use.
    //
    if (uuid)
    {
        // The "view_path_extreme" means removed files will not be used
        // when checking (otherwise, use "view_path_simple").
        fstate_src_ty *src = cp->file_find_uuid(uuid, view_path_extreme);
        if (src)
        {
            sub_context_ty sc;
            sc.var_set_string("Other", src->file_name);
            sc.var_optional("Other");
            change_fatal(cp, &sc, i18n("bad ca, uuid duplicate"));
            // NOTREACHED
        }
    }

    //
    // Search list for resolving filenames.
    //
    string_list_ty search_path;
    cp->search_path_get(&search_path, true);

    //
    // Find the base for relative filenames.
    //
    nstring base(search_path_base_get(search_path, up));

    //
    // resolve the path of each file
    // 1. the absolute path of the file name is obtained
    // 2. if the file is inside the development directory, ok
    // 3. if the file is inside the baseline, ok
    // 4. if neither, error
    //
    {
        string_list_ty wl2;
        for (size_t j = 0; j < wl.nstrings; ++j)
        {
            string_ty *s1 = wl.string[j];
            string_ty *s2 = 0;
            if (s1->str_text[0] == '/')
                s2 = str_copy(s1);
            else
                s2 = os_path_join(base.get_ref(), s1);
            up->become_begin();
            s1 = os_pathname(s2, 1);
            up->become_end();
            str_free(s2);
            s2 = 0;
            for (size_t k = 0; k < search_path.nstrings; ++k)
            {
                s2 = os_below_dir(search_path.string[k], s1);
                if (s2)
                    break;
            }
            str_free(s1);
            if (!s2)
            {
                sub_context_ty sc;
                sc.var_set_string("File_Name", wl.string[j]);
                change_error(cp, &sc, i18n("$filename unrelated"));
                ++nerrs;
                continue;
            }
            if (wl2.member(s2))
            {
                sub_context_ty sc;
                sc.var_set_string("File_Name", s2);
                change_error(cp, &sc, i18n("too many $filename"));
                ++nerrs;
            }
            else
                wl2.push_back(s2);
            str_free(s2);
        }
        wl = wl2;
    }

    //
    // check that each filename is OK
    //
    // If a directory is named, extract the files from beneath it.
    //
    {
        string_ty *dd = change_development_directory_get(cp, 0);
        string_list_ty wl2;
        for (size_t j = 0; j < wl.nstrings; ++j)
        {
            string_ty *fn = wl.string[j];
            up->become_begin();
            string_ty *ffn = os_path_cat(dd, fn);
            if (os_isa_directory(ffn))
            {
                walker_ty       aux;

                aux.up = up;
                aux.cp = cp;
                aux.slp = &wl;
                aux.dd = dd;
                aux.found = 0;
                aux.used = 0;
                dir_walk(ffn, walker, &aux);

                //
                // It's an error if there is nothing to do for
                // this directory.
                //
                if (!aux.used)
                {
                    sub_context_ty sc;
                    if (fn->str_length)
                        sc.var_set_string("File_Name", fn);
                    else
                        sc.var_set_charstar("File_Name", ".");
                    sc.var_set_long("Number", aux.found);
                    sc.var_optional("Number");
                    change_error
                    (
                        cp,
                        &sc,
                        i18n("directory $filename contains no relevant files")
                    );
                    ++nerrs;
                }
                up->become_end();
                str_free(ffn);
                continue;
            }
            else if (os_isa_symlink(ffn))
            {
                //
                // In the case where we are using a
                // symlink farm, this is probably a link
                // into the baseline.  Just nuke it,
                // and keep going.
                //
                os_unlink(ffn);
            }
            else if (os_isa_special_file(ffn))
            {
                //
                // If the file exists, and isn't a normal file,
                // and it isn't a symbolic link,
                // and isn't a directory, you can't add it as
                // a source file.
                //
                sub_context_ty sc;
                sc.var_set_string("File_Name", fn);
                change_error(cp, &sc, i18n("$filename bad nf type"));
                ++nerrs;
                up->become_end();
                str_free(ffn);
                continue;
            }
            str_free(ffn);
            up->become_end();
            string_ty *e = change_filename_check(cp, fn);
            if (e)
            {
                //
                // no internationalization of the error string
                // required, this is done inside the
                // change_filename_check function.
                //
                sub_context_ty sc;
                sc.var_set_string("MeSsaGe", e);
                change_error(cp, &sc, i18n("$message"));
                ++nerrs;
                str_free(e);
                break;
            }

            //
            // Ignore redundant requests.
            //
            if (as_needed)
            {
                fstate_src_ty *csrc =
                    cp->file_find(nstring(fn), view_path_first);
                if (csrc && csrc->action == file_action_create && !csrc->move)
                    continue;
            }

            //
            // Remember this one.
            //
            wl2.push_back(fn);
        }
        wl = wl2;
    }
    if (nerrs)
    {
        sub_context_ty sc;
        sc.var_set_long("Number", nerrs);
        sc.var_optional("Number");
        change_fatal(cp, &sc, i18n("no new files"));
    }

    //
    // ensure that each file
    // 1. is not already part of the change
    //        - except removed files
    // 2. is not already part of the baseline
    //        - except removed files
    // 3. does not have a directory conflict
    //
    for (size_t j = 0; j < wl.nstrings; ++j)
    {
        string_ty *s1 = wl.string[j];
        string_ty *s2 = change_file_directory_conflict(cp, s1);
        if (s2)
        {
            sub_context_ty sc;
            sc.var_set_string("File_Name", s1);
            sc.var_set_string("File_Name2", s2);
            sc.var_optional("File_Name2");
            change_error
            (
                cp,
                &sc,
                i18n("file $filename directory name conflict")
            );
            ++nerrs;
            continue;
        }
        s2 = project_file_directory_conflict(pp, s1);
        if (s2)
        {
            sub_context_ty sc;
            sc.var_set_string("File_Name", s1);
            sc.var_set_string("File_Name2", s2);
            sc.var_optional("File_Name2");
            project_error
            (
                pp,
                &sc,
                i18n("file $filename directory name conflict")
            );
            ++nerrs;
            continue;
        }
        fstate_src_ty *src_data = cp->file_find(nstring(s1), view_path_first);
        if (src_data)
        {
            switch (src_data->action)
            {
            case file_action_remove:
                break;

            case file_action_create:
            case file_action_modify:
            case file_action_insulate:
            case file_action_transparent:
#ifndef DEBUG
            default:
#endif
                {
                    sub_context_ty sc;
                    sc.var_set_string("File_Name", s1);
                    change_error(cp, &sc, i18n("file $filename dup"));
                    ++nerrs;
                }
                break;
            }
        }
        else
        {
            src_data = pp->file_find(s1, view_path_extreme);
            if (src_data)
            {
                sub_context_ty sc;
                sc.var_set_string("File_Name", s1);
                project_error(pp, &sc, i18n("$filename in baseline"));
                ++nerrs;
            }
        }
    }
    if (nerrs)
    {
        sub_context_ty sc;
        sc.var_set_long("Number", nerrs);
        sc.var_optional("Number");
        change_fatal(cp, &sc, i18n("no new files"));
    }

    //
    // Create each file in the development directory,
    // if it does not already exist.
    // Create any necessary directories along the way.
    //
    for (size_t j = 0; j < wl.nstrings; ++j)
    {
        change_file_template(cp, wl.string[j], up, use_template);
    }

    //
    // Walk the whole project, looking for UUIDs that can potentially be
    // recycled.  User expectations appear to expect such recycling when
    // putting a source file "back", often across branches.
    //
    change_functor_file_find whole_of_project_uuids(wl);
    project_inventory_walk(pp->trunk_get(), whole_of_project_uuids);

    //
    // Add each file to the change set meta data,
    // and write it back out.
    //
    {
        string_list_ty wl2;
        for (size_t j = 0; j < wl.nstrings; ++j)
        {
            string_ty *s1 = wl.string[j];
            trace(("s1 = \"%s\"\n", s1->str_text));

            //
            // If the file is already in the change (we checked for this
            // earlier) then it must be being removed, and we are replacing
            // it, usually so we can change its fstate_src::usage.
            //
            fstate_src_ty *src_data =
                cp->file_find(nstring(s1), view_path_first);
            trace(("src_data = %p\n", src_data));
            if (src_data)
            {
                //
                // Discard information that isn't about this new file,
                // it is about the file we are "new-file"ing on top of.
                //
                if (src_data->action == file_action_remove)
                {
                    if (src_data->edit_origin)
                    {
                        trace(("src_data->edit_origin = %p\n",
                            src_data->edit_origin));
                        history_version_type.free(src_data->edit_origin);
                        src_data->edit_origin = 0;
                    }
                    if (src_data->move)
                    {
                        trace(("src_data->move = \"%s\"\n",
                            src_data->move->str_text));
                        str_free(src_data->move);
                        src_data->move = 0;
                    }
                }

                // Set the action appropriately.
                trace(("src_data->action = %s\n",
                    file_action_ename(src_data->action)));
                if (src_data->action == file_action_remove)
                    src_data->action = file_action_create;
                else
                    src_data->action = file_action_modify;
                trace(("src_data->action = %s\n",
                    file_action_ename(src_data->action)));

                // Set the usage.
                trace(("src_data->usage = %s\n",
                    file_usage_ename(src_data->usage)));
                file_usage_ty usage = file_usage_value;
                if (auto_config_allowed && cp->file_is_config(s1))
                {
                    static string_ty *old;
                    if (!old)
                        old = str_from_c(THE_CONFIG_FILE_OLD);
                    if (str_equal(s1, old))
                    {
                        sub_context_ty sc;
                        sc.var_set_string("File_Name1", s1);
                        sc.var_set_charstar("File_Name2", THE_CONFIG_FILE_NEW);
                        sc.error_intl
                        (
                            i18n("warning: $filename1 deprecated, "
                                "use $filename2 file instead")
                        );
                    }
                    usage = file_usage_value;
                }
                src_data->usage = usage;
                trace(("src_data->usage = %s\n",
                    file_usage_ename(src_data->usage)));

                if (uuid && use_uuid)
                {
                    // We checked the user-specified UUID already,
                    // so we know that at this point, it is indeed unique.
                    src_data->uuid = uuid;
                    uuid = 0;
                }
                else if (!uuid && use_uuid)
                {
                    if (!src_data->uuid)
                    {
                        // assign a shiny new UUID
                        assert(!src_data->uuid);
                        for (int ntries = 0; ntries < 100; ++ntries)
                        {
                            string_ty *maybe = universal_unique_identifier();

                            // check for duplicates
                            fstate_src_ty *src2 =
                                cp->file_find_uuid(maybe, view_path_extreme);
                            if (!src2)
                            {
                                trace(("success after %d tries\n", ntries + 1));
                                src_data->uuid = maybe;
                                break;
                            }
                        }
                    }
                }

                // Skip the rest of the loop body,
                // it is for new files, and this one wasn't.
                continue;
            }

            //
            // The normal case for adding a new file.
            //
            trace(("adding new file \"%s\"\n", s1->str_text));
            wl2.push_back(s1);

            //
            // The project config file doesn't need to have the -config
            // option specifically supplied.
            //
            // You have to do this test BEFORE the change::file_new call.
            //
            file_usage_ty usage = file_usage_value;
            if (auto_config_allowed && cp->file_is_config(s1))
            {
                static string_ty *old;
                if (!old)
                    old = str_from_c(THE_CONFIG_FILE_OLD);
                if (str_equal(s1, old))
                {
                    sub_context_ty sc;
                    sc.var_set_string("File_Name1", s1);
                    sc.var_set_charstar("File_Name2", THE_CONFIG_FILE_NEW);
                    sc.error_intl
                    (
                        i18n("warning: $filename1 deprecated, "
                            "use $filename2 file instead")
                    );
                }
                usage = file_usage_config;
            }

            //
            // Add the new file to the change set meta data.
            //
            src_data = cp->file_new(s1);
            src_data->action = file_action_create;
            src_data->usage = usage;

            if (uuid || use_uuid)
            {
                assert(!src_data->uuid);
                if (uuid)
                {
                    // Ideally, this check is utterly redundant,
                    // because we checked that nfiles==1 above.
                    fstate_src_ty *src2 =
                        cp->file_find_uuid(uuid, view_path_extreme);
                    if (src2)
                    {
                        sub_context_ty sc(__FILE__, __LINE__);
                        sc.var_set_string("Other", src2->file_name);
                        sc.var_optional("Other");
                        change_fatal(cp, &sc, i18n("bad ca, uuid duplicate"));
                        // NOTREACHED
                    }

                    src_data->uuid = uuid;
                    uuid = 0;
                }
                else
                {
                    assert(!src_data->uuid);

                    // Look for a project file, using view_path_simple,
                    // to see if we can recycle a (re)moved file's UUID,
                    // when the file has the same name.  Logically, it
                    // is a new file, but user expectation appears to
                    // differ (e.g., putting a file "back").
                    {
                        // We are looking across all branches for
                        // duplicates, because that better matches user
                        // expectations when putting a file "back".
                        nstring *quuid =
                            whole_of_project_uuids.query(nstring(s1));
                        trace(("quuid = %p\n", quuid));
                        if (quuid)
                        {
                            // we have a candidate
                            trace(("quuid = %s\n", quuid->quote_c().c_str()));

                            // Only recycle the UUID if it is unique on this
                            // branch.  Example of not being unique is the
                            // remove half of a rename, because the create
                            // half of the move is still using the UUID.
                            fstate_src_ty *fs3 =
                                cp->file_find_uuid
                                (
                                    quuid->get_ref(),
                                    view_path_extreme
                                );
                            trace(("fs3 = %p\n", fs3));
                            if (!fs3)
                            {
                                uuid = str_copy(quuid->get_ref());
                                trace(("copying uuid\n"));
                                src_data->uuid = str_copy(uuid);
                            }
                        }
                    }

                    if (!uuid)
                    {
                        for (int ntries = 0; ntries < 100; ++ntries)
                        {
                            string_ty *maybe =
                                universal_unique_identifier();

                            // check for duplicates
                            fstate_src_ty *src2 =
                                cp->file_find_uuid(maybe, view_path_extreme);
                            if (!src2)
                            {
                                trace(("success after %d tries\n", ntries + 1));
                                src_data->uuid = maybe;
                                maybe = 0;
                                trace(("src_data->uuid = \"%s\"\n",
                                    src_data->uuid->str_text));
                                break;
                            }
                        }
                    }
                }

                if (src_data->uuid)
                    assert(universal_unique_identifier_valid(src_data->uuid));
            }
            else
            {
                //
                // If the user asked to not have the UUID assigned to this
                // file, make sure it is skipped also at aeipass time.
                //
                src_data->attribute = (attributes_list_ty *)
                    attributes_list_type.alloc();
                attributes_list_append
                (
                    src_data->attribute,
                    AEIPASS_ASSIGN_FILE_UUID,
                    "false"
                );
            }
        }
        wl = wl2;
    }

    //
    // the number of files changed, or the version did,
    // so stomp on the validation fields.
    //
    change_build_times_clear(cp);

    //
    // update the copyright years
    //
    change_copyright_years_now(cp);

    //
    // If the file manifest of the change is altered (e.g. by aenf, aenfu,
    // aecp, aecpu, etc), or the contents of any file is changed, the
    // UUID is cleared.  This is because it is no longer the same change
    // as was received by aedist or aepatch, and the UUID is invalidated.
    //
    change_uuid_clear(cp);

    // remember we are about to
    bool recent_integration = cp->run_project_file_command_needed();
    if (recent_integration)
        cp->run_project_file_command_done();

    //
    // release the locks
    //
    cp->cstate_write();
    commit();
    lock_release();

    //
    // run the change file command
    // and the project file command if necessary
    //
    cp->run_new_file_command(&wl, up);
    if (recent_integration)
        cp->run_project_file_command(up);

    //
    // verbose success message
    //
    for (size_t j = 0; j < wl.nstrings; ++j)
    {
        sub_context_ty sc;
        sc.var_set_string("File_Name", wl.string[j]);
        change_verbose(cp, &sc, i18n("new file $filename completed"));
    }
    change_free(cp);
    project_free(pp);
    trace(("}\n"));
}


void
new_file(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
        { arglex_token_help, new_file_help, 0 },
        { arglex_token_list, new_file_list, 0 },
        { arglex_token_independent, new_file_independent, 0 },
    };

    trace(("new_file()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), new_file_main);
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :

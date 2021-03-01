//
//	aegis - project change supervisor
//	Copyright (C) 1993-1999, 2001-2004 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to implement move file
//

#include <ac/stdio.h>
#include <ael/project/files.h>

#include <aemv.h>
#include <arglex2.h>
#include <arglex/change.h>
#include <arglex/project.h>
#include <change/branch.h>
#include <change/file.h>
#include <commit.h>
#include <error.h>
#include <file.h>
#include <help.h>
#include <lock.h>
#include <log.h>
#include <move_list.h>
#include <os.h>
#include <progname.h>
#include <project.h>
#include <project/file.h>
#include <quit.h>
#include <sub.h>
#include <trace.h>
#include <undo.h>
#include <user.h>
#include <str_list.h>


static void
move_file_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf
    (
	stderr,
	"usage: %s -MoVe_file [ <option>... ] <old-name> <new-name>...\n",
	progname
    );
    fprintf(stderr, "       %s -MoVe_file -List [ <option>... ]\n", progname);
    fprintf(stderr, "       %s -MoVe_file -Help\n", progname);
    quit(1);
}


static void
move_file_help(void)
{
    help("aemv", move_file_usage);
}


static void
move_file_list(void)
{
    string_ty	    *project_name;
    long	    change_number;

    trace(("move_file_list()\n{\n"));
    arglex();
    project_name = 0;
    change_number = 0;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(move_file_usage);
	    continue;

	case arglex_token_change:
	    arglex();
	    // fall through...

	case arglex_token_number:
	    arglex_parse_change(&project_name, &change_number, move_file_usage);
	    continue;

	case arglex_token_project:
	    arglex();
	    // fall through...

	case arglex_token_string:
	    arglex_parse_project(&project_name, move_file_usage);
	    continue;
	}
	arglex();
    }
    list_project_files(project_name, change_number, 0);
    if (project_name)
	str_free(project_name);
    trace(("}\n"));
}


static void
move_file_innards(user_ty *up, change_ty *cp, string_ty *old_name,
    string_ty *new_name, string_list_ty *wl_nf, string_list_ty *wl_nt,
    string_list_ty *wl_rm)
{
    project_ty	    *pp;
    fstate_src_ty   *p_src_data;
    fstate_src_ty   *pn_src_data;
    fstate_src_ty   *c_src_data;
    string_ty	    *from;
    string_ty	    *to;
    string_ty	    *dd;
    int		    mode;

    pp = cp->pp;

    //
    // the old file may not be in the change already
    //
    if (change_file_find(cp, old_name, view_path_first))
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_string(scp, "File_Name", old_name);
	change_fatal(cp, scp, i18n("file $filename dup"));
	// NOTREACHED
	sub_context_delete(scp);
    }

    //
    // the old file must be in the baseline
    //
    p_src_data = project_file_find(pp, old_name, view_path_extreme);
    if (!p_src_data)
    {
	p_src_data = project_file_find_fuzzy(pp, old_name, view_path_extreme);
	if (p_src_data)
	{
	    sub_context_ty  *scp;

	    scp = sub_context_new();
	    sub_var_set_string(scp, "File_Name", old_name);
	    sub_var_set_string(scp, "Guess", p_src_data->file_name);
	    project_fatal(pp, scp, i18n("no $filename, closest is $guess"));
	    // NOTREACHED
	    sub_context_delete(scp);
	}
	else
	{
	    sub_context_ty  *scp;

	    scp = sub_context_new();
	    sub_var_set_string(scp, "File_Name", old_name);
	    project_fatal(pp, scp, i18n("no $filename"));
	    // NOTREACHED
	    sub_context_delete(scp);
	}
    }

    //
    // You may not move a file on top of itself (use aecp!).
    //
    if (str_equal(old_name, new_name))
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_string(scp, "File_Name", old_name);
	change_fatal(cp, scp, i18n("nil move $filename"));
	// NOTREACHED
	sub_context_delete(scp);
    }

    //
    // the new file must not already be part of the change
    //
    if (change_file_find(cp, new_name, view_path_first))
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_string(scp, "File_Name", new_name);
	change_fatal(cp, scp, i18n("file $filename dup"));
	// NOTREACHED
	sub_context_delete(scp);
    }

    //
    // the new file must not be part of the baseline
    //
    pn_src_data = project_file_find(pp, new_name, view_path_extreme);
    if (pn_src_data)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_string(scp, "File_Name", new_name);
	project_fatal(pp, scp, i18n("$filename in baseline"));
	// NOTREACHED
	sub_context_delete(scp);
    }

    //
    // Add the files to the change
    //
    assert(p_src_data);
    c_src_data = change_file_new(cp, old_name);
    c_src_data->action = file_action_remove;
    change_file_copy_basic_attributes(c_src_data, p_src_data);
    c_src_data->move = str_copy(new_name);
    assert(p_src_data->edit);
    assert(p_src_data->edit->revision);
    if (p_src_data->edit)
	c_src_data->edit_origin = history_version_copy(p_src_data->edit);

    c_src_data = change_file_new(cp, new_name);
    c_src_data->action = file_action_create;
    change_file_copy_basic_attributes(c_src_data, p_src_data);
    c_src_data->move = str_copy(old_name);
    if (p_src_data->edit && c_src_data->uuid)
	c_src_data->edit_origin = history_version_copy(p_src_data->edit);

    //
    // Add the file to the appropriate notification lists.
    //
    string_list_append(wl_rm, old_name);
    switch (c_src_data->usage)
    {
    case file_usage_test:
    case file_usage_manual_test:
	string_list_append(wl_nt, new_name);
	break;

    case file_usage_source:
    case file_usage_config:
    case file_usage_build:
	string_list_append(wl_nf, new_name);
	break;
    }

    //
    // If the file is built, we are done.
    //
    switch (c_src_data->usage)
    {
    case file_usage_build:
	return;

    case file_usage_source:
    case file_usage_config:
    case file_usage_test:
    case file_usage_manual_test:
	break;
    }

    //
    // Copy the file into the development directory.
    // Create any necessary directories along the way.
    //
    from = project_file_path(pp, old_name);
    assert(from);
    to = change_file_path(cp, new_name);
    assert(to);
    dd = change_development_directory_get(cp, 0);
    mode = 0644 & ~change_umask(cp);

    //
    // Copy the file.
    //
    // Note: the file copy destroys anything in the development
    // direcory at both ``from'' and ``to''.
    //
    user_become(up);
    os_mkdir_between(dd, new_name, 02755);
    undo_unlink_errok(to);
    if (os_exists(to))
	os_unlink(to);
    copy_whole_file(from, to, 0);
    user_become_undo();
    str_free(from);
    str_free(to);

    //
    // remove the old file
    //
    change_file_whiteout_write(cp, old_name, up);
}


static void
move_file_main(void)
{
    sub_context_ty  *scp;
    string_ty	    *s1;
    string_ty	    *s2;
    string_ty	    *project_name;
    project_ty	    *pp;
    long	    change_number;
    change_ty	    *cp;
    log_style_ty    log_style;
    user_ty	    *up;
    size_t	    k;
    string_list_ty  search_path;
    string_list_ty  wl_in;
    string_list_ty  wl_nf;
    string_list_ty  wl_nt;
    string_list_ty  wl_rm;
    string_list_ty  wl_args;
    move_list_ty    ml;
    int		    based;
    string_ty	    *base;
    size_t          j;
    size_t          i;

    trace(("move_file_main()\n{\n"));
    arglex();
    project_name = 0;
    change_number = 0;
    log_style = log_style_append_default;
    string_list_constructor(&wl_args);
    move_list_constructor(&ml);
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(move_file_usage);
	    continue;

	case arglex_token_string:
	    s1 = str_from_c(arglex_value.alv_string);
            string_list_append(&wl_args, s1);
            str_free(s1);
            break;

	case arglex_token_change:
	    arglex();
	    // fall through...

	case arglex_token_number:
	    arglex_parse_change(&project_name, &change_number, move_file_usage);
	    continue;

	case arglex_token_project:
	    arglex();
	    arglex_parse_project(&project_name, move_file_usage);
	    continue;

	case arglex_token_nolog:
	    if (log_style == log_style_none)
		duplicate_option(move_file_usage);
	    log_style = log_style_none;
	    break;

	case arglex_token_wait:
	case arglex_token_wait_not:
	    user_lock_wait_argument(move_file_usage);
	    break;

	case arglex_token_whiteout:
	case arglex_token_whiteout_not:
	    user_whiteout_argument(move_file_usage);
	    break;

	case arglex_token_base_relative:
	case arglex_token_current_relative:
	    user_relative_filename_preference_argument(move_file_usage);
	    break;
	}
	arglex();
    }

    if (wl_args.nstrings < 2)
    {
	error_intl(0, i18n("too few files named"));
	move_file_usage();
    }
    // It is an error if the user supply an odd number of
    // files/directories.
    if (wl_args.nstrings % 2)
    {
        error_intl(0, i18n("not an even number of args"));
        move_file_usage();
    }


    //
    // locate project data
    //
    if (!project_name)
	project_name = user_default_project();
    pp = project_alloc(project_name);
    str_free(project_name);
    project_bind_existing(pp);

    //
    // locate user data
    //
    up = user_executing(pp);

    //
    // locate change data
    //
    if (!change_number)
	change_number = user_default_change(up);
    cp = change_alloc(pp, change_number);
    change_bind_existing(cp);

    //
    // lock the change file
    //
    change_cstate_lock_prepare(cp);
    lock_take();

    //
    // It is an error if the change is not in the in_development state.
    // It is an error if the change is not assigned to the current user.
    //
    if (!change_is_being_developed(cp))
	change_fatal(cp, 0, i18n("bad mv state"));
    if (change_is_a_branch(cp))
	change_fatal(cp, 0, i18n("bad branch cp"));
    if (!str_equal(change_developer_name(cp), user_name(up)))
	change_fatal(cp, 0, i18n("not developer"));

    //
    // resolve the path of each file
    // 1. the absolute path of the file name is obtained
    // 2. if the file is inside the development directory, ok
    // 3. if the file is inside the baseline, ok
    // 4. if neither, error
    //
    // To cope with automounters, directories are stored as given,
    // or are derived from the home directory in the passwd file.
    // Within aegis, pathnames have their symbolic links resolved,
    // and any comparison of paths is done on this "system idea"
    // of the pathname.
    //
    change_search_path_get(cp, &search_path, 1);

    //
    // Find the base for relative filenames.
    //
    based =
	(
	    search_path.nstrings >= 1
	&&
	    (
		user_relative_filename_preference
		(
		    up,
		    uconf_relative_filename_preference_current
		)
	    ==
		uconf_relative_filename_preference_base
	    )
	);
    if (based)
	base = search_path.string[0];
    else
    {
	os_become_orig();
	base = os_curdir();
	os_become_undo();
    }

    for (i = 0; i < (wl_args.nstrings - 1); ++i)
    {
        string_ty *old_name;
        string_ty *new_name;

        trace(("i=%d; old = \"%s\"; new = \"%s\"; \n",
               i,
               wl_args.string[i]->str_text,
               wl_args.string[i+1]->str_text));
        assert(0 == (i % 2));
        old_name = str_copy(wl_args.string[i]);
        new_name = str_copy(wl_args.string[++i]);

        //
        // Make the old name absolute, now we know where to make it
        // relative to.
        //
        if (old_name->str_text[0] != '/')
        {
            s1 = os_path_join(base, old_name);
            str_free(old_name);
            old_name = s1;
        }
        user_become(up);
        s1 = os_pathname(old_name, 1);
        user_become_undo();
        str_free(old_name);
        old_name = s1;

        //
        // Find the old name, relative to the project tree.
        //
        s2 = 0;
        for (k = 0; k < search_path.nstrings; ++k)
        {
            s2 = os_below_dir(search_path.string[k], old_name);
            if (s2)
                break;
        }
        if (!s2)
        {
            scp = sub_context_new();
            sub_var_set_string(scp, "File_Name", old_name);
            change_fatal(cp, scp, i18n("$filename unrelated"));
            // NOTREACHED
            sub_context_delete(scp);
        }
        str_free(old_name);
        old_name = s2;

        //
        // Make the new name absolute, now we know where to make it
        // relative to.
        //
        if (new_name->str_text[0] != '/')
        {
            s1 = os_path_join(base, new_name);
            str_free(new_name);
            new_name = s1;
        }
        user_become(up);
        s1 = os_pathname(new_name, 1);
        user_become_undo();
        str_free(new_name);
        new_name = s1;

        //
        // Find the new name, relative to the project tree.
        //
        s2 = 0;
        for (k = 0; k < search_path.nstrings; ++k)
        {
            s2 = os_below_dir(search_path.string[k], new_name);
            if (s2)
	    break;
        }
        if (!s2)
        {
            scp = sub_context_new();
            sub_var_set_string(scp, "File_Name", new_name);
            change_fatal(cp, scp, i18n("$filename unrelated"));
            // NOTREACHED
            sub_context_delete(scp);
        }
        str_free(new_name);
        new_name = s2;

        //
        // If the old file was a directory, then move all of its
        // contents into the new directory.	 If the old file was not a
        // directory, just move it.	 All checks to see if the action is
        // valid are done in the inner function.
        //
        string_list_constructor(&wl_in);
        project_file_directory_query(pp, old_name, &wl_in, 0, view_path_simple);
        if (wl_in.nstrings)
        {
            for (j = 0; j < wl_in.nstrings; ++j)
            {
                string_ty	    *filename_old;
                string_ty	    *filename_tail;
                string_ty	    *filename_new;

                //
                // Note: old_name and new_name will be empty
                // strings if they refer to the top of the
                // development directory tree.
                //
                filename_old = wl_in.string[j];
                if (old_name->str_length)
                {
                    filename_tail = os_below_dir(old_name, filename_old);
                    if (!filename_tail)
		    this_is_a_bug();
                }
                else
                {
                    // top-level directory
                    filename_tail = str_copy(filename_old);
                }
                if (new_name->str_length)
                    filename_new = os_path_join(new_name, filename_tail);
                else
                    filename_new = str_copy(filename_tail);
                str_free(filename_tail);

                move_list_append_create(&ml, filename_old, filename_new);

                str_free(filename_old);
                str_free(filename_new);
            }
        }
        else
        {
            move_list_append_create(&ml, old_name, new_name);
        }
        string_list_destructor(&wl_in);
        str_free(old_name);
        str_free(new_name);
    }

    string_list_destructor(&search_path);

    string_list_constructor(&wl_nf);
    string_list_constructor(&wl_nt);
    string_list_constructor(&wl_rm);
    if (ml.length)
    {
        for (j = 0; j < ml.length; ++j)
        {
            move_file_innards
            (
                up,
                cp,
                ml.item[j].from,
                ml.item[j].to,
                &wl_nf,
                &wl_nt,
                &wl_rm
            );
        }
    }


    if (wl_rm.nstrings != wl_nf.nstrings + wl_nt.nstrings)
        this_is_a_bug();


    //
    // the number of files changed,
    // so stomp on the validation fields.
    //
    change_build_times_clear(cp);

    //
    // If the file manifest of the change is altered (e.g. by aenf, aenfu,
    // aecp, aecpu, etc), or the contents of any file is changed, the
    // UUID is cleared.  This is because it is no longer the same change
    // as was received by aedist or aepatch, and the UUID is invalidated.
    //
    change_uuid_clear(cp);

    //
    // release the locks
    //
    change_cstate_write(cp);
    commit();
    lock_release();

    //
    // verbose success message
    //
    for (j = 0; j < ml.length; ++j)
    {
        scp = sub_context_new();
        sub_var_set_string(scp, "File_Name1", ml.item[j].from);
        sub_var_set_string(scp, "File_Name2", ml.item[j].to);
        change_verbose(cp, scp, i18n("move $filename1 to $filename2 complete"));
        sub_context_delete(scp);
    }
    move_list_destructor(&ml);

    //
    // run the change file command
    //
    log_open(change_logfile_get(cp), up, log_style);
    if (wl_nf.nstrings)
	change_run_new_file_command(cp, &wl_nf, up);
    if (wl_nt.nstrings)
	change_run_new_file_command(cp, &wl_nt, up);
    if (wl_rm.nstrings)
	change_run_remove_file_command(cp, &wl_rm, up);
    string_list_destructor(&wl_nf);
    string_list_destructor(&wl_nt);
    string_list_destructor(&wl_rm);
    change_run_project_file_command(cp, up);
    project_free(pp);
    change_free(cp);
    user_free(up);
    trace(("}\n"));
}


void
move_file(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
	{arglex_token_help, move_file_help, },
	{arglex_token_list, move_file_list, },
    };

    trace(("move_file()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), move_file_main);
    trace(("}\n"));
}

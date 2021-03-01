//
//	aegis - project change supervisor
//	Copyright (C) 1991-2008 Peter Miller
//	Copyright (C) 2006, 2008 Walter Franzini;
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/fcntl.h>
#include <common/ac/unistd.h>

#include <common/error.h>
#include <common/progname.h>
#include <common/quit.h>
#include <common/str_list.h>
#include <common/trace.h>
#include <common/uuidentifier.h>
#include <libaegis/ael/project/files.h>
#include <libaegis/arglex2.h>
#include <libaegis/arglex/change.h>
#include <libaegis/arglex/project.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>
#include <libaegis/change/identifier.h>
#include <libaegis/col.h>
#include <libaegis/commit.h>
#include <libaegis/dir.h>
#include <libaegis/file.h>
#include <libaegis/help.h>
#include <libaegis/lock.h>
#include <libaegis/log.h>
#include <libaegis/os.h>
#include <libaegis/pconf.h>
#include <libaegis/project/file.h>
#include <libaegis/project.h>
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
    string_ty	    *dd;
    int		    found;
    int		    used;
};


static void
walker(void *p, dir_walk_message_ty msg, string_ty *path, const struct stat *)
{
    walker_ty	    *aux;
    string_ty	    *s;
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
	    !change_file_find(aux->cp, s, view_path_first)
	&&
	    !project_file_find(aux->cp->pp, s, view_path_extreme)
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
new_file_main(void)
{
    string_ty	    *dd;
    size_t	    j;
    size_t	    k;
    string_ty	    *s1;
    string_ty	    *s2;
    string_ty	    *project_name;
    project_ty	    *pp;
    long	    change_number;
    change::pointer cp;
    log_style_ty    log_style;
    user_ty::pointer up;
    file_usage_ty   file_usage_value;
    int             auto_config_allowed;
    int		    nerrs;
    string_list_ty  search_path;
    int		    based;
    string_ty	    *base;
    int		    use_template;
    string_ty       *uuid;

    trace(("new_file_main()\n{\n"));
    arglex();
    project_name = 0;
    change_number = 0;
    file_usage_value = file_usage_source;
    string_list_ty wl;
    log_style = log_style_append_default;
    nerrs = 0;
    use_template = -1;
    auto_config_allowed = 1;
    uuid = 0;
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
	    s2 = str_from_c(arglex_value.alv_string);
	    wl.push_back(s2);
	    str_free(s2);
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
            if (uuid)
                duplicate_option(new_file_usage);
            if (arglex () != arglex_token_string)
                option_needs_string(arglex_token_uuid, new_file_usage);
            s2 = str_from_c(arglex_value.alv_string);
            if (!universal_unique_identifier_valid(s2))
		option_needs_string(arglex_token_uuid, new_file_usage);
	    uuid = str_downcase(s2);
	    str_free(s2);
	    s2 = 0;
            break;

	case arglex_token_keep:
	case arglex_token_interactive:
	case arglex_token_keep_not:
	    user_ty::delete_file_argument(new_file_usage);
	    break;
	}
	arglex();
    }
    if (nerrs)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_long(scp, "Number", nerrs);
	sub_var_optional(scp, "Number");
	fatal_intl(scp, i18n("no new files"));
	// NOTREACHED
	sub_context_delete(scp);
    }
    if (!wl.nstrings)
    {
	error_intl(0, i18n("no file names"));
	new_file_usage();
    }

    //
    //  It is an error if the -uuid switch is used and more that one
    //  file is nominated on the command line.
    //
    if (uuid && wl.nstrings != 1)
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
    pp = project_alloc(project_name);
    str_free(project_name);
    pp->bind_existing();

    //
    // locate user data
    //
    up = user_ty::create();

    //
    // locate change data
    //
    if (!change_number)
	change_number = up->default_change(pp);
    cp = change_alloc(pp, change_number);
    change_bind_existing(cp);

    //
    // lock the change file
    //
    change_cstate_lock_prepare(cp);
    lock_take();

    log_open(change_logfile_get(cp), up, log_style);

    if (change_file_promote(cp))
    {
	//
	// Write out the file state, and then let go of the locks
	// and take them again.  This ensures the data is consistent
	// for the next stage of processing.
	//
	trace(("Write out what we've done so far.\n"));
	change_cstate_write(cp);
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
    if (change_is_a_branch(cp))
	change_fatal(cp, 0, i18n("bad nf branch"));
    if (nstring(change_developer_name(cp)) != up->name())
	change_fatal(cp, 0, i18n("not developer"));

    //
    // It is an error if the UUID is already is use.
    //
    if (uuid)
    {
	fstate_src_ty   *src;

	src = change_file_find_uuid(cp, uuid, view_path_simple);
	if (src)
	{
	    sub_context_ty  *scp;

	    scp = sub_context_new();
    	    sub_var_set_string(scp, "Other", src->file_name);
	    sub_var_optional(scp, "Other");
	    change_fatal(cp, scp, i18n("bad ca, uuid duplicate"));
	    // NOTREACHED
	}
    }

    //
    // Search list for resolving filenames.
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
		up->relative_filename_preference
		(
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

    //
    // resolve the path of each file
    // 1.   the absolute path of the file name is obtained
    // 2.   if the file is inside the development directory, ok
    // 3.   if the file is inside the baseline, ok
    // 4.   if neither, error
    //
    string_list_ty wl2;
    for (j = 0; j < wl.nstrings; ++j)
    {
	s1 = wl.string[j];
	if (s1->str_text[0] == '/')
	    s2 = str_copy(s1);
	else
	    s2 = os_path_join(base, s1);
	up->become_begin();
	s1 = os_pathname(s2, 1);
	up->become_end();
	str_free(s2);
	s2 = 0;
	for (k = 0; k < search_path.nstrings; ++k)
	{
	    s2 = os_below_dir(search_path.string[k], s1);
	    if (s2)
		break;
	}
	str_free(s1);
	if (!s2)
	{
	    sub_context_ty  *scp;

	    scp = sub_context_new();
	    sub_var_set_string(scp, "File_Name", wl.string[j]);
	    change_error(cp, scp, i18n("$filename unrelated"));
	    sub_context_delete(scp);
	    ++nerrs;
	    continue;
	}
	if (wl2.member(s2))
	{
	    sub_context_ty  *scp;

	    scp = sub_context_new();
	    sub_var_set_string(scp, "File_Name", s2);
	    change_error(cp, scp, i18n("too many $filename"));
	    sub_context_delete(scp);
	    ++nerrs;
	}
	else
	    wl2.push_back(s2);
	str_free(s2);
    }
    wl = wl2;

    //
    // check that each filename is OK
    //
    // If a directory is named, extract the files from beneath it.
    //
    dd = change_development_directory_get(cp, 0);
    wl2.clear();
    for (j = 0; j < wl.nstrings; ++j)
    {
	string_ty	*fn;
	string_ty	*ffn;
	string_ty	*e;

	fn = wl.string[j];
	up->become_begin();
	ffn = os_path_cat(dd, fn);
	if (os_isa_directory(ffn))
	{
	    walker_ty	    aux;

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
		sub_context_ty	*scp;

		scp = sub_context_new();
		if (fn->str_length)
		    sub_var_set_string(scp, "File_Name", fn);
		else
		    sub_var_set_charstar(scp, "File_Name", ".");
		sub_var_set_long(scp, "Number", aux.found);
		sub_var_optional(scp, "Number");
		change_error
		(
		    cp,
		    scp,
		    i18n("directory $filename contains no relevant files")
		);
		sub_context_delete(scp);
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
	    sub_context_ty *scp = sub_context_new();
	    sub_var_set_string(scp, "File_Name", fn);
	    change_error(cp, scp, i18n("$filename bad nf type"));
	    sub_context_delete(scp);
	    ++nerrs;
	    up->become_end();
	    str_free(ffn);
	    continue;
	}
	str_free(ffn);
	up->become_end();
	e = change_filename_check(cp, fn);
	if (e)
	{
	    sub_context_ty  *scp;

	    //
	    // no internationalization of the error string
	    // required, this is done inside the
	    // change_filename_check function.
	    //
	    scp = sub_context_new();
	    sub_var_set_string(scp, "MeSsaGe", e);
	    change_error(cp, scp, i18n("$message"));
	    sub_context_delete(scp);
	    ++nerrs;
	    str_free(e);
	    break;
	}

	//
	// Remember this one.
	//
	wl2.push_back(fn);
    }
    if (nerrs)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_long(scp, "Number", nerrs);
	sub_var_optional(scp, "Number");
	change_fatal(cp, scp, i18n("no new files"));
	sub_context_delete(scp);
    }
    wl = wl2;

    //
    // ensure that each file
    // 1. is not already part of the change
    //        - except removed files
    // 2. is not already part of the baseline
    // 3. does not have a directory conflict
    //
    for (j = 0; j < wl.nstrings; ++j)
    {
	fstate_src_ty   *src_data;

	s1 = wl.string[j];
	s2 = change_file_directory_conflict(cp, s1);
	if (s2)
	{
	    sub_context_ty  *scp;

	    scp = sub_context_new();
	    sub_var_set_string(scp, "File_Name", s1);
	    sub_var_set_string(scp, "File_Name2", s2);
	    sub_var_optional(scp, "File_Name2");
	    change_error
	    (
		cp,
		scp,
		i18n("file $filename directory name conflict")
	    );
	    sub_context_delete(scp);
	    ++nerrs;
	    continue;
	}
	s2 = project_file_directory_conflict(pp, s1);
	if (s2)
	{
	    sub_context_ty  *scp;

	    scp = sub_context_new();
	    sub_var_set_string(scp, "File_Name", s1);
	    sub_var_set_string(scp, "File_Name2", s2);
	    sub_var_optional(scp, "File_Name2");
	    project_error
	    (
		pp,
		scp,
		i18n("file $filename directory name conflict")
	    );
	    sub_context_delete(scp);
	    ++nerrs;
	    continue;
	}
	src_data = change_file_find(cp, s1, view_path_first);
	if (src_data)
	{
	    switch (src_data->action)
	    {
		sub_context_ty  *scp;

	    case file_action_remove:
		break;

	    case file_action_create:
	    case file_action_modify:
	    case file_action_insulate:
	    case file_action_transparent:
#ifndef DEBUG
	    default:
#endif
		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", s1);
		change_error(cp, scp, i18n("file $filename dup"));
		sub_context_delete(scp);
		++nerrs;
		break;
	    }
	}
	else
	{
	    src_data = project_file_find(pp, s1, view_path_extreme);
	    if (src_data)
	    {
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", s1);
		project_error(pp, scp, i18n("$filename in baseline"));
		sub_context_delete(scp);
		++nerrs;
	    }
	}
    }
    if (nerrs)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_long(scp, "Number", nerrs);
	sub_var_optional(scp, "Number");
	change_fatal(cp, scp, i18n("no new files"));
	sub_context_delete(scp);
    }

    //
    // Create each file in the development directory,
    // if it does not already exist.
    // Create any necessary directories along the way.
    //
    for (j = 0; j < wl.nstrings; ++j)
    {
	change_file_template(cp, wl.string[j], up, use_template);
    }

    //
    // Add each file to the change file,
    // and write it back out.
    //
    for (j = 0; j < wl.nstrings; ++j)
    {
	fstate_src_ty   *src_data;
	file_usage_ty   usage;

	s1 = wl.string[j];

	//
	// If the file is already in the change (we checked for this
	// earlier) then it must be being removed, and we are replacing
	// it, so we can change its type.
	//
	src_data = change_file_find(cp, s1, view_path_first);
	if (src_data)
	{
	    assert(src_data->action == file_action_remove);
	    if (src_data->uuid)
	    {
		if (uuid)
		    duplicate_option_by_name(arglex_token_uuid, new_file_usage);
		//
		// If the file is moved and not removed we cannot
		// reassign the UUID because this lead to duplicate
		// UUIDs.
		//
                if (!src_data->move)
                {
                    uuid = src_data->uuid;
                    src_data->uuid = 0;
                }
	    }
	    change_file_remove(cp, s1);
	}

	//
	// The project config file doesn't need to have the -config
	// option specifically supplied.
	//
	// You have to do this test BEFORE the change_file_new call.
	//
	usage = file_usage_value;
	if (auto_config_allowed && change_file_is_config(cp, s1))
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

	src_data = cp->file_new(s1);
	src_data->action = file_action_create;
	src_data->usage = usage;
        if (uuid)
        {
            assert(universal_unique_identifier_valid(uuid));
            src_data->uuid = uuid;
	    uuid = 0;
        }
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
    change_cstate_write(cp);
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
    for (j = 0; j < wl.nstrings; ++j)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_string(scp, "File_Name", wl.string[j]);
	change_verbose(cp, scp, i18n("new file $filename completed"));
	sub_context_delete(scp);
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
    };

    trace(("new_file()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), new_file_main);
    trace(("}\n"));
}

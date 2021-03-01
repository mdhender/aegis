/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-2003 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to implement new file
 */

#include <ac/stdio.h>
#include <ac/stdlib.h>
#include <ac/fcntl.h>
#include <ac/unistd.h>

#include <ael/project/files.h>
#include <aenf.h>
#include <arglex2.h>
#include <change/branch.h>
#include <change/file.h>
#include <col.h>
#include <commit.h>
#include <dir.h>
#include <error.h>
#include <file.h>
#include <help.h>
#include <lock.h>
#include <log.h>
#include <os.h>
#include <pconf.h>
#include <progname.h>
#include <project.h>
#include <project/file.h>
#include <sub.h>
#include <trace.h>
#include <user.h>
#include <str_list.h>


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
    string_ty	    *project_name;
    long	    change_number;

    trace(("new_file_list()\n{\n"));
    project_name = 0;
    change_number = 0;
    arglex();
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(new_file_usage);
	    continue;

	case arglex_token_change:
	    if (arglex() != arglex_token_number)
		option_needs_number(arglex_token_change, new_file_usage);
	    /* fall through... */

	case arglex_token_number:
	    if (change_number)
		duplicate_option_by_name(arglex_token_change, new_file_usage);
	    change_number = arglex_value.alv_number;
	    if (change_number == 0)
		change_number = MAGIC_ZERO;
	    else if (change_number < 1)
	    {
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set_long(scp, "Number", change_number);
		fatal_intl(scp, i18n("change $number out of range"));
		/* NOTREACHED */
		sub_context_delete(scp);
	    }
	    break;

	case arglex_token_project:
	    if (arglex() != arglex_token_string)
		option_needs_name(arglex_token_project, new_file_usage);
	    if (project_name)
		duplicate_option_by_name(arglex_token_project, new_file_usage);
	    project_name = str_from_c(arglex_value.alv_string);
	    break;
	}
	arglex();
    }
    list_project_files(project_name, change_number, 0);
    if (project_name)
	str_free(project_name);
    trace(("}\n"));
}


typedef struct walker_ty walker_ty;
struct walker_ty
{
    user_ty	    *up;
    change_ty	    *cp;
    string_list_ty  *slp;
    string_ty	    *dd;
    int		    found;
    int		    used;
};


static void
walker(void *p, dir_walk_message_ty msg, string_ty *path, struct stat *st)
{
    walker_ty	    *aux;
    string_ty	    *s;

    aux = p;
    switch (msg)
    {
    case dir_walk_dir_before:
    case dir_walk_dir_after:
	break;

    case dir_walk_special:
    case dir_walk_symlink:
	/*
	 * Ignore special files and symlinks.
	 * If they need them, they can create them at build time.
	 */
	break;

    case dir_walk_file:
	/*
	 * Ignore files we already know about.
	 * Ignore files the change already knows about.
	 * Ignore files the project already knows about.
	 */
	aux->found++;
	s = os_below_dir(aux->dd, path);
	user_become_undo();
	if
	(
	    !string_list_member(aux->slp, s)
	&&
	    !change_file_find(aux->cp, s)
	&&
	    !project_file_find(aux->cp->pp, s, view_path_extreme)
	)
	{
	    string_list_append(aux->slp, s);
	    aux->used++;
	}
	user_become(aux->up);
	str_free(s);
	break;
    }
}


static void
new_file_main(void)
{
    string_ty	    *dd;
    string_list_ty  wl;
    cstate	    cstate_data;
    size_t	    j;
    size_t	    k;
    string_ty	    *s1;
    string_ty	    *s2;
    string_ty	    *project_name;
    project_ty	    *pp;
    long	    change_number;
    change_ty	    *cp;
    log_style_ty    log_style;
    user_ty	    *up;
    int		    generated;
    int		    nerrs;
    string_list_ty  search_path;
    string_list_ty  wl2;
    int		    based;
    string_ty	    *base;
    pconf	    pconf_data;
    int		    use_template;

    trace(("new_file_main()\n{\n"));
    arglex();
    project_name = 0;
    change_number = 0;
    generated = 0;
    string_list_constructor(&wl);
    log_style = log_style_append_default;
    nerrs = 0;
    use_template = -1;
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
	    /* fall through... */

	case arglex_token_string:
	    s2 = str_from_c(arglex_value.alv_string);
	    string_list_append(&wl, s2);
	    str_free(s2);
	    break;

	case arglex_token_change:
	    if (arglex() != arglex_token_number)
		option_needs_number(arglex_token_change, new_file_usage);
	    /* fall through... */

	case arglex_token_number:
	    if (change_number)
		duplicate_option_by_name(arglex_token_change, new_file_usage);
	    change_number = arglex_value.alv_number;
	    if (change_number == 0)
		change_number = MAGIC_ZERO;
	    else if (change_number < 1)
	    {
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set_long(scp, "Number", change_number);
		fatal_intl(scp, i18n("change $number out of range"));
		/* NOTREACHED */
		sub_context_delete(scp);
	    }
	    break;

	case arglex_token_project:
	    if (arglex() != arglex_token_string)
		option_needs_name(arglex_token_project, new_file_usage);
	    if (project_name)
		duplicate_option_by_name(arglex_token_project, new_file_usage);
	    project_name = str_from_c(arglex_value.alv_string);
	    break;

	case arglex_token_nolog:
	    if (log_style == log_style_none)
		duplicate_option(new_file_usage);
	    log_style = log_style_none;
	    break;

	case arglex_token_build:
	    if (generated)
		duplicate_option(new_file_usage);
	    generated = 1;
	    break;

	case arglex_token_wait:
	case arglex_token_wait_not:
	    user_lock_wait_argument(new_file_usage);
	    break;

	case arglex_token_base_relative:
	case arglex_token_current_relative:
	    user_relative_filename_preference_argument(new_file_usage);
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
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_long(scp, "Number", nerrs);
	sub_var_optional(scp, "Number");
	fatal_intl(scp, i18n("no new files"));
	/* NOTREACHED */
	sub_context_delete(scp);
    }
    if (!wl.nstrings)
    {
	error_intl(0, i18n("no file names"));
	new_file_usage();
    }

    /*
     * locate project data
     */
    if (!project_name)
	project_name = user_default_project();
    pp = project_alloc(project_name);
    str_free(project_name);
    project_bind_existing(pp);

    /*
     * locate user data
     */
    up = user_executing(pp);

    /*
     * locate change data
     */
    if (!change_number)
	change_number = user_default_change(up);
    cp = change_alloc(pp, change_number);
    change_bind_existing(cp);

    /*
     * lock the change file
     */
    change_cstate_lock_prepare(cp);
    lock_take();
    cstate_data = change_cstate_get(cp);

    log_open(change_logfile_get(cp), up, log_style);

    /*
     * It is an error if the change is not in the being_developed state.
     * It is an error if the change is not assigned to the current user.
     */
    if (cstate_data->state != cstate_state_being_developed)
	change_fatal(cp, 0, i18n("bad nf state"));
    if (change_is_a_branch(cp))
	change_fatal(cp, 0, i18n("bad nf branch"));
    if (!str_equal(change_developer_name(cp), user_name(up)))
	change_fatal(cp, 0, i18n("not developer"));

    /*
     * Search list for resolving filenames.
     */
    change_search_path_get(cp, &search_path, 1);

    /*
     * Find the base for relative filenames.
     */
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

    /*
     * resolve the path of each file
     * 1.   the absolute path of the file name is obtained
     * 2.   if the file is inside the development directory, ok
     * 3.   if the file is inside the baseline, ok
     * 4.   if neither, error
     */
    string_list_constructor(&wl2);
    for (j = 0; j < wl.nstrings; ++j)
    {
	s1 = wl.string[j];
	if (s1->str_text[0] == '/')
	    s2 = str_copy(s1);
	else
	    s2 = str_format("%S/%S", base, s1);
	user_become(up);
	s1 = os_pathname(s2, 1);
	user_become_undo();
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
	if (string_list_member(&wl2, s2))
	{
	    sub_context_ty  *scp;

	    scp = sub_context_new();
	    sub_var_set_string(scp, "File_Name", s2);
	    change_error(cp, scp, i18n("too many $filename"));
	    sub_context_delete(scp);
	    ++nerrs;
	}
	else
	    string_list_append(&wl2, s2);
	str_free(s2);
    }
    string_list_destructor(&search_path);
    string_list_destructor(&wl);
    wl = wl2;

    /*
     * check that each filename is OK
     *
     * If a directory is named, extract the files from beneath it.
     */
    pconf_data = change_pconf_get(cp, 0);
    dd = change_development_directory_get(cp, 0);
    string_list_constructor(&wl2);
    for (j = 0; j < wl.nstrings; ++j)
    {
	string_ty	*fn;
	string_ty	*ffn;
	string_ty	*e;

	fn = wl.string[j];
	if (generated && change_file_is_config(cp, fn))
	{
	    sub_context_ty  *scp;

	    scp = sub_context_new();
	    sub_var_set_string(scp, "File_Name", fn);
	    change_error(cp, scp, i18n("may not build $filename"));
	    sub_context_delete(scp);
	    ++nerrs;
	}
	user_become(up);
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

	    /*
	     * It's an error if there is nothing to do for
	     * this directory.
	     */
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
	    user_become_undo();
	    str_free(ffn);
	    continue;
	}
	if (os_isa_special_file(ffn))
	{
	    if (pconf_data->create_symlinks_before_build)
	    {
		/*
		 * In the case where we are using a
		 * symlink farm, this is probably a link
		 * into the baseline.  Just nuke it,
		 * and keep going.
		 */
		os_unlink(ffn);
	    }
	    else
	    {
		/*
		 * If the file exists, and isn't a normal file,
		 * and isn't a directory, you can't add it as
		 * a source file.
		 */
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", fn);
		change_error(cp, scp, i18n("$filename bad nf type"));
		sub_context_delete(scp);
		++nerrs;
		user_become_undo();
		str_free(ffn);
		continue;
	    }
	}
	str_free(ffn);
	user_become_undo();
	e = change_filename_check(cp, fn);
	if (e)
	{
	    sub_context_ty  *scp;

	    /*
	     * no internationalization of the error string
	     * required, this is done inside the
	     * change_filename_check function.
	     */
	    scp = sub_context_new();
	    sub_var_set_string(scp, "Message", e);
	    change_error(cp, scp, i18n("$message"));
	    sub_context_delete(scp);
	    ++nerrs;
	    str_free(e);
	    break;
	}

	/*
	 * Remember this one.
	 */
	string_list_append(&wl2, fn);
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
    string_list_destructor(&wl);
    wl = wl2;

    /*
     * ensure that each file
     * 1. is not already part of the change
     *        - except removed files
     * 2. is not already part of the baseline
     * 3. does not have a directory conflict
     */
    for (j = 0; j < wl.nstrings; ++j)
    {
	fstate_src	src_data;

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
	src_data = change_file_find(cp, s1);
	if (src_data)
	{
	    if (src_data->action != file_action_remove)
	    {
		sub_context_ty  *scp;

		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", s1);
		change_error(cp, scp, i18n("file $filename dup"));
		sub_context_delete(scp);
		++nerrs;
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

    /*
     * Create each file in the development directory,
     * if it does not already exist.
     * Create any necessary directories along the way.
     */
    for (j = 0; j < wl.nstrings; ++j)
    {
	change_file_template(cp, wl.string[j], up, use_template);
    }

    /*
     * Add each file to the change file,
     * and write it back out.
     */
    for (j = 0; j < wl.nstrings; ++j)
    {
	fstate_src	src_data;

	s1 = wl.string[j];

	/*
	 * If the file is already in the change (we checked for this
	 * earlier) then it must be being removed, and we are replacing
	 * it, so we can change its type.
	 */
	src_data = change_file_find(cp, s1);
	if (src_data)
	{
	    assert(src_data->action == file_action_remove);
	    change_file_remove(cp, s1);
	}

	src_data = change_file_new(cp, s1);
	src_data->action = file_action_create;
	if (generated)
	    src_data->usage = file_usage_build;
	else
	    src_data->usage = file_usage_source;
    }

    /*
     * the number of files changed, or the version did,
     * so stomp on the validation fields.
     */
    change_build_times_clear(cp);

    /*
     * update the copyright years
     */
    change_copyright_years_now(cp);

    /*
     * run the change file command
     * and the project file command if necessary
     */
    change_run_new_file_command(cp, &wl, up);
    change_run_project_file_command(cp, up);

    /*
     * release the locks
     */
    change_cstate_write(cp);
    commit();
    lock_release();

    /*
     * verbose success message
     */
    for (j = 0; j < wl.nstrings; ++j)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_string(scp, "File_Name", wl.string[j]);
	change_verbose(cp, scp, i18n("new file $filename completed"));
	sub_context_delete(scp);
    }
    string_list_destructor(&wl);
    change_free(cp);
    project_free(pp);
    user_free(up);
    trace(("}\n"));
}


void
new_file(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
	{arglex_token_help, new_file_help, },
	{arglex_token_list, new_file_list, },
    };

    trace(("new_file()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), new_file_main);
    trace(("}\n"));
}

/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-1999, 2001-2003 Peter Miller;
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
 * MANIFEST: functions to implement remove file
 */

#include <ac/stdio.h>
#include <ac/stdlib.h>

#include <ael/project/files.h>
#include <aerm.h>
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
#include <os.h>
#include <progname.h>
#include <project.h>
#include <project/file.h>
#include <sub.h>
#include <trace.h>
#include <undo.h>
#include <user.h>
#include <str_list.h>


/*
 * NAME
 *	remove_file_usage
 *
 * SYNOPSIS
 *	void remove_file_usage(void);
 *
 * DESCRIPTION
 *	The remove_file_usage function is used to
 *	tell the user how to use the 'aegis -ReMove_file' command.
 */

static void
remove_file_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf
    (
	stderr,
	"usage: %s -ReMove_file <filename>... [ <option>... ]\n",
	progname
    );
    fprintf(stderr, "       %s -ReMove_file -List [ <option>... ]\n", progname);
    fprintf(stderr, "       %s -ReMove_file -Help\n", progname);
    quit(1);
}


/*
 * NAME
 *	remove_file_help
 *
 * SYNOPSIS
 *	void remove_file_help(void);
 *
 * DESCRIPTION
 *	The remove_file_help function is used to
 *	describe the 'aegis -ReMove_file' command to the user.
 */

static void
remove_file_help(void)
{
    help("aerm", remove_file_usage);
}


/*
 * NAME
 *	remove_file_list
 *
 * SYNOPSIS
 *	void remove_file_list(void);
 *
 * DESCRIPTION
 *	The remove_file_list function is used to
 *	list the file the user may wish to add to the change
 *	as a deletion.	All project files are listed.
 */

static void
remove_file_list(void)
{
    string_ty	    *project_name;
    long	    change_number;

    trace(("remove_file_list()\n{\n"));
    project_name = 0;
    change_number = 0;
    arglex();
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(remove_file_usage);
	    continue;

	case arglex_token_change:
	    arglex();
	    /* fall through... */

	case arglex_token_number:
	    arglex_parse_change
	    (
		&project_name,
		&change_number,
		remove_file_usage
	    );
	    continue;

	case arglex_token_project:
	    arglex();
	    /* fall through... */

	case arglex_token_string:
	    arglex_parse_project(&project_name, remove_file_usage);
	    continue;
	}
	arglex();
    }
    list_project_files(project_name, change_number, 0);
    if (project_name)
	str_free(project_name);
    trace(("}\n"));
}


/*
 * NAME
 *	remove_file_main
 *
 * SYNOPSIS
 *	void remove_file_main(void);
 *
 * DESCRIPTION
 *	The remove_file_main function is used to
 *	add a file to a change as a deletion.
 *	The file will be deleted from the baseline on successful
 *	integration of the change.
 *
 *	The names of the relevant files are gleaned from the command line.
 */

static void
remove_file_main(void)
{
    string_list_ty  wl;
    string_list_ty  wl2;
    string_list_ty  wl_in;
    string_ty	    *s1;
    string_ty	    *s2;
    cstate_ty	    *cstate_data;
    fstate_src_ty   *c_src_data;
    fstate_src_ty   *p_src_data;
    size_t	    j;
    size_t	    k;
    string_ty	    *project_name;
    project_ty	    *pp;
    long	    change_number;
    change_ty	    *cp;
    log_style_ty    log_style;
    user_ty	    *up;
    string_ty	    *dd;
    int		    number_of_errors;
    string_list_ty  search_path;
    string_ty	    *config_name;
    int		    config_seen;
    int		    based;
    string_ty	    *base;

    trace(("remove_file_main()\n{\n"));
    arglex();
    project_name = 0;
    change_number = 0;
    string_list_constructor(&wl);
    log_style = log_style_append_default;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(remove_file_usage);
	    continue;

	case arglex_token_file:
	case arglex_token_directory:
	    if (arglex() != arglex_token_string)
		remove_file_usage();
	    /* fall through... */

	case arglex_token_string:
	    s2 = str_from_c(arglex_value.alv_string);
	    string_list_append(&wl, s2);
	    str_free(s2);
	    break;

	case arglex_token_change:
	    arglex();
	    /* fall through... */

	case arglex_token_number:
	    arglex_parse_change
	    (
		&project_name,
		&change_number,
		remove_file_usage
	    );
	    continue;

	case arglex_token_project:
	    arglex();
	    arglex_parse_project(&project_name, remove_file_usage);
	    continue;

	case arglex_token_nolog:
	    if (log_style == log_style_none)
		duplicate_option(remove_file_usage);
	    log_style = log_style_none;
	    break;

	case arglex_token_wait:
	case arglex_token_wait_not:
	    user_lock_wait_argument(remove_file_usage);
	    break;

	case arglex_token_whiteout:
	case arglex_token_whiteout_not:
	    user_whiteout_argument(remove_file_usage);
	    break;

	case arglex_token_base_relative:
	case arglex_token_current_relative:
	    user_relative_filename_preference_argument(remove_file_usage);
	    break;
	}
	arglex();
    }
    if (!wl.nstrings)
    {
	error_intl(0, i18n("no file names"));
	remove_file_usage();
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
     * take the locks and read the change state
     */
    change_cstate_lock_prepare(cp);
    lock_take();
    cstate_data = change_cstate_get(cp);

    log_open(change_logfile_get(cp), up, log_style);

    /*
     * It is an error if the change is not in the in_development state.
     * It is an error if the change is not assigned to the current user.
     */
    if (cstate_data->state != cstate_state_being_developed)
	change_fatal(cp, 0, i18n("bad rm state"));
    if (change_is_a_branch(cp))
	change_fatal(cp, 0, i18n("bad nf branch"));
    if (!str_equal(change_developer_name(cp), user_name(up)))
	change_fatal(cp, 0, i18n("not developer"));

    /*
     * Where to search to resolve file names.
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
    number_of_errors = 0;
    string_list_constructor(&wl2);
    config_name = str_from_c(THE_CONFIG_FILE);
    config_seen = 0;
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
	    ++number_of_errors;
	    continue;
	}
	project_file_directory_query(pp, s2, &wl_in, 0, view_path_simple);
	if (wl_in.nstrings)
	{
	    int		    used;

	    /*
	     * If the user named a directory, add all of the
	     * source files in that directory, provided they
	     * are not already in the change.
	     */
	    used = 0;
	    for (k = 0; k < wl_in.nstrings; ++k)
	    {
		string_ty	*s3;

		s3 = wl_in.string[k];
		if (!change_file_find(cp, s3))
		{
		    if (string_list_member(&wl2, s3))
		    {
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set_string(scp, "File_Name", s3);
			change_error(cp, scp, i18n("too many $filename"));
			sub_context_delete(scp);
			++number_of_errors;
		    }
		    else
			string_list_append(&wl2, s3);
		    if (str_equal(s3, config_name))
			++config_seen;
		    ++used;
		}
	    }
	    if (!used)
	    {
		sub_context_ty	*scp;

		scp = sub_context_new();
		if (s2->str_length)
		    sub_var_set_string(scp, "File_Name", s2);
		else
		    sub_var_set_charstar(scp, "File_Name", ".");
		sub_var_set_long(scp, "Number", (long)wl_in.nstrings);
		sub_var_optional(scp, "Number");
		change_error
		(
		    cp,
		    scp,
		    i18n("directory $filename contains no relevant files")
		);
		sub_context_delete(scp);
		++number_of_errors;
	    }
	}
	else
	{
	    if (str_equal(s2, config_name))
		++config_seen;
	    if (string_list_member(&wl2, s2))
	    {
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", s2);
		change_error(cp, scp, i18n("too many $filename"));
		sub_context_delete(scp);
		++number_of_errors;
	    }
	    else
		string_list_append(&wl2, s2);
	}
	string_list_destructor(&wl_in);
	str_free(s2);
    }
    string_list_destructor(&wl);
    wl = wl2;
    string_list_destructor(&search_path);

    /*
     * You may not delete the top-level config file.
     *
     * This only applies to the top-level config file, the others
     * may be moved as you like, provided that everything is still
     * defined, and you will get a different error somewhere else
     * if that goes wrong.
     */
    if (config_seen)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_string(scp, "File_Name", config_name);
	change_error(cp, scp, i18n("may not remove $filename"));
	sub_context_delete(scp);
	++number_of_errors;
    }
    str_free(config_name);

    /*
     * ensure that each file
     * 1. is not already part of the change
     * 2. is in the baseline
     * add it to the change
     */
    for (j = 0; j < wl.nstrings; ++j)
    {
	s1 = wl.string[j];
	if (change_file_find(cp, s1))
	{
	    sub_context_ty  *scp;

	    scp = sub_context_new();
	    sub_var_set_string(scp, "File_Name", s1);
	    change_error(cp, scp, i18n("file $filename dup"));
	    sub_context_delete(scp);
	    ++number_of_errors;
	    continue;
	}
	p_src_data = project_file_find(pp, s1, view_path_extreme);
	if (!p_src_data)
	{
	    p_src_data = project_file_find_fuzzy(pp, s1, view_path_extreme);
	    if (p_src_data)
	    {
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", s1);
		sub_var_set_string(scp, "Guess", p_src_data->file_name);
		project_error(pp, scp, i18n("no $filename, closest is $guess"));
		sub_context_delete(scp);
	    }
	    else
	    {
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", s1);
		project_error(pp, scp, i18n("no $filename"));
		sub_context_delete(scp);
	    }
	    ++number_of_errors;
	    continue;
	}
	c_src_data = change_file_new(cp, s1);
	c_src_data->action = file_action_remove;
	c_src_data->usage = p_src_data->usage;

	/*
	 * p_src_data->edit_number
	 *	The head revision of the branch.
	 * p_src_data->edit_number_origin
	 *	The version originally copied.
	 *
	 * c_src_data->edit_number
	 *	Not meaningful until after integrate pass.
	 * c_src_data->edit_number_origin
	 *	The version originally copied.
	 * c_src_data->edit_number_origin_new
	 *	Updates branch edit_number_origin on
	 *	integrate pass.
	 */
	assert(p_src_data->edit);
	assert(p_src_data->edit->revision);
	c_src_data->edit_origin = history_version_copy(p_src_data->edit);
    }
    if (number_of_errors)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_long(scp, "Number", number_of_errors);
	sub_var_optional(scp, "Number");
	change_fatal(cp, scp, i18n("remove file fail"));
	/* NOTREACHED */
	sub_context_delete(scp);
    }

    /*
     * Create each file in the development directory,
     * and make it be gibberish, to generate syntax errors if it is used.
     * Create any necessary directories along the way.
     */
    dd = change_development_directory_get(cp, 0);
    for (j = 0; j < wl.nstrings; ++j)
    {
	/*
	 * Find the mod-time of the file in the project
	 */
	s1 = wl.string[j];

	/*
	 * Remove any existing file (this cleans up junk, and
	 * breaks the link if we are using symlink trees).
	 */
	change_file_whiteout_write(cp, s1, up);
    }

    /*
     * the number of files changed, or the version did,
     * so stomp on the validation fields.
     */
    change_build_times_clear(cp);

    /*
     * run the change file command
     * and the project file command if necessary
     */
    change_run_remove_file_command(cp, &wl, up);
    change_run_project_file_command(cp, up);

    /*
     * write the data and release the lock
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
	change_verbose(cp, scp, i18n("remove file $filename complete"));
	sub_context_delete(scp);
    }
    string_list_destructor(&wl);
    change_free(cp);
    project_free(pp);
    user_free(up);
    trace(("}\n"));
}


/*
 * NAME
 *	remove_file
 *
 * SYNOPSIS
 *	void remove_file(void);
 *
 * DESCRIPTION
 *	The remove_file function is used to
 *	dispatch the 'aegis -ReMove_file' command to the relevant
 *	function to do it's work.
 */

void
remove_file(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
	{arglex_token_help, remove_file_help, },
	{arglex_token_list, remove_file_list, },
    };

    trace(("remove_file()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), remove_file_main);
    trace(("}\n"));
}

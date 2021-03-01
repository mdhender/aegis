//
//	aegis - project change supervisor
//	Copyright (C) 1991-1999, 2001-2008 Peter Miller
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

#include <common/error.h>
#include <common/progname.h>
#include <common/quit.h>
#include <common/str_list.h>
#include <common/trace.h>
#include <libaegis/ael/project/files.h>
#include <libaegis/arglex/change.h>
#include <libaegis/arglex/project.h>
#include <libaegis/arglex2.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>
#include <libaegis/change/identifier.h>
#include <libaegis/commit.h>
#include <libaegis/file.h>
#include <libaegis/help.h>
#include <libaegis/lock.h>
#include <libaegis/log.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/project/file.h>
#include <libaegis/sub.h>
#include <libaegis/undo.h>
#include <libaegis/user.h>

#include <aegis/aerm.h>


//
// NAME
//	remove_file_usage
//
// SYNOPSIS
//	void remove_file_usage(void);
//
// DESCRIPTION
//	The remove_file_usage function is used to
//	tell the user how to use the 'aegis -ReMove_file' command.
//

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


//
// NAME
//	remove_file_help
//
// SYNOPSIS
//	void remove_file_help(void);
//
// DESCRIPTION
//	The remove_file_help function is used to
//	describe the 'aegis -ReMove_file' command to the user.
//

static void
remove_file_help(void)
{
    help("aerm", remove_file_usage);
}


//
// NAME
//	remove_file_list
//
// SYNOPSIS
//	void remove_file_list(void);
//
// DESCRIPTION
//	The remove_file_list function is used to
//	list the file the user may wish to add to the change
//	as a deletion.	All project files are listed.
//

static void
remove_file_list(void)
{
    trace(("remove_file_list()\n{\n"));
    arglex();
    change_identifier cid;
    cid.command_line_parse_rest(remove_file_usage);
    list_project_files(cid, 0);
    trace(("}\n"));
}


static int
count_config_files(change::pointer cp)
{
    size_t          j;
    size_t          result;

    result = 0;
    for (j = 0; ; ++j)
    {
	fstate_src_ty   *src;

	src = change_file_nth(cp, j, view_path_extreme);
	if (!src)
	    break;
	if (src->usage == file_usage_config)
	    ++result;
    }
    return result;
}



//
// NAME
//	remove_file_main
//
// SYNOPSIS
//	void remove_file_main(void);
//
// DESCRIPTION
//	The remove_file_main function is used to
//	add a file to a change as a deletion.
//	The file will be deleted from the baseline on successful
//	integration of the change.
//
//	The names of the relevant files are gleaned from the command line.
//

static void
remove_file_main(void)
{
    string_ty	    *s1;
    string_ty	    *s2;
    fstate_src_ty   *c_src_data;
    fstate_src_ty   *p_src_data;
    size_t	    j;
    size_t	    k;
    string_ty	    *project_name;
    project_ty	    *pp;
    long	    change_number;
    change::pointer cp;
    log_style_ty    log_style;
    user_ty::pointer up;
    int		    number_of_errors;
    string_list_ty  search_path;
    int		    config_seen;
    string_ty       *configuration_filename;
    int		    based;
    string_ty	    *base;

    trace(("remove_file_main()\n{\n"));
    arglex();
    project_name = 0;
    change_number = 0;
    string_list_ty wl;
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
	    user_ty::lock_wait_argument(remove_file_usage);
	    break;

	case arglex_token_whiteout:
	case arglex_token_whiteout_not:
	    user_ty::whiteout_argument(remove_file_usage);
	    break;

	case arglex_token_base_relative:
	case arglex_token_current_relative:
	    user_ty::relative_filename_preference_argument(remove_file_usage);
	    break;
	}
	arglex();
    }
    if (!wl.nstrings)
    {
	error_intl(0, i18n("no file names"));
	remove_file_usage();
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
    // take the locks and read the change state
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
    // It is an error if the change is not in the in_development state.
    // It is an error if the change is not assigned to the current user.
    //
    if (!cp->is_being_developed())
	change_fatal(cp, 0, i18n("bad rm state"));
    if (change_is_a_branch(cp))
	change_fatal(cp, 0, i18n("bad nf branch"));
    if (nstring(change_developer_name(cp)) != up->name())
	change_fatal(cp, 0, i18n("not developer"));

    //
    // Where to search to resolve file names.
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
    number_of_errors = 0;
    string_list_ty wl2;
    config_seen = 0;
    configuration_filename = 0;
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
	    ++number_of_errors;
	    continue;
	}
	string_list_ty wl_in;
	project_file_directory_query(pp, s2, &wl_in, 0, view_path_simple);
	if (wl_in.nstrings)
	{
	    int		    used;

	    //
	    // If the user named a directory, add all of the
	    // source files in that directory, provided they
	    // are not already in the change.
	    //
	    used = 0;
	    for (k = 0; k < wl_in.nstrings; ++k)
	    {
		string_ty	*s3;

		s3 = wl_in.string[k];
		if (!change_file_find(cp, s3, view_path_first))
		{
		    if (wl2.member(s3))
		    {
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set_string(scp, "File_Name", s3);
			change_error(cp, scp, i18n("too many $filename"));
			sub_context_delete(scp);
			++number_of_errors;
		    }
		    else
			wl2.push_back(s3);
		    if (change_file_is_config(cp, s3))
		    {
			++config_seen;
			if (!configuration_filename)
			    configuration_filename = str_copy(s3);
		    }
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
	    if (change_file_is_config(cp, s2))
	    {
		++config_seen;
       		if (!configuration_filename)
		    configuration_filename = str_copy(s2);
	    }
	    if (wl2.member(s2))
	    {
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", s2);
		change_error(cp, scp, i18n("too many $filename"));
		sub_context_delete(scp);
		++number_of_errors;
	    }
	    else
		wl2.push_back(s2);
	}
	str_free(s2);
    }
    wl = wl2;

    //
    // You may not delete the last project configuration file.
    //
    // The actual filename given in the error message is the first one
    // seen, not the complete list of such files seen.
    //
    if (config_seen)
    {
	if (config_seen >= count_config_files(cp))
	{
	    sub_context_ty	*scp;

	    scp = sub_context_new();
	    sub_var_set_string(scp, "File_Name", configuration_filename);
	    change_error(cp, scp, i18n("may not remove $filename"));
	    sub_context_delete(scp);
	    ++number_of_errors;
	}
    }

    //
    // ensure that each file
    // 1. is not already part of the change
    // 2. is in the baseline
    // add it to the change
    //
    for (j = 0; j < wl.nstrings; ++j)
    {
	s1 = wl.string[j];
	if (change_file_find(cp, s1, view_path_first))
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
	    p_src_data = pp->file_find_fuzzy(s1, view_path_extreme);
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
	c_src_data = cp->file_new(p_src_data);
	c_src_data->action = file_action_remove;

	//
	// p_src_data->edit_number
	//	The head revision of the branch.
	// p_src_data->edit_number_origin
	//	The version originally copied.
	//
	// c_src_data->edit_number
	//	Not meaningful until after integrate pass.
	// c_src_data->edit_number_origin
	//	The version originally copied.
	// c_src_data->edit_number_origin_new
	//	Updates branch edit_number_origin on
	//	integrate pass.
	//
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
	// NOTREACHED
	sub_context_delete(scp);
    }

    //
    // Create each file in the development directory,
    // and make it be gibberish, to generate syntax errors if it is used.
    // Create any necessary directories along the way.
    //
    for (j = 0; j < wl.nstrings; ++j)
    {
	//
	// Find the mod-time of the file in the project
	//
	s1 = wl.string[j];

	//
	// Remove any existing file (this cleans up junk, and
	// breaks the link if we are using symlink trees).
	//
	change_file_whiteout_write(cp, s1, up);
    }

    //
    // the number of files changed, or the version did,
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

    // remember we are about to
    bool recent_integration = cp->run_project_file_command_needed();
    if (recent_integration)
        cp->run_project_file_command_done();

    //
    // write the data and release the lock
    //
    change_cstate_write(cp);
    commit();
    lock_release();

    //
    // run the change file command
    // and the project file command if necessary
    //
    cp->run_remove_file_command(&wl, up);
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
	change_verbose(cp, scp, i18n("remove file $filename complete"));
	sub_context_delete(scp);
    }
    change_free(cp);
    project_free(pp);
    trace(("}\n"));
}


//
// NAME
//	remove_file
//
// SYNOPSIS
//	void remove_file(void);
//
// DESCRIPTION
//	The remove_file function is used to
//	dispatch the 'aegis -ReMove_file' command to the relevant
//	function to do it's work.
//

void
remove_file(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
	{ arglex_token_help, remove_file_help, 0 },
	{ arglex_token_list, remove_file_list, 0 },
    };

    trace(("remove_file()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), remove_file_main);
    trace(("}\n"));
}

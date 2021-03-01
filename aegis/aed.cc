//
//	aegis - project change supervisor
//	Copyright (C) 1991-1999, 2001-2008 Peter Miller
//	Copyright (C) 2006, 2007 Walter Franzini
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
#include <common/ac/time.h>
#include <common/ac/unistd.h>

#include <common/error.h>
#include <common/progname.h>
#include <common/quit.h>
#include <common/str_list.h>
#include <common/trace.h>
#include <libaegis/ael/change/files.h>
#include <libaegis/arglex/change.h>
#include <libaegis/arglex/project.h>
#include <libaegis/arglex2.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>
#include <libaegis/change/identifier.h>
#include <libaegis/col.h>
#include <libaegis/commit.h>
#include <libaegis/help.h>
#include <libaegis/lock.h>
#include <libaegis/log.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/project/file.h>
#include <libaegis/sub.h>
#include <libaegis/undo.h>
#include <libaegis/user.h>

#include <aegis/aed.h>


#define NOT_SET (-1)


static void
difference_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf
    (
	stderr,
	"usage: %s -DIFFerence [ <option>... ][ <filename>... ]\n",
	progname
    );
    fprintf(stderr, "       %s -DIFFerence -List [ <option>... ]\n", progname);
    fprintf(stderr, "       %s -DIFFerence -Help\n", progname);
    quit(1);
}


static void
difference_help(void)
{
    help("aed", difference_usage);
}


static void
difference_list(void)
{
    trace(("difference_list()\n{\n"));
    arglex();
    change_identifier cid;
    cid.command_line_parse_rest(difference_usage);
    list_change_files(cid, 0);
    trace(("}\n"));
}


static void
anticipate(string_ty *project_name, long change_number, const char *branch,
    long cn2, log_style_ty log_style, string_list_ty *wl)
{
    cstate_ty       *cstate_data;
    cstate_ty       *cstate2_data;
    size_t          j;
    project_ty      *pp;
    change::pointer cp;
    user_ty::pointer up;
    change::pointer acp;
    project_ty      *pp2;

    trace(("anticipate()\n{\n"));

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
    // locate the other branch
    //
    if (branch)
	pp2 = pp->find_branch(branch);
    else
	pp2 = project_copy(pp);

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
    acp = change_alloc(pp2, cn2);
    change_bind_existing(acp);

    //
    // It is an error if the change is not in the being_developed state.
    // It is an error if the change is not assigned to the current user.
    // It is an error if the anticipated change is not awaiting
    // integration or being integrated.
    //
    cstate_data = cp->cstate_get();
    cstate2_data = acp->cstate_get();
    if
    (
	change_is_a_branch(cp)
    &&
	cstate_data->state != cstate_state_being_integrated
    )
	change_fatal(cp, 0, i18n("bad branch diff"));
    else if (cstate_data->state != cstate_state_being_developed)
	change_fatal(cp, 0, i18n("bad diff state"));
    if
    (
	!change_is_a_branch(acp)
    &&
	cstate2_data->state != cstate_state_awaiting_review
    &&
	cstate2_data->state != cstate_state_being_reviewed
    &&
	cstate2_data->state != cstate_state_awaiting_integration
    &&
	cstate2_data->state != cstate_state_being_integrated
    )
    {
	change_fatal(acp, 0, i18n("bad anticipate diff"));
    }
    if (nstring(change_developer_name(cp)) != up->name())
	change_fatal(cp, 0, i18n("not developer"));

    //
    // Make sure that we terminate elegantly if no diff is required for
    // this project.
    //
    if (!change_diff_required(cp))
    {
	change_verbose(cp, 0, i18n("no diff required"));
	quit(0);
	// NOTREACHED
    }

    //
    // If no files were named on the command line,
    // then diff every file the two changes have in common.
    // Ignore all but the most obvious of combinations.
    //
    if (!wl->nstrings)
    {
	for (j = 0;; ++j)
	{
	    fstate_src_ty   *src1_data;
	    fstate_src_ty   *src2_data;

	    src1_data = change_file_nth(cp, j, view_path_first);
	    if (!src1_data)
		break;
	    switch (src1_data->action)
	    {
	    case file_action_create:
	    case file_action_modify:
		break;

	    case file_action_remove:
	    case file_action_insulate:
	    case file_action_transparent:
#ifndef DEBUG
	    default:
#endif
		continue;
	    }
	    src2_data =
		change_file_find(acp, src1_data->file_name, view_path_first);
	    if (!src2_data)
		continue;
	    switch (src2_data->action)
	    {
	    case file_action_create:
	    case file_action_modify:
		break;

	    case file_action_remove:
	    case file_action_insulate:
	    case file_action_transparent:
#ifndef DEBUG
	    default:
#endif
		continue;
	    }
	    wl->push_back(src1_data->file_name);
	}
	if (!wl->nstrings)
	    change_fatal(cp, 0, i18n("no suitable files in common"));
    }
    else
    {
	string_list_ty  search_path;
	string_list_ty  search_path2;
	size_t          k;
	int             number_of_errors;

	//
	// resolve the path of each file
	// 1.   the absolute path of the file name is obtained
	// 2.   if the file is inside the development directory, ok
	// 3.   if the file is inside the baseline, ok
	// 4.   if neither, error
	//
	number_of_errors = 0;
	change_search_path_get(cp, &search_path, 1);
	change_search_path_get(acp, &search_path2, 1);
	search_path.push_back_unique(search_path2);
	string_list_ty wl2;
	for (j = 0; j < wl->nstrings; ++j)
	{
	    string_ty       *s1;
	    string_ty       *s2;

	    s1 = wl->string[j];
	    assert(s1->str_text[0] == '/');
	    s2 = 0;
	    for (k = 0; k < search_path.nstrings; ++k)
	    {
		s2 = os_below_dir(search_path.string[k], s1);
		if (s2)
		    break;
	    }
	    if (!s2)
	    {
		sub_context_ty *scp;

		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", s1);
		change_error(cp, scp, i18n("$filename unrelated"));
		sub_context_delete(scp);
		++number_of_errors;
	    }
	    else
		wl2.push_back_unique(s1);
	}
	*wl = wl2;

	//
	// confirm that each file is in both changes
	// and is sensable to compare
	//
	for (j = 0; j < wl->nstrings; ++j)
	{
	    fstate_src_ty   *src1_data;
	    fstate_src_ty   *src2_data;
	    string_ty       *s1;

	    s1 = wl->string[j];
	    src1_data = change_file_find(cp, s1, view_path_first);
	    if (!src1_data)
	    {
		sub_context_ty  *scp;

		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", s1);
		change_error(cp, scp, i18n("no $filename"));
		sub_context_delete(scp);
		++number_of_errors;
		continue;
	    }
	    switch (src1_data->action)
	    {
		sub_context_ty  *scp;

	    case file_action_create:
	    case file_action_modify:
		break;

	    case file_action_remove:
	    case file_action_insulate:
	    case file_action_transparent:
#ifndef DEBUG
	    default:
#endif
		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", s1);
		change_error(cp, scp, i18n("bad cp undo $filename"));
		sub_context_delete(scp);
		++number_of_errors;
		continue;
	    }
	    src2_data = change_file_find(acp, s1, view_path_first);
	    if (!src2_data)
	    {
		sub_context_ty  *scp;

		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", s1);
		change_error(acp, scp, i18n("no $filename"));
		sub_context_delete(scp);
		++number_of_errors;
		continue;
	    }
	    switch (src2_data->action)
	    {
		sub_context_ty  *scp;

	    case file_action_create:
	    case file_action_modify:
		break;

	    case file_action_remove:
	    case file_action_insulate:
	    case file_action_transparent:
#ifndef DEBUG
	    default:
#endif
		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", s1);
		change_error(acp, scp, i18n("bad cp undo $filename"));
		sub_context_delete(scp);
		++number_of_errors;
		continue;
	    }
	}

	//
	// abort now if there were errors
	//
	if (number_of_errors > 0)
	{
	    sub_context_ty sc;
	    sc.var_set_long("Number", number_of_errors);
	    sc.var_optional("Number");
	    change_fatal(cp, &sc, i18n("diff fail"));
	    // NOTREACHED
	}
    }

    //
    // diff each file
    //
    log_open(change_logfile_get(cp), up, log_style);
    os_throttle();
    for (j = 0; j < wl->nstrings; ++j)
    {
	fstate_src_ty   *src1_data;
	fstate_src_ty   *src2_data;
	string_ty       *original;
	string_ty       *most_recent;
	string_ty       *inp;
	string_ty       *output_filename;
	int             original_unlink;
	string_ty       *s1;

	s1 = wl->string[j];
	src1_data = change_file_find(cp, s1, view_path_first);
	assert(src1_data);
	src2_data = change_file_find(acp, s1, view_path_first);
	assert(src2_data);

	most_recent = change_file_path(acp, src1_data);
	assert(most_recent);
	if (change_has_merge_command(cp))
	{
	    output_filename = change_file_path(cp, src1_data);
	    assert(output_filename);
	    inp = str_format("%s,B", output_filename->str_text);
            user_ty::become scoped(up);
	    os_rename(output_filename, inp);
            undo_rename(inp, output_filename);
        }
	else
	{
	    inp = change_file_path(cp, src1_data);
	    assert(inp);
	    output_filename = str_format("%s,D", inp->str_text);
	}
	if (change_file_up_to_date(pp2, src1_data))
	{
	    //
	    // if they are both based on the same version,
	    // common ancestor is the baseline
	    //
	    trace(("project file path %s\n", s1->str_text));
	    original = project_file_path(pp, src1_data);
	    assert(original);
	    original_unlink = 0;
	}
	else
	{
	    //
	    // If they are based on different versions,
	    // need to get the original out of history
	    //
	    original = os_edit_filename(0);
	    original_unlink = 1;
	    up->become_begin();
	    undo_unlink_errok(original);
	    up->become_end();

	    assert(src1_data->edit);
	    assert(src1_data->edit->revision);
	    assert(src1_data->edit_origin);
	    assert(src1_data->edit_origin->revision);
	    change_run_history_get_command(cp, src1_data, original, up);
	}

	//
	// merge as appropriate
	//
	if (change_has_merge_command(cp))
	{
	    change_run_merge_command
	    (
		cp,
		up,
		original,
		most_recent,
		inp,
		output_filename
	    );
            user_ty::become scoped(up);
            undo_rename_cancel(inp, output_filename);
	}
	else
	{
	    change_run_diff3_command
	    (
		cp,
		up,
		original,
		most_recent,
		inp,
		output_filename
	    );
	}

	//
	// remove the temporary file
	//
	if (original_unlink)
	{
            user_ty::become scoped(up);
	    os_unlink(original);
	}

	//
	// make sure the diff is out-of-date,
	// to force a non-anticipate diff later
	//
	if (src1_data->diff_file_fp)
	{
	    fingerprint_type.free(src1_data->diff_file_fp);
	    src1_data->diff_file_fp = 0;
	}

	str_free(original);
	str_free(most_recent);
	str_free(inp);
	str_free(output_filename);
    }

    //
    // verbose success message
    //
    change_verbose(cp, 0, i18n("anticipated merge complete"));
    change_free(cp);
    change_free(acp);
    project_free(pp);
    trace(("}\n"));
}


static int
project_file_exists(project_ty *pp, string_ty *filename)
{
    fstate_src_ty   *p_src_data;

    p_src_data = project_file_find(pp, filename, view_path_extreme);
    return !!p_src_data;
}


static int
change_file_exists(change::pointer cp, string_ty *filename)
{
    fstate_src_ty   *c_src_data;

    c_src_data = change_file_find(cp, filename, view_path_first);
    return
	(
	    c_src_data
	&&
    	    !c_src_data->deleted_by
	&&
	    !c_src_data->about_to_be_created_by
	);
}


static void
difference_main(void)
{
    string_ty       *dd;
    string_ty       *s1 = 0;
    string_ty       *s2;
    cstate_ty       *cstate_data;
    size_t          j;
    size_t          k;
    string_ty       *project_name;
    project_ty      *pp;
    long            change_number;
    change::pointer cp;
    log_style_ty    log_style;
    user_ty::pointer up;
    long            cn2;
    int             merge_select;
    size_t          mergable_files;
    int             integrating;
    user_ty::pointer diff_user_p;
    const char      *branch;
    project_ty      *pp2;
    project_ty      *pp2bl;
    int             trunk;
    int             grandparent;

    trace(("difference_main()\n{\n"));
    arglex();
    string_list_ty wl;
    string_list_ty need_new_build;
    project_name = 0;
    change_number = 0;
    log_style = log_style_snuggle_default;
    cn2 = 0;
    merge_select = NOT_SET;
    branch = 0;
    trunk = 0;
    grandparent = 0;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(difference_usage);
	    continue;

	case arglex_token_directory:
	    if (arglex() != arglex_token_string)
		option_needs_dir(arglex_token_directory, difference_usage);
	    goto get_file_names;


	case arglex_token_file:
	    if (arglex() != arglex_token_string)
		option_needs_files(arglex_token_file, difference_usage);
	    // fall through...

	case arglex_token_string:
	    get_file_names:
	    s1 = str_from_c(arglex_value.alv_string);
	    os_become_orig();
	    s2 = os_pathname(s1, 1);
	    os_become_undo();
	    if (wl.member(s2))
	    {
		sub_context_ty sc;
		sc.var_set_string("File_Name", s1);
		sc.fatal_intl(i18n("too many $filename"));
		// NOTREACHED
	    }
	    wl.push_back(s2);
	    str_free(s1);
	    str_free(s2);
	    break;

	case arglex_token_change:
	    arglex();
	    // fall through...

	case arglex_token_number:
	    arglex_parse_change_with_branch
	    (
		&project_name,
		&change_number,
		&branch,
		difference_usage
	    );
	    continue;

	case arglex_token_project:
	    arglex();
	    arglex_parse_project(&project_name, difference_usage);
	    continue;

	case arglex_token_nolog:
	    if (log_style == log_style_none)
		duplicate_option(difference_usage);
	    log_style = log_style_none;
	    break;

	case arglex_token_anticipate:
	    if (cn2)
		duplicate_option(difference_usage);
	    if (arglex() != arglex_token_number)
		option_needs_number(arglex_token_anticipate, difference_usage);
	    cn2 = arglex_value.alv_number;
	    if (cn2 == 0)
		cn2 = MAGIC_ZERO;
	    else if (cn2 < 1)
	    {
		sub_context_ty sc;
		sc.var_set_long("Number", cn2);
		sc.fatal_intl(i18n("change $number out of range"));
		// NOTREACHED
	    }
	    break;

	case arglex_token_merge_not:
	    if (merge_select == uconf_diff_preference_no_merge)
		duplicate_option(difference_usage);
	    if (merge_select != NOT_SET)
	    {
	        too_many_merges:
		mutually_exclusive_options3
		(
		    arglex_token_merge_not,
		    arglex_token_merge_only,
		    arglex_token_merge_automatic,
		    difference_usage
		);
	    }
	    merge_select = uconf_diff_preference_no_merge;
	    break;

	case arglex_token_merge_only:
	    if (merge_select == uconf_diff_preference_only_merge)
		duplicate_option(difference_usage);
	    if (merge_select != NOT_SET)
		goto too_many_merges;
	    merge_select = uconf_diff_preference_only_merge;
	    break;

	case arglex_token_merge_automatic:
	    if (merge_select == uconf_diff_preference_automatic_merge)
		duplicate_option(difference_usage);
	    if (merge_select != NOT_SET)
		goto too_many_merges;
	    merge_select = uconf_diff_preference_automatic_merge;
	    break;

	case arglex_token_branch:
	    if (branch)
		duplicate_option(difference_usage);
	    switch (arglex())
	    {
	    default:
		option_needs_number(arglex_token_branch, difference_usage);

	    case arglex_token_number:
	    case arglex_token_string:
		branch = arglex_value.alv_string;
		break;

	    case arglex_token_stdio:
		branch = "";
		break;
	    }
	    break;

	case arglex_token_trunk:
	    if (trunk)
		duplicate_option(difference_usage);
	    ++trunk;
	    break;

	case arglex_token_grandparent:
	    if (grandparent)
		duplicate_option(difference_usage);
	    ++grandparent;
	    break;

	case arglex_token_wait:
	case arglex_token_wait_not:
	    user_ty::lock_wait_argument(difference_usage);
	    break;

	case arglex_token_base_relative:
	case arglex_token_current_relative:
	    user_ty::relative_filename_preference_argument(difference_usage);
	    break;
	}
	arglex();
    }
    if (grandparent)
    {
	if (branch)
	{
	    mutually_exclusive_options
	    (
		arglex_token_branch,
		arglex_token_grandparent,
		difference_usage
	    );
	}
	if (trunk)
	{
	    mutually_exclusive_options
	    (
		arglex_token_trunk,
		arglex_token_grandparent,
		difference_usage
	    );
	}
	branch = "..";
    }
    if (trunk)
    {
	if (branch)
	{
	    mutually_exclusive_options
	    (
		arglex_token_branch,
		arglex_token_trunk,
		difference_usage
	    );
	}
	branch = "";
    }

    if (cn2)
    {
	if (merge_select != NOT_SET)
	    fatal_intl(0, i18n("no merge anticipate"));
	anticipate(project_name, change_number, branch, cn2, log_style, &wl);
	trace(("}\n"));
	return;
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
    // locate which branch
    //
    if (branch)
	pp2 = pp->find_branch(branch);
    else
	pp2 = project_copy(pp);

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
    // lock on the appropriate row of the change table.
    //
    // Also take a read lock on the baseline, to ensure that it does
    // not change (aeip) for the duration of the diff/merge.
    //
    change_cstate_lock_prepare(cp);
    project_baseline_read_lock_prepare(pp);
    lock_take();

    //
    // It is an error if the change is not in the being_developed or
    // being_integrated state.
    // It is an error if the change is not assigned to the current user.
    //
    cstate_data = cp->cstate_get();
    if
    (
	change_is_a_branch(cp)
    &&
	cstate_data->state != cstate_state_being_integrated
    )
	change_fatal(cp, 0, i18n("bad branch diff"));
    if
    (
	cstate_data->state != cstate_state_being_developed
    &&
	cstate_data->state != cstate_state_being_integrated
    )
	change_fatal(cp, 0, i18n("bad idiff state"));
    integrating = (cstate_data->state == cstate_state_being_integrated);
    if (integrating)
    {
	if (nstring(change_integrator_name(cp)) != up->name())
	    change_fatal(cp, 0, i18n("not integrator"));
    }
    else
    {
	if (nstring(change_developer_name(cp)) != up->name())
	    change_fatal(cp, 0, i18n("not developer"));

	//
	// Look at the change files and make sure that a recent integration
	// hasn't invalidated them in some way.
	//
	trace(("mark\n"));
	if (change_file_promote(cp))
	{
	    // remember that we are about to
            bool recent_integration = cp->run_project_file_command_needed();
            if (recent_integration)
                cp->run_project_file_command_done();

            //
            // Write out the file state, and then let go of the locks
            // and take them again.  This ensures the data is consistent
            // for the next stage of processing.
	    //
	    trace(("Write out what we've done so far.\n"));
	    change_cstate_write(cp);
	    commit();
	    lock_release();

            // always done outside locks
            if (recent_integration)
                cp->run_project_file_command(up);

	    trace(("Take the locks again.\n"));
	    change_cstate_lock_prepare(cp);
	    project_baseline_read_lock_prepare(pp);
	    lock_take();
	}
    }

    //
    // the branch option implies a merge
    //
    if (branch)
    {
	if (merge_select == uconf_diff_preference_no_merge)
	{
	    mutually_exclusive_options
	    (
		arglex_token_branch,
		arglex_token_merge_not,
		difference_usage
	    );
	}
	if (merge_select == uconf_diff_preference_automatic_merge)
	{
	    mutually_exclusive_options
	    (
		arglex_token_branch,
		arglex_token_merge_automatic,
		difference_usage
	    );
	}
	if (integrating)
	    change_fatal(cp, 0, i18n("no cross branch int merge"));
	merge_select = uconf_diff_preference_only_merge;
    }

    //
    // Now that we know the change state, we can set the merge
    // preference.  It is an error to request a merge in the 'being
    // integrated' state.
    //
    if (integrating)
    {
	if (merge_select == uconf_diff_preference_only_merge)
	    change_fatal(cp, 0, i18n("no int merge"));
	merge_select = uconf_diff_preference_no_merge;
    }
    else
    {
	if (merge_select == NOT_SET)
	    merge_select = up->diff_preference();
    }

    //
    // if no files were named on the command line,
    // then diff every file in the change.
    //
    if (!wl.nstrings)
    {
	if (!change_file_nth(cp, 0, view_path_first))
	    change_fatal(cp, 0, i18n("no files"));
	for (j = 0;; ++j)
	{
	    fstate_src_ty   *src_data;

	    src_data = change_file_nth(cp, j, view_path_first);
	    if (!src_data)
		break;
	    wl.push_back(src_data->file_name);
	}
    }
    else
    {
	string_list_ty  search_path;
	int             number_of_errors;

	//
	// When the change is being integrated, you must
	// difference everything.  You may not name files on the
	// command line.
	//
	number_of_errors = 0;
	if (integrating)
	{
	    change_error(cp, 0, i18n("int must diff all"));
	    ++number_of_errors;
	}

	//
	// FIXME: this really needs to be coded to understand
	// the -baserel option.
	//

	//
	// resolve the path of each file
	// 1.   the absolute path of the file name is obtained
	// 2.   if the file is inside the development directory, ok
	// 3.   if the file is inside the baseline, ok
	// 4.   if neither, error
	//
	change_search_path_get(cp, &search_path, 1);
	string_list_ty wl2;
	for (j = 0; j < wl.nstrings; ++j)
	{
	    s1 = wl.string[j];
	    assert(s1->str_text[0] == '/');
	    s2 = 0;
	    for (k = 0; k < search_path.nstrings; ++k)
	    {
		s2 = os_below_dir(search_path.string[k], s1);
		if (s2)
		    break;
	    }
	    if (!s2)
	    {
		sub_context_ty  *scp;

		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", s1);
		change_error(cp, scp, i18n("$filename unrelated"));
		sub_context_delete(scp);
		++number_of_errors;
	    }
	    else
	    {
		wl2.push_back_unique(s2);
		str_free(s2);
	    }
	}
	wl = wl2;

	//
	// confirm that each file is in the change
	// and resolve directories into files
	//
	wl2.clear();
	for (j = 0; j < wl.nstrings; ++j)
	{
	    string_list_ty  wl_in;

	    s1 = wl.string[j];
	    change_file_directory_query(cp, s1, &wl_in, 0);
	    if (wl_in.nstrings)
		wl2.push_back_unique(wl_in);
	    else if (change_file_find(cp, s1, view_path_first))
		wl2.push_back_unique(s1);
	    else
	    {
		sub_context_ty  *scp;

		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", s1);
		change_error(cp, scp, i18n("no $filename"));
		sub_context_delete(scp);
		++number_of_errors;
	    }
	}
	wl = wl2;

	//
	// abort now if there were errors
	//
	if (number_of_errors > 0)
	{
	    sub_context_ty sc;
	    sc.var_set_long("Number", number_of_errors);
	    sc.var_optional("Number");
	    change_fatal(cp, &sc, i18n("diff fail"));
	    // NOTREACHED
	}
    }

    //
    // look for files which need to be merged
    //
    mergable_files = 0;
    for (j = 0; j < wl.nstrings; ++j)
    {
	fstate_src_ty   *src1_data;
	fstate_src_ty   *src2_data;

	//
	// find the relevant change src data
	//
	s1 = wl.string[j];
	src1_data = change_file_find(cp, s1, view_path_first);
	assert(src1_data);

	//
	// generated files are not merged
	// created or deleted files are not merged
	//
	switch (src1_data->usage)
	{
	case file_usage_build:
	    // toss these ones
	    continue;

	case file_usage_source:
	case file_usage_config:
	case file_usage_test:
	case file_usage_manual_test:
	    // keep these ones
	    break;
	}
	switch (src1_data->action)
	{
	case file_action_create:
	    if (src1_data->move)
	    {
		// keep move destinations
		break;
	    }
	    // toss these ones
	    continue;

	case file_action_remove:
	    // toss these ones
	    continue;

	case file_action_modify:
	case file_action_insulate:
	case file_action_transparent:
	    // keep these ones
	    break;
	}

	//
	// find the relevant baseline src data
	// note that someone may have deleted it from under you
	//
	// If the edit numbers match (is up to date)
	// then do not merge this one.
	//
	src2_data = project_file_find(pp2, src1_data, view_path_extreme);
	if (!src2_data)
	    continue;
	if (change_file_up_to_date(pp2, src1_data))
	    continue;

	//
	// this one needs merging
	//
	++mergable_files;
    }

    //
    // figure what to do if the user (indirectly)
    // selected the automatic_merge option
    //
    // DO NOT merge files when the change is in the 'being
    // integrated' state, no matter what the user preference
    // indicates.  This was taken care of earlier.
    //
    if (merge_select == uconf_diff_preference_automatic_merge)
    {
	if (mergable_files)
	    merge_select = uconf_diff_preference_only_merge;
	else
	    merge_select = uconf_diff_preference_no_merge;
    }

    //
    // Differences in the 'being integrated' state are done
    // by the project owner, not the integrator.  (There is
    // a known memory leak: the project user is not free()ed
    // at the end of the loop.)
    //
    pp2bl = pp2;
    if (integrating)
    {
	diff_user_p = project_user(pp);
	if (log_style == log_style_snuggle_default)
	    log_style = log_style_append_default;

	//
	// when integrating, the baseline is one project deeper
	//
	if (!pp2->is_a_trunk())
	    pp2bl = pp2->parent_get();
    }
    else
	diff_user_p = up;

    log_open(change_logfile_get(cp), diff_user_p, log_style);
    if (integrating)
	dd = change_integration_directory_get(cp, 0);
    else
	dd = change_development_directory_get(cp, 0);
    os_throttle();

    if (merge_select == uconf_diff_preference_only_merge)
    {
	//
	// merge each file
	//
	trace(("merging...\n"));
	for (j = 0; j < wl.nstrings; ++j)
	{
	    fstate_src_ty   *src1_data;
	    fstate_src_ty   *src2_data;
	    string_ty       *original;
	    string_ty       *curfile;
	    string_ty       *outname;
	    string_ty       *most_recent;
	    fstate_src_ty   *reconstruct;
	    fstate_src_ty   *p_src_data;

	    //
	    // find the relevant change src data
	    //
	    s1 = wl.string[j];
	    src1_data = change_file_find(cp, s1, view_path_first);
	    assert(src1_data);

	    trace
	    ((
		"change: %s %s \"%s\" %s %s\n",
		file_usage_ename(src1_data->usage),
		file_action_ename(src1_data->action),
		src1_data->file_name->str_text,
		(src1_data->edit_origin ?
		    src1_data->edit_origin->revision->str_text : ""),
		(src1_data->edit ?
		    src1_data->edit->revision->str_text : "")
	    ));

	    //
	    // generated files are not merged
	    // created or deleted files are not merged
	    //
	    // Note: Insulation files are merged! This is
	    // important if they have been silly enough to
	    // change them.
	    //
	    switch (src1_data->usage)
	    {
	    case file_usage_build:
		// toss these ones
		continue;

	    case file_usage_source:
	    case file_usage_config:
	    case file_usage_test:
	    case file_usage_manual_test:
		// keep these ones
		break;
	    }
	    switch (src1_data->action)
	    {
	    case file_action_create:
		if (src1_data->move)
		{
		    // keep move destinations
		    break;
		}
		// toss these ones
		continue;

	    case file_action_remove:
		// toss these ones
		continue;

	    case file_action_modify:
	    case file_action_insulate:
	    case file_action_transparent:
		// keep these ones
		break;
	    }

	    //
	    // find the relevant baseline src data
	    // note that someone may have deleted it from under you
	    //
	    // If the edit numbers match (is up to date)
	    // then do not merge this one.
	    //
	    src2_data = project_file_find(pp2, src1_data, view_path_extreme);
	    if (!src2_data)
	    {
		//
		// If we are doing a cross branch merge, and this file
		// isn't in the other branch, treat it like a new file
		// and ignore it.
		//
		if (pp != pp2)
		    continue;

		sub_context_ty sc;
		sc.var_set_string("File_Name", s1);
		change_fatal(cp, &sc, i18n("no $filename in baseline"));
		// NOTREACHED
	    }
	    if (change_file_up_to_date(pp2, src1_data))
	    {
		trace
		((
		    "%s %s \"%s\" %s %s: up to date\n",
	    	    file_usage_ename(src1_data->usage),
		    file_action_ename(src1_data->action),
		    src1_data->file_name->str_text,
		    (src1_data->edit_origin ?
			src1_data->edit_origin->revision->str_text : ""),
		    (src1_data->edit ? src1_data->edit->revision->str_text : "")
		));
		continue;
	    }

	    //
	    // build various paths
	    //
	    if (change_has_merge_command(cp))
	    {
		outname = change_file_path(cp, src1_data);
		trace_string(outname->str_text);
		curfile = str_format("%s,B", outname->str_text);
		trace_string(curfile->str_text);
                user_ty::become scoped(diff_user_p);
		os_rename(outname, curfile);
                undo_rename(curfile, outname);
	    }
	    else
	    {
		curfile = change_file_path(cp, src1_data);
		trace_string(curfile->str_text);
		outname = str_format("%s,D", curfile->str_text);
		trace_string(outname->str_text);
	    }

	    //
	    // name for temp file
	    //
	    original = os_edit_filename(0);
	    diff_user_p->become_begin();
	    undo_unlink_errok(original);
	    diff_user_p->become_end();

	    //
	    // get the version out of history
	    //
	    // For cross branch merges, use the project
	    // file's edit origin, rather then the
	    // change file's edit origin.
	    //
	    assert(src1_data->edit_origin);
	    assert(src1_data->edit_origin->revision);
	    reconstruct = (fstate_src_ty *)fstate_src_type.alloc();
	    reconstruct->file_name = str_copy(s1);
	    reconstruct->edit = history_version_copy(src1_data->edit_origin);
	    change_file_copy_basic_attributes(reconstruct, src1_data);

	    p_src_data =
		project_file_find(cp->pp, src1_data, view_path_extreme);
	    if (!p_src_data)
	    {
		//
		// The project file we were renamed from has been removed.
		// Treat the change file as if it was up-to-date.
		//
		history_version_type.free(reconstruct);
		continue;
	    }
	    trace
	    ((
		"project: %s %s \"%s\" %s %s\n",
		file_usage_ename(p_src_data->usage),
		file_action_ename(p_src_data->action),
		p_src_data->file_name->str_text,
		(p_src_data->edit_origin ?
		    p_src_data->edit_origin->revision->str_text : ""),
		(p_src_data->edit ?
		    p_src_data->edit->revision->str_text : "")
	    ));
	    if (cp->pp != pp2)
	    {
		assert
		(
		    !p_src_data->edit_origin
		||
		    p_src_data->edit_origin->revision
		);
		if (p_src_data && p_src_data->edit_origin)
		{
		    history_version_type.free(reconstruct->edit);
		    reconstruct->edit =
			history_version_copy(p_src_data->edit_origin);
		}
	    }
	    assert(reconstruct);
	    assert(reconstruct->edit);
	    assert(reconstruct->edit->revision);
	    change_run_history_get_command
	    (
		cp,
		reconstruct,
		original,
		diff_user_p
	    );
	    history_version_type.free(reconstruct);

	    //
	    // use the appropriate merge command
	    //
	    trace(("project file path %s\n", p_src_data->file_name->str_text));
	    most_recent = project_file_path(pp2, p_src_data);
	    assert(most_recent);
	    if (change_has_merge_command(cp))
	    {
                change_run_merge_command
		(
		    cp,
		    diff_user_p,
		    original,
		    most_recent,
		    curfile,
		    outname
		);
                user_ty::become scoped(diff_user_p);
                undo_rename_cancel(curfile, outname);
            }
	    else
	    {
		change_run_diff3_command
		(
		    cp,
		    diff_user_p,
		    original,
		    most_recent,
		    curfile,
		    outname
		);
	    }
	    str_free(most_recent);

	    //
	    // Remember to remove the temporary file when
	    // finished.
	    //
	    diff_user_p->become_begin();
	    os_unlink(original);
	    diff_user_p->become_end();
	    str_free(original);

	    //
	    // p_src_data->edit
	    //      The head revision of the branch.
	    // p_src_data->edit_origin
	    //      The version originally copied.
	    //
	    // c_src_data->edit
	    //      Not meaningful until after integrate pass.
	    // c_src_data->edit_origin
	    //      The version originally copied.
	    // c_src_data->edit_origin_new
	    //      Updates branch edit_origin on
	    //      integrate pass.
	    //
	    assert(src2_data->edit);
	    assert(src2_data->edit->revision);
	    if (cp->pp == pp2)
	    {
		// normal merge
		if (src1_data->edit_origin)
		{
		    history_version_type.free(src1_data->edit_origin);
		}
		src1_data->edit_origin = history_version_copy(src2_data->edit);
	    }
	    else
	    {
		// cross branch merge
		if (src1_data->edit_origin_new)
		{
		    history_version_type.free(src1_data->edit_origin_new);
		}
		src1_data->edit_origin_new =
		    history_version_copy(src2_data->edit);
	    }

	    //
	    // Invalidate the file time stamps.
	    //
	    if (src1_data->file_fp)
	    {
		fingerprint_type.free(src1_data->file_fp);
		src1_data->file_fp = 0;
	    }
	    if (src1_data->diff_file_fp)
	    {
		fingerprint_type.free(src1_data->diff_file_fp);
		src1_data->diff_file_fp = 0;
	    }

	    //
	    // Remember this file name, so that we can give
	    // a verbose success message at the end.
	    //
	    need_new_build.push_back(src1_data->file_name);

	    //
	    // Nuke the build times.
	    //
	    change_build_times_clear(cp);
            str_free(curfile);
	    str_free(outname);
	}

	//
	// write the data back
	//      (always do this, to ensure fstate is correct)
	//
	change_cstate_write(cp);
	commit();
	lock_release();

	for (j = 0; j < need_new_build.nstrings; ++j)
	{
	    sub_context_ty  *scp;

	    scp = sub_context_new();
	    sub_var_set_string(scp, "File_Name", need_new_build.string[j]);
	    if (change_has_merge_command(cp))
		change_error(cp, scp, i18n("$filename merged"));
	    else
		change_error(cp, scp, i18n("$filename diff3ed"));
	    sub_context_delete(scp);
	}

	if (!need_new_build.empty())
	{
	    sub_context_ty  *scp;

	    scp = sub_context_new();
	    sub_var_set_long(scp, "Number", (long)need_new_build.nstrings);
	    sub_var_optional(scp, "Number");
	    change_error(cp, scp, i18n("merge complete"));
	    sub_context_delete(scp);
	}
    }
    else
    {
	if (mergable_files)
	{
	    sub_context_ty  *scp;

	    scp = sub_context_new();
	    sub_var_set_long(scp, "Number", mergable_files);
	    sub_var_optional(scp, "Number");
	    change_verbose(cp, scp, i18n("warning: mergable files"));
	    sub_context_delete(scp);
	}

	//
        // Make sure that we terminate elegantly if no diff is required
        // for this project.
	//
	if (!change_diff_required(cp))
	{
	    change_verbose(cp, 0, i18n("no diff required"));
	    quit(0);
	    // NOTREACHED
	}

	//
	// diff each file
	//
	for (j = 0; j < wl.nstrings; ++j)
	{
	    fstate_src_ty   *src1_data;
	    fstate_src_ty   *src2_data;
	    string_ty       *path;
	    string_ty       *path_d;
	    int             ignore;

	    //
	    // find the relevant change src data
	    //
	    trace(("j = %ld\n", (long)j));
	    s1 = wl.string[j];
	    trace(("file name \"%s\"\n", s1->str_text));
	    src1_data = change_file_find(cp, s1, view_path_first);
	    if (!src1_data)
		this_is_a_bug();
	    trace(("src1_data = %08lX\n", (long)src1_data));
            trace(("file_name = \"%s\"\n", src1_data->file_name->str_text));
	    trace(("action = %s\n", file_action_ename(src1_data->action)));
	    trace(("usage = %s\n", file_usage_ename(src1_data->usage)));

            //
	    // locate the equivalent project file
	    //
	    src2_data = project_file_find(pp2bl, src1_data, view_path_extreme);
	    trace(("src2_data = %08lX\n", (long)src2_data));
	    if (src2_data)
	    {
		trace(("action2 = %s\n", file_action_ename(src2_data->action)));
		trace(("usage2 = %s\n", file_usage_ename(src2_data->usage)));
                trace(("file_name2 = \"%s\";\n",
                       src2_data->file_name->str_text));
	    }

            //
	    // generated files are not differenced
	    //
	    switch (src1_data->usage)
	    {
	    case file_usage_build:
                switch (src1_data->action)
                {
                case file_action_create:
                    if (src2_data)
                    {
                        src1_data->action = file_action_modify;
                        if (!src1_data->uuid && src2_data->uuid)
                            src1_data->uuid = str_copy(src2_data->uuid);
                        if (!str_equal(src1_data->uuid, src2_data->uuid))
                        {
                            // fixme: we come here if an UUID clash is
                            // present (the previous test failed).
                            // Must think a better way to handle it.
                            assert(0);
                        }
                    }
                    break;

                case file_action_modify:
                case file_action_insulate:
                case file_action_remove:
                case file_action_transparent:
                    break;
                }
		continue;

	    case file_usage_source:
	    case file_usage_config:
	    case file_usage_test:
	    case file_usage_manual_test:
		break;
	    }

	    switch (src1_data->action)
	    {
	    case file_action_create:
	    case file_action_modify:
	    case file_action_insulate:
		break;

	    case file_action_remove:
		//
		// the removed half of a move is not differenced
		//
		if
		(
		    src1_data->move
		&&
		    change_file_find(cp, src1_data->move, view_path_first)
		)
		    continue;
		break;

	    case file_action_transparent:
		//
		// Transparent files are not differenced when integrating.
		//
		if (integrating)
		    continue;
		break;
	    }

	    //
	    // build various paths
	    //
	    path = change_file_path(cp, src1_data);
	    assert(path);
	    trace(("change file path \"%s\"\n", path->str_text));
	    path_d = str_format("%s,D", path->str_text);
	    trace(("change file,D path \"%s\"\n", path_d->str_text));

	    //
	    // Check the file's fingerprint.  This will zap
	    // the other timestamps if the fingerprint has
	    // changed.
	    //
	    switch (src1_data->action)
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
		if (!integrating)
		{
		    change_file_fingerprint_check(cp, src1_data);
		    assert(src1_data->file_fp);
		    assert(src1_data->file_fp->youngest > 0);
		    assert(src1_data->file_fp->oldest > 0);
		}
		break;
	    }

	    //
	    // All change files have a difference file,
	    // eventually, even files being removed and
	    // created.  Do nothing if we can.
	    //
	    diff_user_p->become_begin();
	    if (integrating)
	    {
		ignore =
		    change_fingerprint_same
		    (
			src1_data->idiff_file_fp,
			path_d,
			0
		    );
	    }
	    else
	    {
		ignore =
		    change_fingerprint_same
		    (
			src1_data->diff_file_fp,
			path_d,
			0
		    );
	    }
	    diff_user_p->become_end();
	    if (ignore)
	    {
		trace(("ignore\n"));
		str_free(path);
		str_free(path_d);
		continue;
	    }

	    switch (src1_data->action)
	    {
	    case file_action_create:
		//
		// difference the file
		// from nothing
		//
	        pseudo_create:
		{
		    string_ty *original;
		    int org_unlink = 0;
		    if
		    (
			src1_data->move
		    &&
			project_file_exists(pp2bl, src1_data->move)
		    )
		    {
			trace(("project_file_path %s\n",
			    src1_data->move->str_text));
			original = project_file_path(pp2bl, src1_data->move);
			os_become_orig();
			int file_exists = os_exists(original);
			os_become_undo();
			assert(file_exists);
			if (!file_exists)
			{
			    original =
				project_file_version_path
				(
				    pp2bl,
				    src1_data,
				    &org_unlink
				);
			}
			assert(original);
		    }
		    else
			original = str_from_c("/dev/null");
		    change_run_diff_command
		    (
			cp,
			diff_user_p,
			original,
			path,
			path_d
		    );
		    if (org_unlink)
		    {
			assert(original);
			os_become_orig();
			os_unlink(original);
			os_become_undo();
		    }
		    str_free(original);
		}

		//
		// remember the new fingerprint
		//
		set_fingerprint:
		diff_user_p->become_begin();
		if (integrating)
		{
		    if (!src1_data->idiff_file_fp)
                    {
			src1_data->idiff_file_fp =
                            (fingerprint_ty *)fingerprint_type.alloc();
                    }
		    src1_data->idiff_file_fp->youngest = 0;
		    change_fingerprint_same
		    (
			src1_data->idiff_file_fp,
			path_d,
			0
		    );
		}
		else
		{
		    if (!src1_data->diff_file_fp)
                    {
			src1_data->diff_file_fp =
                            (fingerprint_ty *)fingerprint_type.alloc();
                    }
		    src1_data->diff_file_fp->youngest = 0;
		    change_fingerprint_same(src1_data->diff_file_fp, path_d, 0);
		}
		diff_user_p->become_end();
		break;

	    case file_action_remove:
		//
		// create directory for diff file
		//
		diff_user_p->become_begin();
		os_mkdir_between(dd, s1, 02755);
		diff_user_p->become_end();

		//
		// difference the file
		// to nothing
		//
		{
		    string_ty *original;
		    string_ty *input_file_name;
		    int org_unlink = 0;
		    if (src2_data)
		    {
			trace(("project file path %s\n", s1->str_text));
                        //
                        // Use src2_data to handle correctly the case
                        // of a renamed file.  The UUID, if any, is
                        // the same and the file name is the right one
                        // (src1_data->move).
                        //
                        // If the file is simply removed everything is
                        // ok (same name and same UUID).
                        //
                        //
			original = project_file_path(pp2bl, src2_data);

			//
			// The following code is needed to make it
			// possible for aegis to cope with broken
			// baseline.
			//
			os_become_orig();
			int file_exists = os_exists(original);
			os_become_undo();
			assert(file_exists);
			if (!file_exists)
			{
			    original =
				project_file_version_path
				(
				    pp2bl,
				    src2_data,
				    &org_unlink
				);
			}
		    }
		    else
			original = str_from_c("/dev/null");
		    assert(original);
		    if
		    (
			src1_data->move
		    &&
			change_file_exists(cp, src1_data->move)
		    )
		    {
                        //
                        // Use src2_data to handle correctly the case
                        // of a renamed file.  The UUID, if any, is
                        // the same and the file name is the right one
                        // (src1_data->move).
                        //
                        // If the file is simply removed everything is
                        // ok (same name and same UUID).
                        //
                        assert(src2_data);
			input_file_name = change_file_path(cp, src2_data);
			assert(input_file_name);
		    }
		    else
			input_file_name = str_from_c("/dev/null");
		    change_run_diff_command
		    (
			cp,
			diff_user_p,
			original,
			input_file_name,
			path_d
		    );
		    if (org_unlink)
		    {
			assert(original);
			os_become_orig();
			os_unlink(original);
			os_become_undo();
		    }
		    str_free(original);
		    str_free(input_file_name);
		}
		goto set_fingerprint;

	    case file_action_insulate:
		//
		// At integration time, ignore read only entries.
		//
		assert(!integrating);
		if (integrating)
		    break;
		// Fall through...

	    case file_action_transparent:
		// Fall through...

	    case file_action_modify:
		trace(("%s\n", file_action_ename(src1_data->action)));
                trace(("src2_data = %08lX\n", (long)src2_data));
		if (!src2_data)
		{
                    integrate_pseudo_create:
                    assert(integrating);

		    //
                    // If a file is created on the trunk, then is is
                    // possible that the project file pointer is NULL
                    // here, because we are trying to difference it
                    // against the trunk's non-existent parent branch,
                    // which is completely empty.
                    //
                    // Something similar can happen for deeper branches,
                    // too, because we used the view_path_extreme
                    // option.
		    //
		    goto pseudo_create;
		}
		assert(src2_data);

		//
		// we did merges earlier,
		// should not be necessary here
		//
		assert(change_file_up_to_date(pp2, src1_data));

		//
		// use the diff-command
		//
		{
                    fstate_src_ty *pp2src_data =
                        project_file_find(pp2bl, s1, view_path_extreme);
                    if (!pp2src_data)
                    {
                        trace(("not in grandparent branch\n"));
                        goto integrate_pseudo_create;
                    }

		    trace(("project file path \"%s\"\n", s1->str_text));
		    int org_unlink = 0;
		    string_ty *original = project_file_path(pp2bl, src1_data);
		    trace_string(original->str_text);

		    //
		    // The following code is needed to make it
		    // possible for aegis to cope with broken
		    // baseline.
		    //
		    os_become_orig();
		    int file_exists = os_exists(original);
		    os_become_undo();
		    assert(file_exists);
		    if (!file_exists)
		    {
			original =
			    project_file_version_path
			    (
				pp2bl,
				src2_data,
				&org_unlink
			    );
			trace_string(original->str_text);
		    }

		    assert(original);
		    change_run_diff_command
		    (
			cp,
			diff_user_p,
			original,
			path,
			path_d
		    );
		    if (org_unlink)
		    {
			assert(original);
			os_become_orig();
			os_unlink(original);
			os_become_undo();
		    }
		    str_free(original);
		}
		goto set_fingerprint;
	    }
            str_free(path);
	    str_free(path_d);
	}

	//
	// If the change row (or change file table) changed,
	// write it out.
	// Release advisory lock.
	//
	change_cstate_write(cp);
	commit();
	lock_release();

	//
	// verbose success message
	//
	change_verbose(cp, 0, i18n("difference complete"));
    }
    change_free(cp);
    project_free(pp);
    trace(("}\n"));
}


void
difference(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
	{ arglex_token_help, difference_help, 0 },
	{ arglex_token_list, difference_list, 0 },
    };

    trace(("difference()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), difference_main);
    trace(("}\n"));
}

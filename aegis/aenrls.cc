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

#include <common/ac/ctype.h>
#include <common/ac/stdio.h>
#include <common/ac/string.h>
#include <common/ac/sys/types.h>
#include <common/ac/sys/stat.h>

#include <common/error.h>
#include <common/mem.h>
#include <common/progname.h>
#include <common/quit.h>
#include <common/trace.h>
#include <libaegis/ael/project/projects.h>
#include <libaegis/arglex2.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>
#include <libaegis/change/identifier.h>
#include <libaegis/commit.h>
#include <libaegis/dir.h>
#include <libaegis/file.h>
#include <libaegis/gonzo.h>
#include <libaegis/help.h>
#include <libaegis/lock.h>
#include <libaegis/log.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/project/file.h>
#include <libaegis/project/history.h>
#include <libaegis/sub.h>
#include <libaegis/undo.h>
#include <libaegis/user.h>

#include <aegis/aenrls.h>

#define GIVEN -1
#define NOT_GIVEN -2


static void
new_release_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf
    (
	stderr,
	"usage: %s -New_ReLeaSe <name> [ <option>... ]\n",
	progname
    );
    fprintf(stderr, "       %s -New_ReLeaSe -List [ <option>... ]\n", progname);
    fprintf(stderr, "       %s -New_ReLeaSe -Help\n", progname);
    quit(1);
}


static void
new_release_help(void)
{
    help("aenrls", new_release_usage);
}


static void
new_release_list(void)
{
    arglex();
    change_identifier cid;
    cid.command_line_parse_rest(new_release_usage);
    list_projects(cid, 0);
}


struct copy_tree_arg_ty
{
    string_ty	    *from;
    string_ty	    *to;
};


static void
copy_tree_callback(void *arg, dir_walk_message_ty message, string_ty *path,
    const struct stat *st)
{
    string_ty	    *s1;
    string_ty	    *s2;
    copy_tree_arg_ty *info;

    trace(("copy_tree_callback(arg = %08lX, message = %d, path = %08lX, "
	"st = %08lX)\n{\n", (long)arg, message, (long)path, (long)st));
    info = (copy_tree_arg_ty *)arg;
    trace_string(path->str_text);
    s1 = os_below_dir(info->from, path);
    assert(s1);
    trace_string(s1->str_text);
    if (!s1->str_length)
	s2 = str_copy(info->to);
    else
	s2 = os_path_join(info->to, s1);
    trace_string(s2->str_text);
    switch (message)
    {
    case dir_walk_dir_before:
	if (s1->str_length)
	{
	    os_mkdir(s2, st->st_mode & 07755);
	    undo_rmdir_errok(s2);
	}
	break;

    case dir_walk_file:
	//
	// copy the file
	//
	copy_whole_file(path, s2, 1);
	undo_unlink_errok(s2);
	os_chmod(s2, st->st_mode & 07755);
	break;

    case dir_walk_dir_after:
    case dir_walk_special:
    case dir_walk_symlink:
	break;
    }
    str_free(s2);
    str_free(s1);
    trace(("}\n"));
}


static void
new_release_main(void)
{
    sub_context_ty  *scp;
    string_ty	    *ip;
    string_ty	    *bl;
    string_ty	    *hp;
    size_t	    j;
    size_t	    k;
    pstate_ty	    *pstate_data[2];
    string_ty	    *s1;
    string_ty	    *project_name[2];
    int		    project_name_count;
    project_ty	    *pp[2];
    project_ty	    *ppp;
    change::pointer cp;
    cstate_history_ty *chp;
    cstate_ty       *cstate_data;
    copy_tree_arg_ty info;
    log_style_ty    log_style;
    user_ty::pointer up;
    user_ty::pointer pup;
    user_ty::pointer pup1;
    long	    new_version_number[10];
    int		    new_version_number_length;
    string_ty	    *new_version_string;
    project_ty	    *version_pp[SIZEOF(new_version_number)];
    long	    old_version_number[SIZEOF(new_version_number)];
    int		    old_version_number_length;
    long	    change_number;

    trace(("new_release_main()\n{\n"));
    arglex();
    log_style = log_style_create_default;
    nstring home;
    project_name_count = 0;
    new_version_number[0] = NOT_GIVEN;
    new_version_number[1] = NOT_GIVEN;
    new_version_number_length = 0;
    new_version_string = 0;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(new_release_usage);
	    continue;

	case arglex_token_project:
	    if (arglex() != arglex_token_string)
		option_needs_name(arglex_token_project, new_release_usage);
	    // fall through...

	case arglex_token_string:
	    if (project_name_count >= 2)
		fatal_intl(0, i18n("too many proj name"));
	    project_name[project_name_count++] =
		str_from_c(arglex_value.alv_string);
	    break;

	case arglex_token_directory:
	    if (arglex() != arglex_token_string)
		option_needs_dir(arglex_token_directory, new_release_usage);
	    if (home)
	    {
		duplicate_option_by_name
		(
		    arglex_token_directory,
		    new_release_usage
		);
	    }
	    os_become_orig();
	    home = os_pathname(arglex_value.alv_string, true);
	    os_become_undo();
	    break;

	case arglex_token_major:
	    if (new_version_number[0] != NOT_GIVEN)
		duplicate_option(new_release_usage);
	    scp = sub_context_new();
	    sub_var_set_charstar
	    (
		scp,
		"Name1",
		arglex_token_name(arglex_token_major)
	    );
	    sub_var_set_charstar
	    (
		scp,
		"Name2",
		arglex_token_name(arglex_token_version)
	    );
	    error_intl(scp, "warning: $name1 obsolete, use $name2 option");
	    sub_context_delete(scp);
	    if (new_version_number_length < 1)
		new_version_number_length = 1;
	    if (arglex() != arglex_token_number)
	    {
		new_version_number[0] = GIVEN;
		continue;
	    }
	    if (arglex_value.alv_number < 0)
		option_needs_number(arglex_token_major, new_release_usage);
	    new_version_number[0] = arglex_value.alv_number;
	    break;

	case arglex_token_minor:
	    if (new_version_number[1] != NOT_GIVEN)
		duplicate_option(new_release_usage);
	    scp = sub_context_new();
	    sub_var_set_charstar
	    (
		scp,
		"Name1",
		arglex_token_name(arglex_token_minor)
	    );
	    sub_var_set_charstar
	    (
		scp,
		"Name2",
		arglex_token_name(arglex_token_version)
	    );
	    error_intl
	    (
		scp,
		i18n("warning: $name1 obsolete, use $name2 option")
	    );
	    sub_context_delete(scp);
	    while (new_version_number_length < 2)
		new_version_number[new_version_number_length++] = NOT_GIVEN;
	    if (arglex() != arglex_token_number)
	    {
		new_version_number[1] = GIVEN;
		continue;
	    }
	    if (arglex_value.alv_number <= 0)
		option_needs_number(arglex_token_minor, new_release_usage);
	    new_version_number[1] = arglex_value.alv_number;
	    break;

	case arglex_token_version:
	    if (new_version_string)
		duplicate_option(new_release_usage);
	    switch (arglex())
	    {
	    default:
		option_needs_string(arglex_token_version, new_release_usage);

	    case arglex_token_number:
	    case arglex_token_string:
		break;
	    }
	    new_version_string = str_from_c(arglex_value.alv_string);
	    break;

	case arglex_token_nolog:
	    if (log_style == log_style_none)
		duplicate_option(new_release_usage);
	    log_style = log_style_none;
	    break;

	case arglex_token_wait:
	case arglex_token_wait_not:
	    user_ty::lock_wait_argument(new_release_usage);
	    break;
	}
	arglex();
    }
    if (!project_name_count)
	fatal_intl(0, i18n("no project name"));
    if (new_version_number_length > 0)
    {
	if (new_version_string)
	{
	    error_intl(0, i18n("don't mix old and new version options"));
	    new_release_usage();
	}
	while (new_version_number_length < 2)
	    new_version_number[new_version_number_length++] = 0;
    }

    //
    // locate OLD project data
    //
    pp[0] = project_alloc(project_name[0]);
    pp[0]->bind_existing();
    if (!pp[0]->is_a_trunk())
	goto too_modern;
    pstate_data[0] = pp[0]->pstate_get();

    //
    // You may only use this command for old-style (pre branching)
    // projects.  Only old-style projects (pre-3.0) define the
    // version_major and version_minor fields in the project state
    // data.
    //
    if (!pstate_data[0]->version_major && !pstate_data[0]->version_minor)
    {
	too_modern:
	project_fatal(pp[0], 0, i18n("bad nrls, too modern"));
    }

    //
    // locate user data
    //
    up = user_ty::create();

    //
    // it is an error if the current user is not an administrator
    // of the old project.
    //
    if (!project_administrator_query(pp[0], up->name()))
	project_fatal(pp[0], 0, i18n("not an administrator"));

    //
    // figure out the old version number
    //
    // Because we know that the old project is in the pre-3.0
    // format, at this point we know definitely that the version
    // fields are set.
    //
    old_version_number_length = 2;
    old_version_number[0] = pstate_data[0]->version_major;
    old_version_number[1] = pstate_data[0]->version_minor;

    if (project_name_count < 2)
    {
	long		junk[10];
	int		junk_length;

	//
	// Default the new project name from the old, when the
	// new name was not given.
	//
	project_name[1] = str_copy(project_name[0]);
	project_name_count = 2;

	//
	// Throw away any version information which may be
	// contained in the project name.  It is usually
	// redundant, when it is present at all.
	//
	extract_version_from_project_name
	(
	    &project_name[1],
	    junk,
	    (int)SIZEOF(junk),
	    &junk_length
	);
    }
    else
    {
	//
	// Extract the version number information from the new
	// project name.
	//
	extract_version_from_project_name
	(
	    &project_name[1],
	    new_version_number,
	    (int)SIZEOF(new_version_number),
	    &new_version_number_length
	);
    }

    //
    // Make sure the project name is acceptable.
    //
    if (!project_name_ok(project_name[1]))
	fatal_bad_project_name(project_name[1]);

    //
    // Default to a minor release incriment.
    //
    // Only do this if there is no version number implicit in the
    // project name AND no version string was given.
    //
    // This test is done BEFORE the version string break up, because
    // if the version string is the empty string, it means to use NO
    // version numbers.
    //
    if (!new_version_number_length && !new_version_string)
    {
	new_version_number_length = 2;
	new_version_number[0] = NOT_GIVEN;
	new_version_number[1] = GIVEN;
    }

    //
    // If the user specified a version string on the command line,
    // break it up into its component parts.  If there was version
    // numbers in the NEW project name, the version numbers
    // determined here will be appended to them.
    //
    if (new_version_string && new_version_string->str_length)
    {
	int		err;

	err =
	    break_up_version_string
	    (
		new_version_string->str_text,
		new_version_number,
		(int)SIZEOF(new_version_number),
		&new_version_number_length,
		0
	    );
	if (err)
	{
	    scp = sub_context_new();
	    sub_var_set_string(scp, "Number", new_version_string);
	    fatal_intl(scp, i18n("bad version $number"));
	    // NOTREACHED
	    sub_context_delete(scp);
	}
    }

    //
    // Figure the new version number.  This code only ever operates
    // for the -major and -minor options, when they are not given
    // explicit numbers.
    //
    for (j = 0; j < (size_t)new_version_number_length; ++j)
    {
	switch (new_version_number[j])
	{
	case NOT_GIVEN:
	    //
	    // Use the old version number if available.
	    // Otherwise, the major version defaults to 1,
	    // and all others default to 0.
	    //
	    if (j < (size_t)old_version_number_length)
		new_version_number[j] = old_version_number[j];
	    else
		new_version_number[j] = !j;
	    break;

	case GIVEN:
	    //
	    // Use the old version number if available.
	    // Otherwise, the major version defaults to 1,
	    // and all others default to 0.
	    //
	    if (j < (size_t)old_version_number_length)
		new_version_number[j] = old_version_number[j];
	    else
		new_version_number[j] = !j;

	    //
	    // Incriment the version number by one.
	    //
	    new_version_number[j]++;

	    //
	    // Set all of the remaining components to zero.
	    // (Usually these are the minor components,
	    // after using the -major option.)
	    //
	    for (k = j + 1; k < (size_t)new_version_number_length; ++k)
	    {
		if (new_version_number[k] == NOT_GIVEN)
		    new_version_number[k] = 0;
	    }
	    break;

	default:
	    break;
	}
    }

    //
    // locate NEW project data
    //
    // It is a fatal error if the project name already exists.
    //
    pp[1] = project_alloc(project_name[1]);
    pp[1]->bind_new();
    pp[1]->copy_the_owner(pp[0]);
    pup = project_user(pp[0]);

    //
    // if no project directory was specified
    // create the directory in their home directory.
    //
    if (!home)
    {
	nstring s3 = pup->default_project_directory();
	assert(s3);
	os_become_orig();
	int name_max = os_pathconf_name_max(s3);
	os_become_undo();
	if (project_name[1]->str_length > (size_t)name_max)
	    fatal_project_name_too_long(project_name[1], name_max);
	home = os_path_join(s3, nstring(project_name[1]));

	scp = sub_context_new();
	sub_var_set_string(scp, "File_Name", home);
	project_verbose(pp[1], scp, i18n("proj dir $filename"));
	sub_context_delete(scp);
    }
    pp[1]->home_path_set(home);

    //
    // take the relevant locks
    //
    pp[1]->pstate_lock_prepare();
    project_baseline_read_lock_prepare(pp[0]);
    gonzo_gstate_lock_prepare_new();
    lock_take();
    pstate_data[0] = pp[0]->pstate_get();
    pstate_data[1] = pp[1]->pstate_get();

    //
    // Create the directory and subdirectories.
    // It is an error if the directories can't be created.
    //
    home = nstring(pp[1]->home_path_get());
    bl = pp[1]->baseline_path_get();
    hp = pp[1]->history_path_get();
    ip = pp[1]->info_path_get();
    project_become(pp[1]);
    os_mkdir(home, 02755);
    undo_rmdir_errok(home);
    os_mkdir(bl, 02755);
    undo_rmdir_errok(bl);
    os_mkdir(hp, 02755);
    undo_rmdir_errok(hp);
    os_mkdir(ip, 02755);
    undo_rmdir_errok(ip);
    project_become_undo(pp[1]);

    //
    // create a new release state file
    //
    pstate_data[1]->next_test_number = pstate_data[0]->next_test_number;
    project_version_previous_set(pp[1], project_version_get(pp[0]));

    // administrators
    for (j = 0;; ++j)
    {
	s1 = project_administrator_nth(pp[0], j);
	if (!s1)
	    break;
	project_administrator_add(pp[1], nstring(s1));
    }

    // developers
    for (j = 0;; ++j)
    {
	s1 = project_developer_nth(pp[0], j);
	if (!s1)
	    break;
	project_developer_add(pp[1], nstring(s1));
    }

    // reviewers
    for (j = 0;; ++j)
    {
	s1 = project_reviewer_nth(pp[0], j);
	if (!s1)
	    break;
	project_reviewer_add(pp[1], nstring(s1));
    }

    // integrators
    for (j = 0;; ++j)
    {
	s1 = project_integrator_nth(pp[0], j);
	if (!s1)
	    break;
	project_integrator_add(pp[1], nstring(s1));
    }

    //
    // Copy the project attributes across.
    //
    // Please keep this in the same order as aegis/pattr.def
    // to make sure none are missed.
    //
    // DO NOT copy the major and minor version numbers across,
    // because we are creating a new-style project.
    //
    project_description_set(pp[1], project_description_get(pp[0]));
    project_developer_may_review_set
    (
	pp[1],
	project_developer_may_review_get(pp[0])
    );
    project_developer_may_integrate_set
    (
	pp[1],
	project_developer_may_integrate_get(pp[0])
    );
    project_reviewer_may_integrate_set
    (
	pp[1],
	project_reviewer_may_integrate_get(pp[0])
    );
    project_developers_may_create_changes_set
    (
	pp[1],
	project_developers_may_create_changes_get(pp[0])
    );
    project_forced_develop_begin_notify_command_set
    (
	pp[1],
	project_forced_develop_begin_notify_command_get(pp[0])
    );
    project_develop_end_notify_command_set
    (
	pp[1],
	project_develop_end_notify_command_get(pp[0])
    );
    project_develop_end_undo_notify_command_set
    (
	pp[1],
	project_develop_end_undo_notify_command_get(pp[0])
    );
    project_review_pass_notify_command_set
    (
	pp[1],
	project_review_pass_notify_command_get(pp[0])
    );
    project_review_pass_undo_notify_command_set
    (
	pp[1],
	project_review_pass_undo_notify_command_get(pp[0])
    );
    project_review_fail_notify_command_set
    (
	pp[1],
	project_review_fail_notify_command_get(pp[0])
    );
    project_integrate_pass_notify_command_set
    (
	pp[1],
	project_integrate_pass_notify_command_get(pp[0])
    );
    project_integrate_fail_notify_command_set
    (
	pp[1],
	project_integrate_fail_notify_command_get(pp[0])
    );
    project_default_development_directory_set
    (
	pp[1],
	project_default_development_directory_get(pp[0])
    );
    project_umask_set(pp[1], project_umask_get(pp[0]));
    project_default_test_exemption_set
    (
	pp[1],
	project_default_test_exemption_get(pp[0])
    );
    project_default_test_regression_exemption_set
    (
	pp[1],
	project_default_test_regression_exemption_get(pp[0])
    );
    project_minimum_change_number_set
    (
	pp[1],
	project_minimum_change_number_get(pp[0])
    );
    project_reuse_change_numbers_set
    (
	pp[1],
	project_reuse_change_numbers_get(pp[0])
    );
    project_minimum_branch_number_set
    (
	pp[1],
	project_minimum_branch_number_get(pp[0])
    );

    //
    // add a row to the project table
    //
    gonzo_project_add(pp[1]);

    //
    // create each of the branches
    // (attributes are inherited)
    //
    ppp = pp[1];
    for (j = 0; j < (size_t)new_version_number_length; ++j)
    {
	trace(("ppp = %8.8lX\n", (long)ppp));
	change_number = magic_zero_encode(new_version_number[j]);
	trace(("change_number = %ld;\n", change_number));
	ppp = project_new_branch(ppp, up, change_number);
	version_pp[j] = ppp;
    }

    //
    // Add all of the files to the final branch.
    //
    change_number = project_next_change_number(ppp, 1);
    cp = change_alloc(ppp, change_number);
    change_bind_new(cp);
    cstate_data = cp->cstate_get();
    scp = sub_context_new();
    sub_var_set_string(scp, "Name", project_name[0]);
    cstate_data->brief_description =
	subst_intl(scp, i18n("New release derived from $name."));
    sub_context_delete(scp);
    cstate_data->cause = change_cause_internal_enhancement;
    cstate_data->test_exempt = true;
    cstate_data->test_baseline_exempt = true;
    project_change_append(ppp, change_number, 0);

    //
    // lots of fake history so we don't confuse
    // anything later with otherwise illegal state transitions
    //
    chp = change_history_new(cp, up);
    chp->what = cstate_history_what_new_change;
    chp = change_history_new(cp, up);
    chp->what = cstate_history_what_develop_begin;
    chp = change_history_new(cp, up);
    cstate_data->development_directory = os_edit_filename(0);
    chp->what = cstate_history_what_develop_end;
    chp = change_history_new(cp, up);
    chp->what = cstate_history_what_review_pass;
    chp = change_history_new(cp, up);
    chp->what = cstate_history_what_integrate_begin;
    cstate_data->state = cstate_state_being_integrated;
    bl = ppp->baseline_path_get();
    cstate_data->integration_directory = str_copy(bl);

    //
    // update the copyright years of the change
    //
    change_copyright_years_now(cp);

    //
    // update the copyright years of the branch
    // (a) from the original project
    // (b) from the fake change that created the files
    //
    project_copyright_years_merge(ppp, pp[0]->change_get());
    project_copyright_years_merge(ppp, cp);

    //
    // add all of the files to the change
    //
    for (j = 0;; ++j)
    {
	fstate_src_ty   *p_src_data;
	fstate_src_ty   *p1_src_data;
	fstate_src_ty   *c_src_data;

	p_src_data = pp[0]->file_nth(j, view_path_extreme);
	if (!p_src_data)
	    break;

	p1_src_data = ppp->file_new(p_src_data);
	p1_src_data->action = file_action_create;

	c_src_data = cp->file_new(p_src_data);
	c_src_data->action = file_action_create;

	//
	// copy testing correlations
	//
	if (p_src_data->test && p_src_data->test->length)
	{
	    size_t	    m;

	    p1_src_data->test =
                (fstate_src_test_list_ty *)fstate_src_test_list_type.alloc();
	    for (m = 0; m < p_src_data->test->length; ++m)
	    {
		meta_type *type_p = 0;
		string_ty **addr_p =
		    (string_ty **)
		    fstate_src_test_list_type.list_parse
		    (
			p1_src_data->test,
			&type_p
		    );
		assert(type_p == &string_type);
		*addr_p = str_copy(p_src_data->test->list[m]);
	    }
	}
    }

    //
    // Open the log file
    //
    pup1 = project_user(pp[1]);
    s1 = str_format("%s/%s.log", bl->str_text, progname_get());
    log_open(s1, pup1, log_style);
    str_free(s1);

    //
    // copy files from old baseline to new baseline
    //
    info.from = pp[0]->baseline_path_get(true);
    info.to = bl;
    project_verbose(ppp, 0, i18n("copy baseline"));
    project_become(ppp);
    dir_walk(info.from, copy_tree_callback, &info);
    project_become_undo(ppp);

    //
    // Build all of the difference files,
    // and record the fingerprints.
    //
    for (j = 0;; ++j)
    {
	fstate_src_ty   *src_data;
	string_ty	*original;
	string_ty	*path;
	string_ty	*path_d;

	//
	// find the relevant change src data
	//
	src_data = ppp->file_nth(j, view_path_extreme);
	if (!src_data)
	    break;

	//
	// generated files are not fingerprinted or differenced
	//
	switch (src_data->usage)
	{
	case file_usage_build:
	    continue;

	case file_usage_source:
	case file_usage_config:
	case file_usage_test:
	case file_usage_manual_test:
	    break;
	}

	//
	// build the path to the source file
	//
	path = os_path_join(bl, src_data->file_name);
	assert(path);
	trace_string(path->str_text);

	//
	// Record the source file's fingerprint.
	//
	if (!src_data->file_fp)
        {
	    src_data->file_fp = (fingerprint_ty *)fingerprint_type.alloc();
        }
	project_become(ppp);
	change_fingerprint_same(src_data->file_fp, path, 1);
	project_become_undo(ppp);

	//
	// Don't bother differencing the file for the project
	// trunk.
	//
	if (ppp->is_a_trunk())
	{
	    str_free(path);
	    continue;
	}

	//
	// build the path to the difference file
	//
	path_d = str_format("%s,D", path->str_text);
	trace_string(path_d->str_text);

	//
	// Run the difference command.	All newly create
	// files are differenced against /dev/null, and
	// every file in this change is a new file.
	//
	original = str_from_c("/dev/null");
	change_run_diff_command(cp, pup1, original, path, path_d);
	str_free(original);

	//
	// Record the fingerprint of the difference file.
	//
	if (!src_data->diff_file_fp)
	    src_data->diff_file_fp = (fingerprint_ty *)fingerprint_type.alloc();
        user_ty::become scoped(pup1);
	change_fingerprint_same(src_data->diff_file_fp, path_d, 1);
	str_free(path);
	str_free(path_d);
    }

    //
    // build history files
    //
    for (j = 0;; ++j)
    {
	fstate_src_ty   *c_src_data;
	fstate_src_ty   *p_src_data;

	c_src_data = change_file_nth(cp, j, view_path_first);
	if (!c_src_data)
	    break;
	p_src_data =
	    project_file_find(ppp, c_src_data->file_name, view_path_none);
	assert(p_src_data);

	//
	// create a new history file
	//
	change_run_history_create_command(cp, c_src_data);

	//
	// Extract the version number from the history file.
	// Record it in the project and in the change.
	//
	assert(c_src_data->edit);
	assert(c_src_data->edit->revision);
	p_src_data->edit = history_version_copy(c_src_data->edit);
	p_src_data->edit_origin = history_version_copy(c_src_data->edit);

	//
	// Don't set edit number origin in change file state
	// for created files.
	// Why not?  Because it only means something for branches
	// once the change is integrated.
	//
    }

    //
    // some more history
    //
    str_free(cstate_data->integration_directory);
    cstate_data->integration_directory = 0;
    str_free(cstate_data->development_directory);
    cstate_data->development_directory = 0;
    chp = change_history_new(cp, up);
    chp->what = cstate_history_what_integrate_pass;
    cstate_data->delta_number = 1;
    project_history_new(ppp, cstate_data->delta_number, change_number);
    cstate_data->state = cstate_state_completed;

    //
    // write the project state
    //	    (the trunk change state is implicitly written)
    //
    // Write each of the branch states.	 You must write each one
    // AFTER the next branch down has been created, because
    // creating a branch alters the pstate of the one above.
    //
    pp[1]->pstate_write();
    for (j = 0; j < (size_t)new_version_number_length; ++j)
	version_pp[j]->pstate_write();
    change_cstate_write(cp);
    gonzo_gstate_write();

    //
    // release locks
    //
    commit();
    lock_release();

    //
    // verbose success message
    //
    project_verbose(pp[1], 0, i18n("new release complete"));
    for (j = 0; j < (size_t)new_version_number_length; ++j)
	project_verbose(version_pp[j], 0, i18n("new release complete"));
    str_free(project_name[0]);
    project_free(pp[0]);
    str_free(project_name[1]);
    project_free(pp[1]);
    change_free(cp);
    trace(("}\n"));
}


void
new_release(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
	{ arglex_token_help, new_release_help, 0 },
	{ arglex_token_list, new_release_list, 0 },
    };

    trace(("new_release()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), new_release_main);
    trace(("}\n"));
}

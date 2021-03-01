//
//	aegis - project change supervisor
//	Copyright (C) 1991-1999, 2001-2006 Peter Miller;
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
// MANIFEST: functions to implement new project
//

#include <common/ac/ctype.h>
#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>

#include <libaegis/ael/project/projects.h>
#include <aegis/aenpr.h>
#include <libaegis/arglex2.h>
#include <libaegis/arglex/project.h>
#include <libaegis/change.h>
#include <libaegis/change/branch.h>
#include <libaegis/commit.h>
#include <common/error.h>
#include <libaegis/gonzo.h>
#include <libaegis/help.h>
#include <libaegis/io.h>
#include <libaegis/lock.h>
#include <libaegis/os.h>
#include <common/progname.h>
#include <libaegis/project.h>
#include <libaegis/project/pattr/edit.h>
#include <libaegis/project/pattr/get.h>
#include <libaegis/project/pattr/set.h>
#include <libaegis/project/verbose.h>
#include <libaegis/project/history.h>
#include <common/quit.h>
#include <libaegis/sub.h>
#include <common/trace.h>
#include <libaegis/undo.h>
#include <libaegis/user.h>


static void
new_project_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf
    (
	stderr,
	"usage: %s -New_Project <name> [ <option>... ]\n",
	progname
    );
    fprintf(stderr, "       %s -New_Project -List [ <option>... ]\n", progname);
    fprintf(stderr, "       %s -New_Project -Help\n", progname);
    quit(1);
}


static void
new_project_help(void)
{
    help("aenpr", new_project_usage);
}


static void
new_project_list(void)
{
    arglex();
    while (arglex_token != arglex_token_eoln)
	generic_argument(new_project_usage);
    list_projects(0, 0, 0);
}


static void
new_project_main(void)
{
    string_ty	    *s;
    sub_context_ty  *scp;
    string_ty	    *home;
    string_ty	    *s1;
    string_ty	    *project_name;
    project_ty	    *pp;
    project_ty	    *ppp;
    user_ty	    *up;
    string_ty	    *bl;
    string_ty	    *hp;
    string_ty	    *ip;
    long	    version_number[10];
    int		    version_number_length;
    project_ty	    *version_pp[SIZEOF(version_number)];
    string_ty	    *version_string;
    long	    j;
    int		    keep;
    pattr_ty	    *pattr_data;
    edit_ty	    edit;

    trace(("new_project_main()\n{\n"));
    arglex();
    project_name = 0;
    home = 0;
    version_number_length = 0;
    version_string = 0;
    keep = -1;
    pattr_data = 0;
    edit = edit_not_set;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(new_project_usage);
	    continue;

	case arglex_token_project:
	    arglex();
	    // fall through...

	case arglex_token_string:
	    arglex_parse_project(&project_name, new_project_usage);
	    continue;

	case arglex_token_directory:
	    if (home)
		duplicate_option(new_project_usage);
	    if (arglex() != arglex_token_string)
		option_needs_dir(arglex_token_directory, new_project_usage);

	    //
	    // To cope with automounters, directories are stored as
	    // given, or are derived from the home directory in the
	    // passwd file.  Within aegis, pathnames have their
	    // symbolic links resolved, and any comparison of paths
	    // is done on this "system idea" of the pathname.
	    //
	    home = str_from_c(arglex_value.alv_string);
	    break;

	case arglex_token_edit:
	    if (edit == edit_foreground)
		duplicate_option(new_project_usage);
	    if (edit != edit_not_set)
	    {
		too_many_edits:
		mutually_exclusive_options
		(
		    arglex_token_edit,
		    arglex_token_edit_bg,
		    new_project_usage
		);
	    }
	    edit = edit_foreground;
	    break;

	case arglex_token_edit_bg:
	    if (edit == edit_background)
		duplicate_option(new_project_usage);
	    if (edit != edit_not_set)
		goto too_many_edits;
	    edit = edit_background;
	    break;

	case arglex_token_file:
	    if (pattr_data)
		duplicate_option(new_project_usage);
	    switch (arglex())
	    {
	    default:
		option_needs_file(arglex_token_file, new_project_usage);
		// NOTREACHED

	    case arglex_token_string:
		os_become_orig();
		s = str_from_c(arglex_value.alv_string);
		pattr_data = pattr_read_file(s);
		str_free(s);
		os_become_undo();
		break;

	    case arglex_token_stdio:
		os_become_orig();
		pattr_data = pattr_read_file((string_ty *)0);
		os_become_undo();
		break;
	    }
	    assert(pattr_data);
	    break;

	case arglex_token_major:
	    if (version_number_length > 0 && version_number[0])
		duplicate_option(new_project_usage);
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
	    error_intl
	    (
		scp,
		i18n("warning: $name1 obsolete, use $name2 option")
	    );
	    sub_context_delete(scp);
	    if (arglex() != arglex_token_number)
		option_needs_number(arglex_token_major, new_project_usage);
	    if (version_number_length < 1)
		version_number_length = 1;
	    version_number[0] = arglex_value.alv_number;
	    if (version_number[0] <= 0)
		option_needs_number(arglex_token_major, new_project_usage);
	    break;

	case arglex_token_minor:
	    if (version_number_length >= 2 && version_number[1])
		duplicate_option(new_project_usage);
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
	    if (arglex() != arglex_token_number)
		option_needs_number(arglex_token_minor, new_project_usage);
	    if (version_number_length < 1)
		version_number[0] = 0;
	    if (version_number_length < 2)
		version_number_length = 2;
	    version_number[1] = arglex_value.alv_number;
	    if (version_number[1] <= 0)
		option_needs_number(arglex_token_minor, new_project_usage);
	    break;

	case arglex_token_version:
	    if (version_string)
		duplicate_option(new_project_usage);
	    switch (arglex())
	    {
	    default:
		option_needs_number(arglex_token_version, new_project_usage);

	    case arglex_token_number:
	    case arglex_token_string:
		version_string = str_from_c(arglex_value.alv_string);
		break;

	    case arglex_token_stdio:
		version_string = str_from_c("");
		break;
	    }
	    break;

	case arglex_token_wait:
	case arglex_token_wait_not:
	    user_lock_wait_argument(new_project_usage);
	    break;

	case arglex_token_keep:
	    if (keep > 0)
		duplicate_option(new_project_usage);
	    if (keep >= 0)
	    {
		mut_exc_keep:
		mutually_exclusive_options
		(
		    arglex_token_keep,
		    arglex_token_keep_not,
		    new_project_usage
		);
	    }
	    keep = 1;
	    break;

	case arglex_token_keep_not:
	    if (keep == 0)
		duplicate_option(new_project_usage);
	    if (keep >= 0)
		goto mut_exc_keep;
	    keep = 0;
	    break;
	}
	arglex();
    }
    if (keep < 0)
	keep = 0;
    if (!project_name)
    {
	error_intl(0, i18n("no project name"));
	new_project_usage();
    }
    if (version_number_length > 0)
    {
	if (version_string)
	{
	    error_intl(0, i18n("don't mix old and new version options"));
	    new_project_usage();
	}
	if (keep)
	{
	    mutually_exclusive_options
	    (
		arglex_token_keep,
		(
		    version_number_length < 2
		?
		    arglex_token_major
		:
		    arglex_token_minor
		),
		new_project_usage
	    );
	}
	while (version_number_length < 2)
	    version_number[version_number_length++] = 0;
	if (version_number[0] == 0)
	    version_number[0] = 1;
    }
    extract_version_from_project_name
    (
	&project_name,
	version_number,
	(int)SIZEOF(version_number),
	&version_number_length
    );
    if (!project_name_ok(project_name))
	fatal_bad_project_name(project_name);
    if (!keep && !version_number_length && !version_string)
    {
	//
	// Only do this if there is no version number implicit
	// in the project name AND no version string was given.
	//
	// This test is done BEFORE the version string break up,
	// because if the version string is the empty string, it
	// means to use NO version numbers.
	//
	version_number_length = 2;
	version_number[0] = 1;
	version_number[1] = 0;
    }
    if (version_string && version_string->str_length)
    {
	int		err;

	if (keep)
	{
	    mutually_exclusive_options
	    (
		arglex_token_keep,
		arglex_token_version,
		new_project_usage
	    );
	}
	err =
	    break_up_version_string
	    (
		version_string->str_text,
		version_number,
		(int)SIZEOF(version_number),
		&version_number_length,
		0
	    );
	if (err)
	{
	    scp = sub_context_new();
	    sub_var_set_string(scp, "Number", version_string);
	    fatal_intl(scp, i18n("bad version $number"));
	    // NOTREACHED
	    sub_context_delete(scp);
	}
    }
    if (keep && edit == edit_foreground)
    {
	mutually_exclusive_options
	(
	    arglex_token_edit,
	    arglex_token_keep,
	    new_project_usage
	);
    }
    if (keep && edit == edit_background)
    {
	mutually_exclusive_options
	(
	    arglex_token_edit_bg,
	    arglex_token_keep,
	    new_project_usage
	);
    }
    if (keep && pattr_data)
    {
	mutually_exclusive_options
	(
	    arglex_token_file,
	    arglex_token_keep,
	    new_project_usage
	);
    }
    if (!pattr_data)
    {
	pattr_data = (pattr_ty *)pattr_type.alloc();
    }
    if (!pattr_data->description)
	pattr_data->description =
	    str_format("The \"%s\" program.", project_name->str_text);
    if (!(pattr_data->mask & pattr_reuse_change_numbers_mask))
    {
	pattr_data->reuse_change_numbers = true;
	pattr_data->mask |= pattr_reuse_change_numbers_mask;
    }
    if (!(pattr_data->mask & pattr_default_test_regression_exemption_mask))
    {
	pattr_data->default_test_regression_exemption = true;
	pattr_data->mask |= pattr_default_test_regression_exemption_mask;
    }
    if (edit != edit_not_set)
    {
	scp = sub_context_new();
	sub_var_set_string(scp, "Name", project_name);
	io_comment_append(scp, "Project $name");
	sub_context_delete(scp);
	project_pattr_edit(&pattr_data, edit);

	//
	// You can't have no description.
	//
	if (!pattr_data->description)
	    pattr_data->description =
		str_format("The \"%s\" program.", project_name->str_text);
	if (!(pattr_data->mask & pattr_reuse_change_numbers_mask))
	{
	    pattr_data->reuse_change_numbers = true;
	    pattr_data->mask |= pattr_reuse_change_numbers_mask;
	}
    }

    //
    // locate user data
    //
    up = user_executing((project_ty *)0);

    //
    // read in the project table
    //
    gonzo_gstate_lock_prepare_new();
    lock_take();

    //
    // make sure not too privileged
    //
    if (!user_uid_check(up->name))
	fatal_user_too_privileged(user_name(up));
    if (!user_gid_check(up->group))
	fatal_group_too_privileged(user_group(up));

    //
    // it is an error if the name is already in use
    //
    if (gonzo_alias_to_actual(project_name))
	fatal_project_alias_exists(project_name);
    pp = project_alloc(project_name);
    str_free(project_name);
    if (keep)
    {
	if (gonzo_project_home_path_from_name(project_name))
	{
	    scp = sub_context_new();
	    sub_var_set_string(scp, "Name", pp->name);
	    fatal_intl(scp, i18n("project $name exists"));
	    // NOTREACHED
	    sub_context_delete(scp);
	}
	if (!home)
	{
	    error_intl(0, i18n("aenpr -keep needs -dir"));
	    new_project_usage();
	    // NOTREACHED
	}
	pp->home_path_set(os_canonify_dirname(home));
    }
    else
	pp->bind_new();

    //
    // the user who ran the command is the project administrator.
    // no other staff are defined at this time.
    //
    project_administrator_add(pp, user_name(up));

    //
    // if no project directory was specified
    // create the directory in their home directory.
    //
    if (!home)
    {
	int		max;

	s1 = user_default_project_directory(up);
	assert(s1);
	os_become_orig();
	max = os_pathconf_name_max(s1);
	os_become_undo();
	if (project_name_get(pp)->str_length > (size_t)max)
	    fatal_project_name_too_long(project_name_get(pp), max);
	home = os_path_join(s1, project_name_get(pp));
	str_free(s1);

	project_verbose_directory(pp, home);
    }
    if (!keep)
	pp->home_path_set(home);
    str_free(home);

    //
    // create the directory and subdirectories.
    // It is an error if the directories can't be created.
    //
    home = pp->home_path_get();
    bl = pp->baseline_path_get();
    hp = pp->history_path_get();
    ip = pp->info_path_get();
    if (!keep)
    {
	project_become(pp);
	os_mkdir(home, 02755);
	undo_rmdir_errok(home);
	os_mkdir(bl, 02755);
	undo_rmdir_errok(bl);
	os_mkdir(hp, 02755);
	undo_rmdir_errok(hp);
	os_mkdir(ip, 02755);
	undo_rmdir_errok(ip);
	project_become_undo();
    }

    //
    // add a row to the table
    //
    gonzo_project_add(pp);

    //
    // copy the attributes into the project
    //
    if (!keep)
	project_pattr_set(pp, pattr_data);
    pattr_type.free(pattr_data);

    //
    // create each of the branches
    //
    ppp = pp;
    for (j = 0; !keep && j < version_number_length; ++j)
    {
	trace(("ppp = %8.8lX\n", (long)ppp));
	long change_number = magic_zero_encode(version_number[j]);
	trace(("change_number = %ld;\n", change_number));
	ppp = project_new_branch(ppp, up, change_number);
	version_pp[j] = ppp;
    }

    //
    // write the project state
    //	    (the trunk change state is implicitly written)
    //
    // Write each of the branch state.	You must write *after* the
    // next branch down is created, because creating a branch alters
    // pstate.
    //
    pp->pstate_write();
    for (j = 0; j < version_number_length; ++j)
	version_pp[j]->pstate_write();
    gonzo_gstate_write();

    //
    // release locks
    //
    commit();
    lock_release();

    //
    // verbose success message
    //
    project_verbose_new_project_complete(pp);
    for (j = 0; j < version_number_length; ++j)
	project_verbose_new_branch_complete(version_pp[j]);

    project_free(pp);
    user_free(up);
    trace(("}\n"));
}


void
new_project(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
	{arglex_token_help, new_project_help, },
	{arglex_token_list, new_project_list, },
    };

    trace(("new_project()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), new_project_main);
    trace(("}\n"));
}

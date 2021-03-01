/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993 Peter Miller.
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
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * MANIFEST: functions to implement new release
 */

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <ael.h>
#include <aenrls.h>
#include <arglex2.h>
#include <change.h>
#include <commit.h>
#include <dir.h>
#include <error.h>
#include <file.h>
#include <gonzo.h>
#include <help.h>
#include <lock.h>
#include <log.h>
#include <mem.h>
#include <option.h>
#include <os.h>
#include <project.h>
#include <sub.h>
#include <trace.h>
#include <undo.h>
#include <user.h>

#define GIVEN -1
#define NOT_GIVEN -2


static void new_release_usage _((void));

static void
new_release_usage()
{
	char	*progname;

	progname = option_progname_get();
	fprintf(stderr, "usage: %s -New_ReLeaSe <name> [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -New_ReLeaSe -List [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -New_ReLeaSe -Help\n", progname);
	quit(1);
}


static void new_release_help _((void));

static void
new_release_help()
{
	static char *text[] =
	{
"NAME",
"	%s -New_ReLeaSe - create a new project from an existing",
"	project.",
"",
"SYNOPSIS",
"	%s -New_ReLeaSe <project-name> [ <new-project-name> ][ <option>... ]",
"	%s -New_ReLeaSe -List [ <option>... ]",
"	%s -New_ReLeaSe -Help",
"",
"DESCRIPTION",
"	The %s -New_ReLeaSe command is used to create a new",
"	project from an existing project.",
"",
"	If no new-project-name is specified, it will be derived",
"	from the project given as follows: any minor version dot",
"	suffix will be removed from the name, then any major",
"	version dot suffix will be removed from the name.  A major",
"	version dot suffix will be appended, and then a minor",
"	version dot suffix will be appended.  As an example,",
"	\"foo.1.0\" would become \"foo.1.1\" assuming the default",
"	minor version incriment, and \"foo\" would become \"foo.1.1\"",
"	assiming the same minor version incriment.",
"",
"	The entire project baseline will be copied.  The project",
"	state will be as if change 1 had already been integrated,",
"	naming every file (in the old project) as a new file.  The",
"	history files will reflect this.  No build will be",
"	necessary; it is assumed that the old baseline was built",
"	sucessfully.  Change numbers will commence at 2, as will",
"	build numbers.  Test numbers will commence where the old",
"	project left off (because all the earlier test numbers",
"	were used by the old project).",
"",
"	The default is for the minor version number to be",
"	incrimented.  If the major version number is incrimented",
"	or set, the minor version number will be set to zero if it",
"	is not explicitly given.",
"",
"	The pointer to the new project will be added to the first",
"	element of the search path, or /usr/local/lib/%s if",
"	none is set.  If this is inappropriate, use the",
"	-LIBrary option to explicitly set the desired location.",
"	See the -LIBrary option for more information.",
"",
"	The project directory, under which the project baseline",
"	and history and state and change data are kept, will be",
"	created at this time.  If the -DIRectory option is not",
"	given, the project directory will be created in the",
"	directory specified by the default_project_directory field",
"	of the project user's aeuconf(5), or if not set in project",
"	user's home directory; in either case with the same name",
"	as the project.",
"",
"	All staff will be copied from the old project to the new",
"	project without change, as will all of the project",
"	attributes.",
"",
"OPTIONS",
"	The following options are understood:",
"",
"	-DIRectory <path>",
"		This option may be used to specify which directory",
"		is to be used.  It is an error if the current user",
"		does not have appropriate permissions to create",
"		the directory path given.  This must be an",
"		absolute path.",
"",
"		Caution: If you are using an automounter do not",
"		use `pwd` to make an absolute path, it usually",
"		gives the wrong answer.",
"",
"	-Help",
"		This option may be used to obtain more information",
"		about how to use the %s program.",
"",
"	-LIBrary <abspath>",
"		This option may be used to specify a directory to",
"		be searched for global state files and user state",
"		files.  (See aegstate(5) and aeustate(5) for more",
"		information.)  Several library options may be",
"		present on the command line, and are search in the",
"		order given.  Appended to this explicit search",
"		path are the directories specified by the AEGIS",
"		enviroment variable (colon separated), and",
"		finally, /usr/local/lib/%s is always searched.",
"		All paths specified, either on the command line or",
"		in the AEGIS environment variable, must be",
"		absolute.",
"",
"	-List",
"		This option may be used to obtain a list of",
"		suitable subjects for this command.  The list may",
"		be more general than expected.",
"",
"	-MAJor [ <number> ]",
"		Specify that the major version number is to be",
"		incrimented, or set to the given number if",
"		specified.",
"",
"	-MINOr [ <number> ]",
"		Specify that the minor version number is to be",
"		incrimented, or set to the given number if",
"		specified.",
"",
"	-Not_Logging",
"		This option may be used to disable the automatic",
"		logging of output and errors to a file.  This is",
"		often useful when several %s commands are",
"		combined in a shell script.",
"",
"	-TERse",
"		This option may be used to cause listings to",
"		produce the bare minimum of information.  It is",
"		usually useful for shell scripts.",
"",
"	-Verbose",
"		This option may be used to cause %s to produce",
"		more output.  By default %s only produces",
"		output on errors.  When used with the -List option",
"		this option causes column headings to be added.",
"",
"	All options may be abbreviated; the abbreviation is",
"	documented as the upper case letters, all lower case",
"	letters and underscores (_) are optional.  You must use",
"	consecutive sequences of optional letters.",
"",
"	All options are case insensitive, you may type them in",
"	upper case or lower case or a combination of both, case",
"	is not important.",
"",
"	For example: the arguments \"-project, \"-PROJ\" and \"-p\"",
"	are all interpreted to mean the -Project option.  The",
"	argument \"-prj\" will not be understood, because",
"	consecutive optional characters were not supplied.",
"",
"	Options and other command line arguments may be mixed",
"	arbitrarily on the command line, after the function",
"	selectors.",
"",
"	The GNU long option names are understood.  Since all",
"	option names for aegis are long, this means ignoring the",
"	extra leading '-'.  The \"--option=value\" convention is",
"	also understood.",
"",
"RECOMMENDED ALIAS",
"	The recommended alias for this command is",
"	csh%%	alias aenrls '%s -nrls \\!* -v'",
"	sh$	aenrls(){%s -nrls $* -v}",
"",
"ERRORS",
"	It is an error if the old project named does not exist.",
"",
"	It is an error if the old project named has not yet had",
"	any changes integrated.",
"",
"	It is an error if the old project named has any changes",
"	not in the completed state.",
"",
"	It is an error if the current user is not an administrator",
"	of the old project.",
"",
"EXIT STATUS",
"	The %s command will exit with a status of 1 on any",
"	error.  The %s command will only exit with a status of",
"	0 if there are no errors.",
"",
"COPYRIGHT",
"	%C",
"",
"AUTHOR",
"	%A",
	};

	help(text, SIZEOF(text), new_release_usage);
}


static void new_release_list _((void));

static void
new_release_list()
{
	arglex();
	while (arglex_token != arglex_token_eoln)
		generic_argument(new_release_usage);
	list_projects(0, 0);
}


static void remove_suffix _((char *, char *));

static void
remove_suffix(str, suf)
	char	*str;
	char	*suf;
{
	size_t	str_len;
	size_t	suf_len;

	str_len = strlen(str);
	suf_len = strlen(suf);
	if
	(
		str_len > suf_len + 1
	&&
		ispunct(str[str_len - suf_len - 1])
	&&
		!strcmp(str + str_len - suf_len, suf)
	)
		str[str_len - suf_len - 1] = 0;
}


static string_ty *build_new_name _((string_ty *, long, long, long, long));

static string_ty *
build_new_name(s, major_old, minor_old, major_new, minor_new)
	string_ty	*s;
	long		major_old;
	long		minor_old;
	long		major_new;
	long		minor_new;
{
	char		tmp[1000];
	char		suffix[20];

	strcpy(tmp, s->str_text);
	sprintf(suffix, "%ld", minor_old);
	remove_suffix(tmp, suffix);
	sprintf(suffix, "%ld", major_old);
	remove_suffix(tmp, suffix);
	return str_format("%s.%ld.%ld", tmp, major_new, minor_new);
}


typedef struct copy_tree_arg_ty copy_tree_arg_ty;
struct copy_tree_arg_ty
{
	string_ty	*from;
	string_ty	*to;
};


static void copy_tree_callback _((void *, dir_walk_message_ty, string_ty *,
	struct stat *));

static void
copy_tree_callback(arg, message, path, st)
	void		*arg;
	dir_walk_message_ty message;
	string_ty	*path;
	struct stat	*st;
{
	string_ty	*s1;
	string_ty	*s2;
	copy_tree_arg_ty	*info;

	trace(("copy_tree_callback(arg = %08lX, message = %d, path = %08lX, st = %08lX)\n{\n"/*}*/, arg, message, path, st));
	info = (copy_tree_arg_ty *)arg;
	trace_string(path->str_text);
	s1 = os_below_dir(info->from, path);
	assert(s1);
	trace_string(s1->str_text);
	if (!s1->str_length)
		s2 = str_copy(info->to);
	else
		s2 = str_format("%S/%S", info->to, s1);
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
		/*
		 * copy the file
		 */
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
	trace((/*{*/"}\n"));
}


static void new_release_main _((void));

static void
new_release_main()
{
	string_ty	*ip;
	string_ty	*bl;
	string_ty	*hp;
	long		major_new;
	long		minor_new;
	int		j;
	pstate		pstate_data[2];
	string_ty	*home;
	string_ty	*s1;
	string_ty	*s2;
	string_ty	*project_name[2];
	int		project_name_count;
	project_ty	*pp[2];
	change_ty	*cp;
	cstate_history	chp;
	pstate_history	php;
	cstate		cstate_data;
	copy_tree_arg_ty	info;
	pconf		pconf_data;
	int		nolog;
	user_ty		*up;
	user_ty		*pup;

	trace(("new_release_main()\n{\n"/*}*/));
	nolog = 0;
	home = 0;
	project_name_count = 0;
	major_new = NOT_GIVEN;
	minor_new = NOT_GIVEN;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(new_release_usage);
			continue;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				new_release_usage();
			/* fall through... */

		case arglex_token_string:
			if (project_name_count >= 2)
				fatal("too many project names given");
			project_name[project_name_count++] =
				str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_directory:
			if (arglex() != arglex_token_string)
		  		new_release_usage();
			if (home)
			{
				duplicate:
				fatal("duplicate %s option", arglex_value.alv_string);
			}
			s1 = str_from_c(arglex_value.alv_string);
			os_become_orig();
			home = os_pathname(s1, 1);
			os_become_undo();
			str_free(s1);
			break;

		case arglex_token_major:
			if (major_new != NOT_GIVEN)
				goto duplicate;
			if (arglex() != arglex_token_number)
			{
				major_new = GIVEN;
				continue;
			}
			major_new = arglex_value.alv_number;
			if (major_new < 1)
				fatal("major version number out of range");
			break;

		case arglex_token_minor:
			if (minor_new != NOT_GIVEN)
				goto duplicate;
			if (arglex() != arglex_token_number)
			{
				minor_new = GIVEN;
				continue;
			}
			minor_new = arglex_value.alv_number;
			if (minor_new < 0)
				fatal("minor version number out of range");
			break;

		case arglex_token_nolog:
			if (nolog)
				goto duplicate;
			nolog = 1;
			break;
		}
		arglex();
	}
	if (!project_name_count)
	{
		fatal
		(
"You must name the project to be used as the basis for the new release.  \
You may optionally specify a second name as the name of the new project."
		);
	}

	/*
	 * locate OLD project data
	 */
	pp[0] = project_alloc(project_name[0]);
	project_bind_existing(pp[0]);
	pstate_data[0] = project_pstate_get(pp[0]);

	/*
	 * locate user data
	 */
	up = user_executing(pp[0]);

	/*
	 * it is an error if the current user is not an administrator
	 * of the old project.
	 */
	if (!project_administrator_query(pp[0], user_name(up)))
	{
		project_fatal
		(
			pp[0],
			"user \"%S\" is not an administrator",
			user_name(up)
		);
	}

	/*
	 * figure the new version number
	 */
	switch (major_new)
	{
	case NOT_GIVEN:
		major_new = pstate_data[0]->version_major;
		major_not_given:
		switch (minor_new)
		{
		case NOT_GIVEN:
		case GIVEN:
			minor_new = pstate_data[0]->version_minor + 1;
			break;

		default:
			if (minor_new <= pstate_data[0]->version_minor)
			{
				fatal
				(
	      "minor version number too small (you gave %d, the default is %d)",
					minor_new,
					pstate_data[0]->version_minor + 1
				);
			}
			break;
		}
		break;

	case GIVEN:
		major_new = pstate_data[0]->version_major + 1;
		switch (minor_new)
		{
		case NOT_GIVEN:
		case GIVEN:
			minor_new = 0;
			break;

		default:
			break;
		}
		break;

	default:
		if (major_new == pstate_data[0]->version_major)
			goto major_not_given;
		if (major_new <= pstate_data[0]->version_major)
		{
			fatal
			(
	      "major version number too small (you gave %d, the default is %d)",
				major_new,
				pstate_data[0]->version_major + 1
			);
		}
		switch (minor_new)
		{
		case NOT_GIVEN:
		case GIVEN:
			minor_new = 0;
			break;

		default:
			break;
		}
		break;
	}

	/*
	 * build new project name if none given
	 */
	if (project_name_count < 2)
	{
		project_name[project_name_count++] =
			build_new_name
			(
				project_name[0],
				pstate_data[0]->version_major,
				pstate_data[0]->version_minor,
				major_new,
				minor_new
			);
	}

	/*
	 * locate NEW project data
	 */
	pp[1] = project_alloc(project_name[1]);
	project_bind_new(pp[1]);
	pup = project_user(pp[0]);

	/*
	 * read in the table
	 */
	project_pstate_lock_prepare(pp[1]);
	project_build_read_lock_prepare(pp[0]);
	gonzo_gstate_lock_prepare_new();
	lock_take();
	pstate_data[0] = project_pstate_get(pp[0]);
	pstate_data[1] = project_pstate_get(pp[1]);

	/*
	 * create a new release state file
	 */
	pstate_data[1] = project_pstate_get(pp[1]);
	pstate_data[1]->description = str_copy(pstate_data[0]->description);
	pstate_data[1]->next_change_number = 2;
	pstate_data[1]->next_delta_number = 2;
	pstate_data[1]->next_test_number = pstate_data[0]->next_test_number;
	pstate_data[1]->version_major = major_new;
	pstate_data[1]->version_minor = minor_new;
	pstate_data[1]->owner_name = str_copy(project_owner(pp[0]));
	pstate_data[1]->group_name = str_copy(project_group(pp[0]));
	pstate_data[1]->version_previous = project_version_get(pp[0]);
	pstate_data[1]->umask = pstate_data[0]->umask;
	pstate_data[1]->default_test_exemption =
		pstate_data[0]->default_test_exemption;
	/* administrators */
	for (j = 0; j < pstate_data[0]->administrator->length; ++j)
	{
		project_administrator_add
		(
			pp[1],
			pstate_data[0]->administrator->list[j]
		);
	}
	/* developers */
	for (j = 0; j < pstate_data[0]->developer->length; ++j)
	{
		project_developer_add
		(
			pp[1],
			pstate_data[0]->developer->list[j]
		);
	}
	/* reviewers */
	for (j = 0; j < pstate_data[0]->reviewer->length; ++j)
	{
		project_reviewer_add
		(
			pp[1],
			pstate_data[0]->reviewer->list[j]
		);
	}
	/* integrators */
	for (j = 0; j < pstate_data[0]->integrator->length; ++j)
	{
		project_integrator_add
		(
			pp[1],
			pstate_data[0]->integrator->list[j]
		);
	}
	pstate_data[1]->developer_may_review =
		pstate_data[0]->developer_may_review;
	pstate_data[1]->developer_may_integrate =
		pstate_data[0]->developer_may_integrate;
	pstate_data[1]->reviewer_may_integrate =
		pstate_data[0]->reviewer_may_integrate;
	pstate_data[1]->developers_may_create_changes =
		pstate_data[0]->developers_may_create_changes;

	if (pstate_data[0]->develop_end_notify_command)
		pstate_data[1]->develop_end_notify_command =
			str_copy(pstate_data[0]->develop_end_notify_command);
	if (pstate_data[0]->develop_end_undo_notify_command)
		pstate_data[1]->develop_end_undo_notify_command =
			str_copy(pstate_data[0]->develop_end_undo_notify_command);
	if (pstate_data[0]->review_pass_notify_command)
		pstate_data[1]->review_pass_notify_command =
			str_copy(pstate_data[0]->review_pass_notify_command);
	if (pstate_data[0]->review_pass_undo_notify_command)
		pstate_data[1]->review_pass_undo_notify_command =
			str_copy(pstate_data[0]->review_pass_undo_notify_command);
	if (pstate_data[0]->review_fail_notify_command)
		pstate_data[1]->review_fail_notify_command =
			str_copy(pstate_data[0]->review_fail_notify_command);
	if (pstate_data[0]->integrate_pass_notify_command)
		pstate_data[1]->integrate_pass_notify_command =
			str_copy(pstate_data[0]->integrate_pass_notify_command);
	if (pstate_data[0]->integrate_fail_notify_command)
		pstate_data[1]->integrate_fail_notify_command =
			str_copy(pstate_data[0]->integrate_fail_notify_command);
	if (pstate_data[0]->default_development_directory)
		pstate_data[1]->default_development_directory =
			str_copy(pstate_data[0]->default_development_directory);

	/*
	 * if no project directory was specified
	 * create the directory in their home directory.
	 */
	if (!home)
	{
		s2 = user_default_project_directory(pup);
		assert(s2);
		home = str_format("%S/%S", s2, project_name[1]);
		str_free(s2);
	}
	project_home_path_set(pp[1], home);
	project_verbose(pp[1], "project directory \"%S\"", home);
	str_free(home);

	/*
	 * create the diectory and subdirectories.
	 * It is an error if the directories can't be created.
	 *
	 * Don't use the project_baseline_path_get function,
	 * because it resolves any symlinks.
	 */
	s1 = project_home_path_get(pp[1]);
	bl = project_baseline_path_get(pp[1], 0);
	hp = project_history_path_get(pp[1]);
	ip = project_info_path_get(pp[1]);
	project_become(pp[1]);
	os_mkdir(s1, 02755);
	undo_rmdir_errok(s1);
	os_mkdir(bl, 02755);
	undo_rmdir_errok(bl);
	os_mkdir(hp, 02755);
	undo_rmdir_errok(hp);
	os_mkdir(ip, 02755);
	undo_rmdir_errok(ip);
	os_become_undo();

	/*
	 * add a row to the table
	 */
	gonzo_project_add(pp[1]);

	/*
	 * the first change adds all of the files
	 */
	cp = change_alloc(pp[1], 1);
	change_bind_new(cp);
	cstate_data = change_cstate_get(cp);
	cstate_data->brief_description =
		str_format("New release derived from %S.", project_name[0]);
	cstate_data->cause = change_cause_internal_enhancement;
	cstate_data->test_exempt = 1;
	cstate_data->test_baseline_exempt = 1;
	project_change_append(pp[1], 1);

	/*
	 * lots of fake history so we don't confuse
	 * anything later with otherwise illegal state transitions
	 */
	chp = change_history_new(cp, up);
	chp->what = cstate_history_what_new_change;
	chp = change_history_new(cp, up);
	chp->what = cstate_history_what_develop_begin;
	chp = change_history_new(cp, up);
	chp->what = cstate_history_what_develop_end;
	chp = change_history_new(cp, up);
	chp->what = cstate_history_what_review_pass;
	chp = change_history_new(cp, up);
	chp->what = cstate_history_what_integrate_begin;
	cstate_data->state = cstate_state_being_integrated;
	cstate_data->integration_directory = str_copy(bl);

	/*
	 * add all of the files to the change
	 */
	for (j = 0; j < pstate_data[0]->src->length; ++j)
	{
		pstate_src	p_src_data;
		pstate_src	p1_src_data;
		cstate_src	c_src_data;

		p_src_data = pstate_data[0]->src->list[j];
		if (p_src_data->deleted_by)
			continue;
		if (p_src_data->about_to_be_created_by)
			continue;

		p1_src_data = project_src_new(pp[1], p_src_data->file_name);
		p1_src_data->usage = p_src_data->usage;

		c_src_data = change_src_new(cp);
		c_src_data->file_name = str_copy(p_src_data->file_name);
		c_src_data->action = file_action_create;
		c_src_data->usage = p_src_data->usage;
	}

	/*
	 * copy files from old baseline to new baseline
	 */
	info.from = project_baseline_path_get(pp[0], 1);
	info.to = bl;
	project_verbose(pp[1], "copy baseline");
	project_become(pp[1]);
	dir_walk(info.from, copy_tree_callback, &info);
	os_become_undo();

	/*
	 * build history files
	 */
	if (!nolog)
	{
		user_ty	*pup;

		s1 = str_format("%S/%s.log", bl, option_progname_get());
		pup = project_user(pp[1]);
		log_open(s1, pup);
		user_free(pup);
		str_free(s1);
	}
	pconf_data = change_pconf_get(cp);
	for (j = 0; j < cstate_data->src->length; ++j)
	{
		cstate_src	c_src_data;
		pstate_src	p_src_data;

		c_src_data = cstate_data->src->list[j];
		p_src_data = project_src_find(pp[1], c_src_data->file_name);
		assert(p_src_data);

		/*
		 * create a new history file
		 */
		change_run_history_create_command(cp, c_src_data->file_name);
		p_src_data->edit_number =
			change_run_history_query_command
			(
				cp,
				c_src_data->file_name
			);
		c_src_data->edit_number = str_copy(p_src_data->edit_number);
	}

	/*
	 * some more history
	 */
	str_free(cstate_data->integration_directory);
	cstate_data->integration_directory = 0;
	chp = change_history_new(cp, up);
	chp->what = cstate_history_what_integrate_pass;
	php = project_history_new(pp[1]);
	php->delta_number = 1;
	php->change_number = 1;
	cstate_data->state = cstate_state_completed;
	cstate_data->delta_number = 1;

	/*
	 * write the project pointer back out
	 * release locks
	 */
	change_cstate_write(cp);
	project_pstate_write(pp[1]);
	gonzo_gstate_write();
	commit();
	lock_release();

	/*
	 * verbose success message
	 */
	project_verbose(pp[1], "created");
	str_free(project_name[0]);
	project_free(pp[0]);
	str_free(project_name[1]);
	project_free(pp[1]);
	change_free(cp);
	user_free(up);
	user_free(pup);
	trace((/*{*/"}\n"));
}


void
new_release()
{
	trace(("new_release()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		new_release_main();
		break;

	case arglex_token_help:
		new_release_help();
		break;

	case arglex_token_list:
		new_release_list();
		break;
	}
	trace((/*{*/"}\n"));
}

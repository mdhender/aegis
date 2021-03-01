/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1994, 1995 Peter Miller;
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
#include <ac/string.h>

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
	fprintf
	(
		stderr,
		"usage: %s -New_ReLeaSe <name> [ <option>... ]\n",
		progname
	);
	fprintf
	(
		stderr,
		"       %s -New_ReLeaSe -List [ <option>... ]\n",
		progname
	);
	fprintf(stderr, "       %s -New_ReLeaSe -Help\n", progname);
	quit(1);
}


static void new_release_help _((void));

static void
new_release_help()
{
	static char *text[] =
	{
#include <../man1/aenrls.h>
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


static void remove_suffix _((char *, char *, int *));

static void
remove_suffix(str, suf, punct)
	char		*str;
	char		*suf;
	int		*punct;
{
	size_t		str_len;
	size_t		suf_len;
	char		*cp;

	*punct = '.';
	str_len = strlen(str);
	suf_len = strlen(suf);
	if (str_len <= suf_len + 1)
		return;
	cp = str + str_len - suf_len - 1;
	if (ispunct(*cp) && !strcmp(cp + 1, suf))
	{
		*punct = (unsigned char)*cp;
		*cp = 0;
	}
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
	char		*tmp;
	char		suffix[20];
	int		min_sep;
	int		maj_sep;
	string_ty	*result;

	tmp = mem_copy_string(s->str_text);
	sprintf(suffix, "%ld", minor_old);
	remove_suffix(tmp, suffix, &min_sep);
	sprintf(suffix, "%ld", major_old);
	remove_suffix(tmp, suffix, &maj_sep);
	result =
		str_format
		(
			"%s%c%ld%c%ld",
			tmp,
			maj_sep,
			major_new,
			min_sep,
			minor_new
		);
	mem_free(tmp);
	return result;
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
	copy_tree_arg_ty *info;

	trace(("copy_tree_callback(arg = %08lX, message = %d, path = %08lX, \
st = %08lX)\n{\n"/*}*/, arg, message, path, st));
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
	size_t		j;
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
	copy_tree_arg_ty info;
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
				fatal
				(
					"duplicate %s option",
					arglex_value.alv_string
				);
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

	if (pstate_data[0]->copyright_years)
	{
		pstate_data[1]->copyright_years =
			pstate_copyright_years_list_type.alloc();
		for (j = 0; j < pstate_data[0]->copyright_years->length; ++j)
		{
			long		*year_p;
			type_ty		*type_p;

			year_p =
				pstate_copyright_years_list_type.list_parse
				(
					pstate_data[1]->copyright_years,
					&type_p
				);
			assert(type_p == &integer_type);
			*year_p = pstate_data[0]->copyright_years->list[j];
		}
	}

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
			str_copy
			(
				pstate_data[0]->develop_end_undo_notify_command
			);
	if (pstate_data[0]->review_pass_notify_command)
		pstate_data[1]->review_pass_notify_command =
			str_copy(pstate_data[0]->review_pass_notify_command);
	if (pstate_data[0]->review_pass_undo_notify_command)
		pstate_data[1]->review_pass_undo_notify_command =
			str_copy
			(
				pstate_data[0]->review_pass_undo_notify_command
			);
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
		int	max;

		s2 = user_default_project_directory(pup);
		assert(s2);
		os_become_orig();
		max = os_pathconf_name_max(s2);
		os_become_undo();
		if (project_name[1]->str_length > max)
		{
			fatal
			(
				"project name \"%S\" too long (by %ld)",
				project_name[1],
				project_name[1]->str_length - max
			);
		}
		home = str_format("%S/%S", s2, project_name[1]);
		str_free(s2);
		project_verbose(pp[1], "project directory \"%S\"", home);
	}
	project_home_path_set(pp[1], home);
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
		user_ty	*pup1;

		s1 = str_format("%S/%s.log", bl, option_progname_get());
		pup1 = project_user(pp[1]);
		log_open(s1, pup1, log_style_create);
		user_free(pup1);
		str_free(s1);
	}
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

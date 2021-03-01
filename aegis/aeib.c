/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998 Peter Miller;
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
 * MANIFEST: functions for implementing integrate begin
 */

#include <stdio.h>
#include <ac/stdlib.h>
#include <ac/string.h>
#include <ac/time.h>
#include <ac/unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <aeib.h>
#include <ael.h>
#include <arglex2.h>
#include <commit.h>
#include <change.h>
#include <change_bran.h>
#include <change_file.h>
#include <dir.h>
#include <error.h>
#include <file.h>
#include <help.h>
#include <lock.h>
#include <log.h>
#include <progname.h>
#include <os.h>
#include <project.h>
#include <project_file.h>
#include <project_hist.h>
#include <sub.h>
#include <trace.h>
#include <undo.h>
#include <user.h>


static void integrate_begin_usage _((void));

static void
integrate_begin_usage()
{
	char		*progname;

	progname = progname_get();
	fprintf(stderr, "usage: %s -Integrate_Begin [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Integrate_Begin -List [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Integrate_Begin -Help\n", progname);
	quit(1);
}


static void integrate_begin_help _((void));

static void
integrate_begin_help()
{
	help("aeib", integrate_begin_usage);
}


static void integrate_begin_list _((void));

static void
integrate_begin_list()
{
	string_ty	*project_name;

	trace(("integrate_begin_list()\n{\n"/*}*/));
	arglex();
	project_name = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(integrate_begin_usage);
			continue;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				option_needs_name(arglex_token_project, integrate_begin_usage);
			if (project_name)
				duplicate_option_by_name(arglex_token_project, integrate_begin_usage);
			project_name = str_from_c(arglex_value.alv_string);
			break;
		}
		arglex();
	}
	list_changes_in_state_mask
	(
		project_name,
		1 << cstate_state_awaiting_integration
	);
	if (project_name)
		str_free(project_name);
	trace((/*{*/"}\n"));
}


static string_ty *remove_comma_d_if_present _((string_ty *));

static string_ty *
remove_comma_d_if_present(s)
	string_ty	*s;
{
	char		*cp;

	if (s->str_length < 2)
		return str_copy(s);
	cp = s->str_text + s->str_length - 2;
	if (cp[0] == ',' && cp[1] == 'D')
		return str_n_from_c(s->str_text, s->str_length - 2);
	return str_copy(s);
}


static void link_tree_callback_minimum _((void *, dir_walk_message_ty,
	string_ty *, struct stat *));

static void
link_tree_callback_minimum(arg, message, path, st)
	void		*arg;
	dir_walk_message_ty message;
	string_ty	*path;
	struct stat	*st;
{
	string_ty	*s1;
	string_ty	*s1short;
	string_ty	*s2;
	change_ty	*cp;
	fstate_src	src;

	trace(("link_tree_callback_minimum(message = %d, path = %08lX, \
st = %08lX)\n{\n"/*}*/, message, path, st));
	os_interrupt_cope();
	cp = (change_ty *)arg;
	assert(cp);
	trace_string(path->str_text);
	s1 = os_below_dir(project_baseline_path_get(cp->pp, 1), path);
	assert(s1);
	trace_string(s1->str_text);
	if (!s1->str_length)
		s2 = str_copy(change_integration_directory_get(cp, 1));
	else
		s2 = str_format("%S/%S", change_integration_directory_get(cp, 1), s1);
	trace_string(s2->str_text);
	switch (message)
	{
	case dir_walk_dir_before:
		if (!s1->str_length)
		{
			assert(!os_exists(s2));
			os_mkdir(s2, 02755);
		}
		break;

	case dir_walk_file:
		if (st->st_mode & 07000)
		{
			/*
			 * Don't link files with horrible modes.
			 * They shouldn't be source, anyway.
			 */
			if (!project_file_find(cp->pp, s1))
				break;
		}

		/*
		 * Remove the ,D suffix, if present, and use the
		 * shortened from to test for project and change
		 * membership.  This ensures that diff files are also
		 * copied across.  Note that deleted files *have*
		 * difference files.
		 */
		s1short = remove_comma_d_if_present(s1);

		/*
		 * Don't link it if it's not a source file, or a
		 * relevant ,D file.
		 */
		src = project_file_find(cp->pp, s1short);
		if
		(
			!src
		||
			(src->deleted_by && str_equal(s1, s1short))
		||
			src->about_to_be_created_by
		)
		{
			str_free(s1short);
			break;
		}

		/*
		 * make sure the directory is there
		 */
		os_mkdir_between
		(
			change_integration_directory_get(cp, 1),
			s1,
			02755
		);

		/*
		 * Don't link a file (or the corresponding ,D file) if
		 * the file is in the change.
		 */
		if (change_file_find(cp, s1short))
		{
			str_free(s1short);
			break;
		}
		str_free(s1short);

		/*
		 * link the file and make sure it is a suitable mode
		 */
		trace(("ln %s %s\n", path->str_text, s2->str_text));
		os_link(path, s2);
		if (!project_file_find(cp->pp, s1))
			os_chmod(s2, (st->st_mode | 0644) & ~0022 & ~change_umask(cp));
		else
			os_chmod(s2, (st->st_mode | 0444) & ~0222 & ~change_umask(cp));

		/*
		 * Update the modify time of the linked file.  On a
		 * fully-functional Unix, this is unnecessary, because a
		 * hard link alters the ctime, not the mtime, and this
		 * is a no-op.  Solaris, on the other hand, is brain dead.
		 */
		os_mtime_set(s2, st->st_mtime);
		break;

	case dir_walk_dir_after:
		break;

	case dir_walk_special:
	case dir_walk_symlink:
		/*
		 * ignore special files
		 *
		 * They could never be source files, so they must be
		 * created by the build.  These ones must always be
		 * created at build time, that's all.
		 */
		break;
	}
	str_free(s2);
	str_free(s1);
	trace((/*{*/"}\n"));
}


static void link_tree_callback _((void *, dir_walk_message_ty, string_ty *,
	struct stat *));

static void
link_tree_callback(arg, message, path, st)
	void		*arg;
	dir_walk_message_ty message;
	string_ty	*path;
	struct stat	*st;
{
	string_ty	*s1;
	string_ty	*s1short;
	string_ty	*s2;
	change_ty	*cp;
	fstate_src	src;
	string_ty	*contents;

	trace(("link_tree_callback(message = %d, path = %08lX, \
st = %08lX)\n{\n"/*}*/, message, path, st));
	os_interrupt_cope();
	cp = (change_ty *)arg;
	assert(cp);
	trace_string(path->str_text);
	s1 = os_below_dir(project_baseline_path_get(cp->pp, 1), path);
	assert(s1);
	trace_string(s1->str_text);
	if (!s1->str_length)
		s2 = str_copy(change_integration_directory_get(cp, 1));
	else
		s2 = str_format("%S/%S", change_integration_directory_get(cp, 1), s1);
	trace_string(s2->str_text);
	switch (message)
	{
	case dir_walk_dir_before:
		assert(!os_exists(s2));
		os_mkdir(s2, 02755);
		break;

	case dir_walk_file:
		src = project_file_find(cp->pp, s1);
		if (st->st_mode & 07000)
		{
			/*
			 * Don't link files with horrible modes.
			 * They shouldn't be source, anyway.
			 */
			if (!src)
				break;
		}

		/*
		 * Don't link a file (or the corresponding ,D file) if
		 * the file is in the change.
		 */
		s1short = remove_comma_d_if_present(s1);
		if (change_file_find(cp, s1short))
		{
			str_free(s1short);
			break;
		}
		str_free(s1short);

		/*
		 * link the file and make sure it is a suitable mode
		 */
		os_link(path, s2);
		if (!src)
			os_chmod(s2, (st->st_mode | 0644) & ~0022 & ~change_umask(cp));
		else
			os_chmod(s2, (st->st_mode | 0444) & ~0222 & ~change_umask(cp));

		/*
		 * Update the modify time of the linked file.  On a
		 * fully-functional Unix, this is unnecessary, because a
		 * hard link alters the ctime, not the mtime, and this
		 * is a no-op.  Solaris, on the other hand, is brain dead.
		 */
		os_mtime_set(s2, st->st_mtime);
		break;

	case dir_walk_dir_after:
		break;

	case dir_walk_symlink:
		contents = os_readlink(path);
		os_symlink(contents, s2);
		str_free(contents);
		break;

	case dir_walk_special:
		/*
		 * ignore special files
		 *
		 * They could never be source files, so they must be
		 * created by the build.  These ones must always be
		 * created at build time, that's all.
		 */
		break;
	}
	str_free(s2);
	str_free(s1);
	trace((/*{*/"}\n"));
}


static void copy_tree_callback_minimum _((void *, dir_walk_message_ty,
	string_ty *, struct stat *));

static void
copy_tree_callback_minimum(arg, message, path, st)
	void		*arg;
	dir_walk_message_ty message;
	string_ty	*path;
	struct stat	*st;
{
	string_ty	*s1;
	string_ty	*s1short;
	string_ty	*s2;
	change_ty	*cp;
	fstate_src	src;
	int		uid;

	trace(("copy_tree_callback_minimum(message = %d, path = %08lX, \
st = %08lX)\n{\n"/*}*/, message, path, st));
	os_interrupt_cope();
	cp = (change_ty *)arg;
	assert(cp);
	trace_string(path->str_text);
	s1 = os_below_dir(project_baseline_path_get(cp->pp, 1), path);
	assert(s1);
	trace_string(s1->str_text);
	if (!s1->str_length)
		s2 = str_copy(change_integration_directory_get(cp, 1));
	else
	{
		s2 =
			str_format
			(
				"%S/%S",
				change_integration_directory_get(cp, 1),
				s1
			);
	}
	trace_string(s2->str_text);
	switch (message)
	{
	case dir_walk_dir_before:
		if (!s1->str_length)
		{
			assert(!os_exists(s2));
			os_mkdir(s2, 02755);
		}
		break;

	case dir_walk_file:
		/*
		 * Don't copy files which don't belong to us.
		 * Don't copy files with horrible modes.
		 * They shouldn't be source, anyway.
		 */
		os_become_query(&uid, (int *)0, (int *)0);
		if ((st->st_uid != uid) || (st->st_mode & 07000))
		{
			if (!project_file_find(cp->pp, s1))
				break;
		}

		/*
		 * Remove the ,D suffix, if present, and use the
		 * shortened from to test for project and change
		 * membership.  This ensures that diff files are also
		 * copied across.  Note that deleted files *have*
		 * difference files.
		 */
		s1short = remove_comma_d_if_present(s1);
		trace_string(s1short->str_text);

		/*
		 * Don't copy it if it's not a source file, or a
		 * relevant ,D file.
		 */
		src = project_file_find(cp->pp, s1short);
		if
		(
			!src
		||
			(src->deleted_by && str_equal(s1, s1short))
		||
			src->about_to_be_created_by
		)
		{
			str_free(s1short);
			break;
		}

		/*
		 * make sure the directory is there
		 */
		os_mkdir_between
		(
			change_integration_directory_get(cp, 1),
			s1,
			02755
		);

		/*
		 * Don't copy a file (or the corresponding ,D file) if
		 * the file is in the change.
		 */
		if (change_file_find(cp, s1short))
		{
			str_free(s1short);
			break;
		}
		str_free(s1short);

		/*
		 * copy the file
		 */
		trace(("cp %s %s\n", path->str_text, s2->str_text));
		copy_whole_file(path, s2, 1);
		if (!project_file_find(cp->pp, s1))
			os_chmod(s2, (st->st_mode | 0644) & ~0022 & ~change_umask(cp));
		else
			os_chmod(s2, (st->st_mode | 0444) & ~0222 & ~change_umask(cp));
		break;

	case dir_walk_dir_after:
		break;

	case dir_walk_special:
	case dir_walk_symlink:
		/*
		 * ignore special files
		 *
		 * They could never be source files, so they must be
		 * created by the build.  These ones must always be
		 * created at build time, that's all.
		 */
		break;
	}
	str_free(s2);
	str_free(s1);
	trace((/*{*/"}\n"));
}


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
	string_ty	*s1short;
	string_ty	*s2;
	change_ty	*cp;
	int		uid;
	string_ty	*contents;

	trace(("copy_tree_callback(message = %d, path = %08lX, st = %08lX)\n{\n"/*}*/, message, path, st));
	os_interrupt_cope();
	cp = (change_ty *)arg;
	assert(cp);
	trace_string(path->str_text);
	s1 = os_below_dir(project_baseline_path_get(cp->pp, 1), path);
	assert(s1);
	trace_string(s1->str_text);
	if (!s1->str_length)
		s2 = str_copy(change_integration_directory_get(cp, 1));
	else
	{
		s2 =
			str_format
			(
				"%S/%S",
				change_integration_directory_get(cp, 1),
				s1
			);
	}
	trace_string(s2->str_text);
	switch (message)
	{
	case dir_walk_dir_before:
		assert(!os_exists(s2));
		os_mkdir(s2, 02755);
		break;

	case dir_walk_file:
		/*
		 * Don't copy files which don't belong to us.
		 * Don't copy files with horrible modes.
		 * They shouldn't be source, anyway.
		 */
		os_become_query(&uid, (int *)0, (int *)0);
		if ((st->st_uid != uid) || (st->st_mode & 07000))
		{
			if (!project_file_find(cp->pp, s1))
				break;
		}

		/*
		 * Remove the ,D suffix, if present, and use the
		 * shortened from to test for project and change
		 * membership.  This ensures that diff files are also
		 * copied across.  Note that deleted files *have*
		 * difference files.
		 */
		s1short = remove_comma_d_if_present(s1);
		trace_string(s1short->str_text);

		/*
		 * Don't copy a file (or the corresponding ,D file) if
		 * the file is in the change.
		 */
		if (change_file_find(cp, s1short))
		{
			str_free(s1short);
			break;
		}
		str_free(s1short);

		/*
		 * copy the file
		 */
		copy_whole_file(path, s2, 1);
		if (!project_file_find(cp->pp, s1))
			os_chmod(s2, (st->st_mode | 0644) & ~0022 & ~change_umask(cp));
		else
			os_chmod(s2, (st->st_mode | 0444) & ~0222 & ~change_umask(cp));
		break;

	case dir_walk_dir_after:
		break;

	case dir_walk_symlink:
		contents = os_readlink(path);
		os_symlink(contents, s2);
		str_free(contents);
		break;

	case dir_walk_special:
		/*
		 * ignore special files
		 *
		 * They could never be source files, so they must be
		 * created by the build.  These ones must always be
		 * created at build time, that's all.
		 */
		break;
	}
	str_free(s2);
	str_free(s1);
	trace((/*{*/"}\n"));
}


static void integrate_begin_main _((void));

static void
integrate_begin_main()
{
	string_ty	*bl;
	string_ty	*dd;
	string_ty	*id;
	pconf		pconf_data;
	cstate		cstate_data;
	int		j;
	cstate_history	history_data;
	int		minimum;
	int		maximum;
	string_ty	*project_name;
	project_ty	*pp;
	long		change_number;
	change_ty	*cp;
	user_ty		*up;
	user_ty		*pup;
	int		errs;
	string_ty	*s;
	long		other;
	log_style_ty	log_style;
	string_ty	*base;
	int		base_max;
	string_ty	*num;

	trace(("integrate_begin_main()\n{\n"/*}*/));
	minimum = 0;
	maximum = 0;
	project_name = 0;
	change_number = 0;
	log_style = log_style_create_default;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(integrate_begin_usage);
			continue;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				option_needs_number(arglex_token_change, integrate_begin_usage);
			/* fall through... */

		case arglex_token_number:
			if (change_number)
				duplicate_option_by_name(arglex_token_change, integrate_begin_usage);
			change_number = arglex_value.alv_number;
			if (change_number == 0)
				change_number = MAGIC_ZERO;
			else if (change_number < 1)
			{
				sub_context_ty	*scp;

				scp = sub_context_new();
				sub_var_set(scp, "Number", "%ld", change_number);
				fatal_intl(scp, i18n("change $number out of range"));
				/* NOTREACHED */
				sub_context_delete(scp);
			}
			break;

		case arglex_token_minimum:
			if (minimum)
				duplicate_option(integrate_begin_usage);
			minimum = 1;
			break;

		case arglex_token_maximum:
			if (maximum)
				duplicate_option(integrate_begin_usage);
			maximum = 1;
			break;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				option_needs_name(arglex_token_project, integrate_begin_usage);
			/* fall through... */

		case arglex_token_string:
			if (project_name)
				duplicate_option_by_name(arglex_token_project, integrate_begin_usage);
			project_name = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_nolog:
			if (log_style == log_style_none)
				duplicate_option(integrate_begin_usage);
			log_style = log_style_none;
			break;

		case arglex_token_wait:
		case arglex_token_wait_not:
			user_lock_wait_argument(integrate_begin_usage);
			break;
		}
		arglex();
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
	trace_long(change_number);
	cp = change_alloc(pp, change_number);
	change_bind_existing(cp);

	/*
	 * lock the project, the change and the user
	 */
	project_pstate_lock_prepare(pp);
	change_cstate_lock_prepare(cp);
	user_ustate_lock_prepare(up);
	lock_take();
	cstate_data = change_cstate_get(cp);
	pconf_data = change_pconf_get(cp, 1);

	/*
	 * make sure they are allowed to
	 */
	if (!project_integrator_query(pp, user_name(up)))
		project_fatal(pp, 0, i18n("not an integrator"));
	if (cstate_data->state != cstate_state_awaiting_integration)
		change_fatal(cp, 0, i18n("bad ib state"));

	/*
	 * make sure only one integration at a time
	 * for each project
	 */
	other = project_current_integration_get(pp);
	if (other)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "Number", "%ld", magic_zero_decode(other));
		project_fatal(pp, scp, i18n("currently integrating $number"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}
	trace_long(change_number);
	project_current_integration_set(pp, change_number);

	/*
	 * grab a delta number
	 * and advance the project's delta counter
	 */
	cstate_data->delta_number = project_delta_number_get(pp);

	/*
	 * include the current year in the copyright_years field
	 */
	change_copyright_years_now(cp);

	/*
	 * Create the integration directory.
	 */
	base = str_format("delta%d", getpid());
	num = str_format(".%3.3ld", cstate_data->delta_number);
	os_become_orig();
	base_max = os_pathconf_name_max(project_top_path_get(pp, 0));
	os_become_undo();
	base_max -= num->str_length;
	if (base_max < 5)
		base_max = 5;
	s =
		str_format
		(
			"%S/%.*S%S",
			project_top_path_get(pp, 0),
			base_max,
			base,
			num
		);
	str_free(base);
	str_free(num);
	change_integration_directory_set(cp, s);
	str_free(s);

	/*
	 * There will be many files in the baseline in addition to the
	 * sources files.
	 *
	 * If any files are being deleted, only copy the source files
	 * from the baseline to the integration directory.  This way
	 * the additional files relating to the removed sources file
	 * are also removed (eg remove a .c file and you need to get
	 * rid of the .o file).  It also shows when dependencies have
	 * become out-of-date.
	 *
	 * It is possible to ask for this from the command line, too.
	 */
	if (maximum && minimum)
	{
		mutually_exclusive_options
		(
			arglex_token_minimum,
			arglex_token_maximum,
			integrate_begin_usage
		);
	}
	if (!maximum && !minimum)
	{
		for (j = 0; ; ++j)
		{
			fstate_src	src_data;
	
			src_data = change_file_nth(cp, j);
			if (!src_data)
				break;
			if (src_data->action == file_action_remove)
			{
				minimum = 1;
				break;
			}
		}
	}

	/*
	 * before creating the integration directory,
	 * make sure none of the change files have been tampered with.
	 *
	 * (a) the project owner must have access
	 * (b) the changed file must exist and not have been modified
	 * (c) the difference file must exist and not have been modified
	 */
	if (cstate_data->branch)
		dd = str_format("%S/baseline", change_development_directory_get(cp, 0));
	else
		dd = change_development_directory_get(cp, 1);
	id = change_integration_directory_get(cp, 1);
	bl = project_baseline_path_get(pp, 1);
	project_file_nth(pp, 0); /* make sure proj fstate read in */
	project_become(pp);
	errs = 0;
	for (j = 0; ; ++j)
	{
		fstate_src	src_data;
		string_ty	*s1;
		string_ty	*s2;

		src_data = change_file_nth(cp, j);
		if (!src_data)
			break;
		if (src_data->usage == file_usage_build)
			continue;
		if (src_data->action == file_action_remove)
			continue;

		s1 = change_file_path(cp, src_data->file_name);
		if (!change_fingerprint_same(src_data->file_fp, s1))
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "File_Name", "%S", src_data->file_name);
			change_error(cp, scp, i18n("$filename altered"));
			sub_context_delete(scp);
			errs++;
		}

		s2 = str_format("%S,D", s1);
		if (!change_fingerprint_same(src_data->diff_file_fp, s2))
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "File_Name", "%S,D", src_data->file_name);
			change_error(cp, scp, i18n("$filename altered"));
			sub_context_delete(scp);
			errs++;
		}
		str_free(s1);
		str_free(s2);
	}
	if (errs)
		quit(1);

	/*
	 * add to the change history
	 *	(The time stamp is used later for the file mod times.)
	 */
	os_throttle();
	history_data = change_history_new(cp, up);
	history_data->what = cstate_history_what_integrate_begin;

	/*
	 * Make sure the integration directory is not already there
	 * (a user make have killed a previous aeib).  Register a rmdir
	 * (to be done in the background) if an undo is needed.
	 */
	if (os_exists(id))
		os_rmdir_tree(id);
	undo_rmdir_bg(id);

	/*
	 * create the integration directory
	 * copy everything from baseline to integration directory
	 * except things from the change
	 * and change owner to the project's owner.
	 */
	if (pconf_data->link_integration_directory)
	{
		change_verbose(cp, 0, i18n("link baseline to integration directory"));
		dir_walk
		(
			bl,
			(
				minimum
			?
				link_tree_callback_minimum
			:
				link_tree_callback
			),
			cp
		);
	}
	else
	{
		change_verbose(cp, 0, i18n("copy baseline to integration directory"));
		dir_walk
		(
			bl,
			(
				minimum
			?
				copy_tree_callback_minimum
			:
				copy_tree_callback
			),
			cp
		);
	}

	/*
	 * apply the changes to the integration directory
	 */
	change_verbose(cp, 0, i18n("apply change to integration directory"));
	for (j = 0; ; ++j)
	{
		fstate_src	src_data;
		string_ty	*s1;
		string_ty	*s2;

		src_data = change_file_nth(cp, j);
		if (!src_data)
			break;
		s1 = str_format("%S/%S", dd, src_data->file_name);
		s2 = str_format("%S/%S", id, src_data->file_name);
		if (os_exists(s2))
		{
			/*
			 * this is defensive,
			 * and should never need to be executed
			 */
			os_unlink(s2);
		}
		switch (src_data->action)
		{
		case file_action_insulate:
			/*
			 * This should never happen: aede will fail if
			 * there are any insulation files.
			 */
			assert(0);
			break;

		case file_action_remove:
			break;

		case file_action_modify:
		case file_action_create:
			if (src_data->usage == file_usage_build)
				break;
			/*
			 * New files do not exist in the baseline,
			 * and old files may not be copied under -MINimum,
			 * so we may need to create directories.
			 */
			os_mkdir_between(id, src_data->file_name, 02755);
			copy_whole_file(s1, s2, 0);
			os_chmod(s2, 0444 & ~change_umask(cp));

			/*
			 * Make all of the change's files have the same
			 * mod time, so that when aeipass flattens the
			 * mod times, all of them fall into a single
			 * second, minimizing the chance that mod times
			 * will extend into the future after aeipass.
			 *
			 * This also helps cooperating DMTs flatten the
			 * targets' mod times into a smaller range,
			 * which also helps aeipass.
			 */
			os_mtime_set(s2, history_data->when);
			break;
		}
		str_free(s1);
		str_free(s2);

		/*
		 * clear the file's test times
		 */
		if (src_data->architecture_times)
		{
			fstate_src_architecture_times_list_type.free
			(
				src_data->architecture_times
			);
			src_data->architecture_times = 0;
		}

		/*
		 * clear the file's integrate difference time
		 */
		if (src_data->idiff_file_fp)
		{
			fingerprint_type.free(src_data->idiff_file_fp);
			src_data->idiff_file_fp = 0;
		}
	}
	project_become_undo();

	/*
	 * add the change to the user's list
	 */
	trace_long(change_number);
	user_own_add(up, project_name_get(pp), change_number);
	cstate_data->state = cstate_state_being_integrated;

	/*
	 * clear the change build times,
	 * and test times
	 */
	change_build_times_clear(cp);

	/*
	 * write the data out
	 * and release the locks
	 */
	project_pstate_write(pp);
	change_cstate_write(cp);
	user_ustate_write(up);
	commit();
	lock_release();

	/*
	 * run the integrate begin command
	 */
	pup = project_user(pp);
	log_open(change_logfile_get(cp), pup, log_style);
	user_free(pup);
	change_run_integrate_begin_command(cp);

	/*
	 * verbose success message
	 */
	change_verbose(cp, 0, i18n("integrate begin complete"));
	change_free(cp);
	project_free(pp);
	user_free(up);
	trace((/*{*/"}\n"));
}


void
integrate_begin()
{
	trace(("integrate_begin()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		integrate_begin_main();
		break;

	case arglex_token_help:
		integrate_begin_help();
		break;

	case arglex_token_list:
		integrate_begin_list();
		break;
	}
	trace((/*{*/"}\n"));
}

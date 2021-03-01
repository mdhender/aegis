/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999 Peter Miller;
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
 * MANIFEST: functions for implementing integrate pass
 */

#include <ac/stdio.h>
#include <ac/stdlib.h>
#include <ac/string.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <aeip.h>
#include <ael.h>
#include <arglex2.h>
#include <commit.h>
#include <change_bran.h>
#include <change_file.h>
#include <dir.h>
#include <error.h>
#include <file.h>
#include <help.h>
#include <lock.h>
#include <log.h>
#include <metrics.h>
#include <mem.h>
#include <progname.h>
#include <os.h>
#include <project.h>
#include <project_file.h>
#include <project_hist.h>
#include <str_list.h>
#include <sub.h>
#include <trace.h>
#include <undo.h>
#include <user.h>

#define MTIME_BLURB 0

typedef struct time_map_ty time_map_ty;
struct time_map_ty
{
	time_t		old;
	time_t		new;
};

typedef struct time_map_list_ty time_map_list_ty;
struct time_map_list_ty
{
	time_map_ty	*list;
	size_t		len;
	size_t		max;
	time_t		time_aeib;
	time_t		time_aeip;
};


static void integrate_pass_usage _((void));

static void
integrate_pass_usage()
{
	char		*progname;

	progname = progname_get();
	fprintf
	(
		stderr,
		"usage: %s -Integrate_PASS [ <option>... ]\n",
		progname
	);
	fprintf
	(
		stderr,
		"       %s -Integrate_PASS -List [ <option>... ]\n",
		progname
	);
	fprintf(stderr, "       %s -Integrate_PASS -Help\n", progname);
	quit(1);
}


static void integrate_pass_help _((void));

static void
integrate_pass_help()
{
	help("aeip", integrate_pass_usage);
}


static void integrate_pass_list _((void (*)(void)));

static void
integrate_pass_list(usage)
	void		(*usage)_((void));
{
	string_ty	*project_name;

	trace(("integrate_pass_list()\n{\n"/*}*/));
	arglex();
	project_name = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(usage);
			continue;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				option_needs_name(arglex_token_project, usage);
			if (project_name)
				duplicate_option_by_name(arglex_token_project, usage);
			project_name = str_from_c(arglex_value.alv_string);
			break;
		}
		arglex();
	}
	list_changes_in_state_mask
	(
		project_name,
		1 << cstate_state_being_integrated
	);
	if (project_name)
		str_free(project_name);
	trace((/*{*/"}\n"));
}


static void time_map_get _((void *, dir_walk_message_ty, string_ty *,
	struct stat *));

static void
time_map_get(p, message, path, st)
	void		*p;
	dir_walk_message_ty message;
	string_ty	*path;
	struct stat	*st;
{
	time_map_list_ty *tlp;
	time_map_ty	*tp;
	time_t		t;
	long		min;
	long		max;
	long		j;

	/*
	 * If it's not a file, ignore it.
	 * If its mtime is before aeib, ignore it. 
	 */
	if (message != dir_walk_file)
		return;
	tlp = p;
	t = st->st_mtime;
	if (t < tlp->time_aeib)
		return;

	/*
	 * Find the time in the list using a binary chop.
	 * If it is already there, do nothing.
	 */
	min = 0;
	max = (long)tlp->len - 1;
	while (min <= max)
	{
		long	mid;
		time_t	mid_t;

		mid = (min + max) / 2;
		mid_t = tlp->list[mid].old;
		if (mid_t == t)
			return;
		if (mid_t < t)
			min = mid + 1;
		else
			max = mid - 1;
	}

	/*
	 * This is a new time, insert it into the list sorted by time.
	 */
	if (tlp->len >= tlp->max)
	{
		tlp->max += 100;
		tlp->list =
			mem_change_size
			(
				tlp->list,
				tlp->max * sizeof(tlp->list[0])
			);
	}
	for (j = tlp->len; j > min; --j)
		tlp->list[j] = tlp->list[j - 1];
	tlp->len++;
	tp = &tlp->list[min];
	tp->old = st->st_mtime;
	tp->new = st->st_mtime;
}


static void time_map_set _((void *, dir_walk_message_ty, string_ty *,
	struct stat *));

static void
time_map_set(p, message, path, st)
	void		*p;
	dir_walk_message_ty message;
	string_ty	*path;
	struct stat	*st;
{
	time_map_list_ty *tlp;
	time_t		t;
	long		min;
	long		max;

	/*
	 * If it's not a file, ignore it.
	 * If its mtime is before aeib, ignore it. 
	 */
	if (message != dir_walk_file)
		return;
	tlp = p;
	t = st->st_mtime;
	if (t < tlp->time_aeib)
		return;

	/*
	 * Find the time in the list using a binary chop.
	 */
	min = 0;
	max = (long)tlp->len - 1;
	while (min <= max)
	{
		long	mid;
		time_t	mid_t;

		mid = (min + max) / 2;
		mid_t = tlp->list[mid].old;
		if (mid_t == t)
		{
			min = mid;
			max = mid;
			break;
		}
		if (mid_t < t)
			min = mid + 1;
		else
			max = mid - 1;
	}

	/*
	 * If the file time has been altered since aeipass began (and
	 * thus is not in the list), which hopefully is *very* rare, as
	 * it requires direct human interference, use a close time.
	 */
	if (min >= tlp->len)
		min = tlp->len - 1;

	/*
	 * set the file time
	 */
	os_mtime_set_errok(path, tlp->list[min].new);
}


static metric metric_copy _((metric));

static metric
metric_copy(mp)
	metric		mp;
{
	metric		result;

	result = metric_type.alloc();
	if (mp->name)
		result->name = str_copy(mp->name);
	if (mp->mask & metric_value_mask)
	{
		result->value = mp->value;
		result->mask = metric_value_mask;
	}
	return result;
}


static metric_list metric_list_copy _((metric_list));

static metric_list
metric_list_copy(mlp)
	metric_list	mlp;
{
	metric_list	result;
	size_t		j;

	if (!mlp)
		return 0;
	result = metric_list_type.alloc();
	for (j = 0; j < mlp->length; ++j)
	{
		metric		mp;
		metric		*mpp;
		type_ty		*bogus;

		mp = mlp->list[j];
		mpp = metric_list_type.list_parse(result, &bogus);
		*mpp = metric_copy(mp);
	}
	return result;
}


static void integrate_pass_main _((void));

static void
integrate_pass_main()
{
	time_t		mtime;
	time_t		youngest;
	string_ty	*hp;
	string_ty	*id;
	string_ty	*cwd;
	cstate		cstate_data;
	string_ty	*old_baseline;
	string_ty	*new_baseline;
	string_ty	*dev_dir;
	string_ty	*int_name;
	string_ty	*rev_name;
	string_ty	*dev_name;
	cstate_history	history_data;
	int		j, k;
	int		ncmds;
	string_ty	*project_name;
	project_ty	*pp;
	long		change_number;
	change_ty	*cp;
	log_style_ty	log_style;
	user_ty		*up;
	user_ty		*devup;
	user_ty		*pup;
	int		nerr;
	int		diff_whine;
	change_ty	*pcp;
	cstate		p_cstate_data;
	time_map_list_ty tml;
	time_t		time_final;
	string_list_ty	trashed;

	trace(("integrate_pass_main()\n{\n"/*}*/));
	project_name = 0;
	change_number = 0;
	log_style = log_style_append_default;
	nerr = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(integrate_pass_usage);
			continue;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				option_needs_number(arglex_token_change, integrate_pass_usage);
			/* fall through... */

		case arglex_token_number:
			if (change_number)
				duplicate_option_by_name(arglex_token_change, integrate_pass_usage);
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

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				option_needs_name(arglex_token_project, integrate_pass_usage);
			/* fall through... */
	
		case arglex_token_string:
			if (project_name)
				duplicate_option_by_name(arglex_token_project, integrate_pass_usage);
			project_name = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_nolog:
			if (log_style == log_style_none)
				duplicate_option(integrate_pass_usage);
			log_style = log_style_none;
			break;

		case arglex_token_wait:
		case arglex_token_wait_not:
			user_lock_wait_argument(integrate_pass_usage);
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
	pup = project_user(pp);

	/*
	 * locate change data
	 */
	if (!change_number)
		change_number = user_default_change(up);
	cp = change_alloc(pp, change_number);
	change_bind_existing(cp);

	/*
	 * Take an advisory write lock on the appropriate row of the change
	 * table.  Take an advisory write lock on the appropriate row of the
	 * user table.  Block until can get both simultaneously.
	 *
	 * Also take a write lock on the baseline, to ensure that it does
	 * not change for the duration of builds (aeb) or file copies (aecp).
	 * And a lock on the history files, so that no aeip can trash
	 * another's history files.
	 */
	project_pstate_lock_prepare(pp);
	change_cstate_lock_prepare(cp);
	user_ustate_lock_prepare(up);
	project_baseline_write_lock_prepare(pp);
	project_history_lock_prepare(pp);
	lock_take();
	cstate_data = change_cstate_get(cp);

	/*
	 * It is an error if the change is not in the being_integrated state.
	 * It is an error if the change is not assigned to the current user.
	 */
	if (cstate_data->state != cstate_state_being_integrated)
		change_fatal(cp, 0, i18n("bad ip state"));
	if (!str_equal(change_integrator_name(cp), user_name(up)))
		change_fatal(cp, 0, i18n("not integrator"));
	nerr = 0;
	youngest = 0;
	os_throttle();

	/*
	 * add to history for state change
	 */
	history_data = change_history_new(cp, up);
	history_data->what = cstate_history_what_integrate_pass;
	tml.time_aeip = history_data->when;
#if MTIME_BLURB
	error_raw("lock taken %.24s", ctime(&tml.time_aeip));
#endif

	/*
	 * add to project history
	 */
	project_history_new(pp, cstate_data->delta_number, change_number);

	/*
	 * Find the most recent aeib in the history, and remember when
	 * that was, so we can leave file mtimes alone when they fall
	 * before that point.
	 */
	assert(cstate_data->history);
	assert(cstate_data->history->length);
	tml.time_aeib = 0;
	for (j = 0; j < cstate_data->history->length; ++j)
	{
		cstate_history	hhp;

		hhp = cstate_data->history->list[j];
		if (hhp->what == cstate_history_what_integrate_begin)
			tml.time_aeib = hhp->when;
	}
	assert(tml.time_aeib);

	id = change_integration_directory_get(cp, 1);

	/*
	 * Walk the change files, making sure
	 *	1. the change has been diffed (except for the lowest b.l.)
	 *	2. parent files are copied into the change
	 *	3. test times are transferred
	 */
	pcp = project_change_get(pp);
	diff_whine = 0;
	for (j = 0; ; ++j)
	{
		fstate_src	c_src_data;
		fstate_src	p_src_data;
		int		transfer_architecture_times;
		int		transfer_file_times;
		int		transfer_diff_file_times;

		c_src_data = change_file_nth(cp, j);
		if (!c_src_data)
			break;
		trace(("file_name = \"%s\"\n", c_src_data->file_name->str_text));

		/*
		 * check the the file has been differenced
		 */
		if
		(
			pp->parent
		&&
			c_src_data->usage != file_usage_build
		&&
			!(
				c_src_data->action == file_action_remove
			&&
				c_src_data->move
			&&
				change_file_find(cp, c_src_data->move)
			)
		&&
			(
				!c_src_data->idiff_file_fp
			||
				!c_src_data->idiff_file_fp->youngest
			)
		&&
			!diff_whine
		)
		{
			change_error(cp, 0, i18n("diff required"));
			++nerr;
			++diff_whine;
		}

		/*
		 * For each change file that is acting on a project file
		 * from a deeper level than the immediate parent
		 * project, the file needs to be added to the immediate
		 * parent project.
		 *
		 * This should already have been done by aede.
		 */
		if (!project_file_shallow_check(pp, c_src_data->file_name))
			this_is_a_bug();

		p_src_data = project_file_find(pp, c_src_data->file_name);
		if (!p_src_data)
			this_is_a_bug();

		/*
		 * remove the file metrics, if any
		 */
		if (c_src_data->metrics)
		{
			metric_list_type.free(c_src_data->metrics);
			c_src_data->metrics = 0;
		}
		if (p_src_data->metrics)
		{
			metric_list_type.free(p_src_data->metrics);
			p_src_data->metrics = 0;
		}

		/*
		 * Grab the file metrics,
		 * if they were produced by the build.
		 *
		 * Only do this for primary source files, and only for
		 * creates and modifies.
		 */
		if
		(
			c_src_data->usage != file_usage_build
		&&
			(
				c_src_data->action == file_action_create
			||
				c_src_data->action == file_action_modify
			)
		)
		{
			metric_list	mlp;

			mlp = change_file_metrics_get(cp, c_src_data->file_name);
			if (mlp)
			{
				c_src_data->metrics = mlp;
				p_src_data->metrics = metric_list_copy(mlp);
			}
		}

		/*
		 * don't do any of the transfers
		 * if the file is built
		 */
		if (c_src_data->usage == file_usage_build)
		{
			transfer_architecture_times = 0;
			transfer_file_times = 0;
			transfer_diff_file_times = 0;
		}
		else if (c_src_data->action == file_action_remove)
		{
			transfer_architecture_times = 0;
			transfer_file_times = 0;
			transfer_diff_file_times = 1;
			if
			(
				c_src_data->move
			&&
				change_file_find(cp, c_src_data->move)
			)
				transfer_diff_file_times = 0;
		}
		else
		{
			transfer_architecture_times = 1;
			transfer_file_times = 1;
			transfer_diff_file_times = 1;
		}

		/*
		 * Preserve the file movement information (this makes
		 * branches and changes more symmetric, as well as
		 * preserving useful information).
		 */
		if (c_src_data->move)
		{
			if (p_src_data->move)
				str_free(p_src_data->move);
			p_src_data->move = str_copy(c_src_data->move);
		}

		/*
		 * Transfer the test times from the change
		 * file to the project file.
		 */
		if (p_src_data->architecture_times)
		{
			fstate_src_architecture_times_list_type.free(p_src_data->architecture_times);
			p_src_data->architecture_times = 0;
		}
		if
		(
			transfer_architecture_times
		&&
			c_src_data->architecture_times
		)
		{
			p_src_data->architecture_times = fstate_src_architecture_times_list_type.alloc();
			for (k = 0; k < c_src_data->architecture_times->length; ++k)
			{
				fstate_src_architecture_times catp;
				fstate_src_architecture_times patp;
				fstate_src_architecture_times *addr;
				type_ty		*type_p;
	
				catp = c_src_data->architecture_times->list[k];
				addr = fstate_src_architecture_times_list_type.list_parse(p_src_data->architecture_times, &type_p);
				assert(type_p == &fstate_src_architecture_times_type);
				patp = fstate_src_architecture_times_type.alloc();
				*addr = patp;
				patp->variant = str_copy(catp->variant);
				patp->test_time = catp->test_time;
				patp->test_baseline_time = catp->test_baseline_time;
			}
		}

		/*
		 * Transfer the diff time from the change
		 * file to the project file.
		 */
		if (p_src_data->file_fp)
		{
			fingerprint_type.free(p_src_data->file_fp);
			p_src_data->file_fp = 0;
		}
		if (transfer_file_times)
		{
			/*
			 * It is possible that there is NO file fingerprint if
			 * an old-format project had a change being integrated
			 * when aegis was upgraded from 2.3 to 3.0.  Give an
			 * error recommending that they start the integration
			 * again.
			 */
			if (!c_src_data->file_fp)
				change_fatal(cp, 0, i18n("restart integration after upgrade"));

			/*
			 * Transfer the file fingerprint.
			 *
			 * Note: the fp->youngest will be wrong, because
			 * the file was copied into the integration
			 * directory.  This will be fixed next time some
			 * operation is done on this file, because the
			 * crypto will be correct.
			 */
			assert(c_src_data->file_fp);
			if (!p_src_data->file_fp)
				p_src_data->file_fp = fingerprint_type.alloc();
			assert(c_src_data->file_fp->youngest);
			p_src_data->file_fp->youngest = c_src_data->file_fp->youngest;
			assert(c_src_data->file_fp->oldest);
			p_src_data->file_fp->oldest = c_src_data->file_fp->oldest;
			if (p_src_data->file_fp->crypto)
				str_free(p_src_data->file_fp->crypto);
			assert(c_src_data->file_fp->crypto);
			p_src_data->file_fp->crypto =
				str_copy(c_src_data->file_fp->crypto);
		}

		/*
		 * Transfer the difference fingerprint.
		 * Note the diff/idiff dichotomy.
		 */
		if (p_src_data->diff_file_fp)
		{
			fingerprint_type.free(p_src_data->diff_file_fp);
			p_src_data->diff_file_fp = 0;
		}
		if (transfer_diff_file_times && c_src_data->idiff_file_fp)
		{
			if (!p_src_data->diff_file_fp)
				p_src_data->diff_file_fp = fingerprint_type.alloc();
			assert(c_src_data->idiff_file_fp->youngest);
			p_src_data->diff_file_fp->youngest =
				c_src_data->idiff_file_fp->youngest;
			assert(c_src_data->idiff_file_fp->oldest);
			p_src_data->diff_file_fp->oldest =
				c_src_data->idiff_file_fp->oldest;
			if (p_src_data->diff_file_fp->crypto)
				str_free(p_src_data->diff_file_fp->crypto);
			assert(c_src_data->idiff_file_fp->crypto);
			p_src_data->diff_file_fp->crypto =
				str_copy(c_src_data->idiff_file_fp->crypto);
		}

		/*
		 * built files and removed file stop here
		 */
		if (!transfer_architecture_times)
			continue;

		/*
		 * The edit number origin *must* be up-to-date, or the
		 * change could not have got this far.
		 *
		 * As a branch advances, the edit_number tracks the
		 * history, but the edit_number_origin is the number when
		 * the file was first created or copied into the branch.
		 * By definition, a file in a change is out of date when
		 * it's edit_number_origin does not equal the edit_number
		 * of its project.
		 *
		 * In order to merge branches, this must be done as a
		 * cross branch merge in a change to that branch; the
		 * edit_number_origin_new field of the change is copied
		 * into the edit_number_origin origin field of the branch.
		 *
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
		if (c_src_data->edit_number && !c_src_data->edit_number_origin)
		{
			/* Historical 2.3 -> 3.0 transition.  */
			c_src_data->edit_number_origin =
				str_copy(c_src_data->edit_number);
		}
		if (c_src_data->edit_number_origin_new)
		{
			if (p_src_data->edit_number_origin)
				str_free(p_src_data->edit_number_origin);
			p_src_data->edit_number_origin =
				c_src_data->edit_number_origin_new;
			c_src_data->edit_number_origin_new = 0;
		}

		/*
		 * update the test correlation
		 */
		if (c_src_data->usage == file_usage_source)
		{
			if (change_was_a_branch(cp))
			{
				size_t		n;

				if (!c_src_data->test)
					n = 0;
				else
					n = c_src_data->test->length;
				for (k = 0; k < n; ++k)
				{
					string_ty	*fn;
					size_t		m;

					if (!p_src_data->test)
						p_src_data->test =
					      fstate_src_test_list_type.alloc();
					fn = c_src_data->test->list[k];
					for (m = 0; m < p_src_data->test->length; ++m)
						if (str_equal(p_src_data->test->list[m], fn))
							break;
					if (m >= p_src_data->test->length)
					{
						string_ty	**addr_p;
						type_ty		*type_p;
	
						addr_p =
					    fstate_src_test_list_type.list_parse
							(
								p_src_data->test,
								&type_p
							);
						assert(type_p == &string_type);
						*addr_p = str_copy(fn);
					}
				}
			}
			else
			{
				for (k = 0; ; ++k)
				{
					fstate_src	src2;
					size_t		m;
		
					src2 = change_file_nth(cp, k);
					if (!src2)
						break;
					if
					(
						src2->usage != file_usage_test
					&&
						src2->usage != file_usage_manual_test
					)
						continue;
		
					if (!p_src_data->test)
						p_src_data->test =
							fstate_src_test_list_type.alloc();
					for (m = 0; m < p_src_data->test->length; ++m)
						if (str_equal(p_src_data->test->list[m], src2->file_name))
							break;
					if (m >= p_src_data->test->length)
					{
						string_ty	**addr_p;
						type_ty		*type_p;
	
						addr_p =
						    fstate_src_test_list_type.list_parse
							(
								p_src_data->test,
								&type_p
							);
						assert(type_p == &string_type);
						*addr_p = str_copy(src2->file_name);
					}
				}
			}
		}
	}

	/*
	 * It is an error if the change has no current build.
	 * It is an error if the change has no current test pass.
	 * It is an error if the change has no current baseline test pass.
	 * It is an error if the change has no current regression test pass.
	 */
	if (!cstate_data->build_time)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "Outstanding", "%s", change_outstanding_builds(cp, youngest));
		sub_var_optional(scp, "Outstanding");
		change_error(cp, scp, i18n("bad ip, build required"));
		sub_context_delete(scp);
		++nerr;
	}
	if (!cstate_data->test_exempt && !cstate_data->test_time)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "Outstanding", "%s", change_outstanding_tests(cp, youngest));
		sub_var_optional(scp, "Outstanding");
		change_error(cp, scp, i18n("bad ip, test required"));
		sub_context_delete(scp);
		++nerr;
	}
	if
	(
		!cstate_data->test_baseline_exempt
	&&
		!cstate_data->test_baseline_time
	)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "Outstanding", "%s", change_outstanding_tests_baseline(cp, youngest));
		sub_var_optional(scp, "Outstanding");
		change_error(cp, scp, i18n("bad ip, test -bl required"));
		++nerr;
	}
	if
	(
		!cstate_data->regression_test_exempt
	&&
		!cstate_data->regression_test_time
	)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set
		(
			scp,
			"Outstanding",
			"%s",
			change_outstanding_tests_regression(cp, youngest)
		);
		sub_var_optional(scp, "Outstanding");
		change_error(cp, scp, i18n("bad ip, test -reg required"));
		sub_context_delete(scp);
		++nerr;
	}

	/*
	 * fail if any of the above detected errors
	 */
	if (nerr)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "Number", "%d", nerr);
		sub_var_optional(scp, "Number");
		change_fatal(cp, scp, i18n("integrate pass fail"));
		sub_context_delete(scp);
	}

	/*
	 * transfer the build and test times into the project,
	 * making sure to leave holes for architecture exemptions
	 */
	p_cstate_data = change_cstate_get(pcp);
	p_cstate_data->build_time = cstate_data->build_time;
	p_cstate_data->test_time = cstate_data->test_time;
	p_cstate_data->test_baseline_time = cstate_data->test_baseline_time;
	p_cstate_data->regression_test_time = cstate_data->regression_test_time;
	if (p_cstate_data->architecture_times)
		cstate_architecture_times_list_type.free
		(
			p_cstate_data->architecture_times
		);
	p_cstate_data->architecture_times =
		cstate_architecture_times_list_type.alloc();
	if (!cstate_data->architecture_times)
		this_is_a_bug();
	for (j = 0; j < cstate_data->architecture_times->length; ++j)
	{
		cstate_architecture_times catp;
		cstate_architecture_times patp;
		cstate_architecture_times *addr;
		type_ty		*type_p;

		catp = cstate_data->architecture_times->list[j];
		addr =
			cstate_architecture_times_list_type.list_parse
			(
				p_cstate_data->architecture_times,
				&type_p
			);
		assert(type_p == &cstate_architecture_times_type);
		patp = cstate_architecture_times_type.alloc();
		*addr = patp;
		if (!catp->variant)
			this_is_a_bug();
		patp->variant = str_copy(catp->variant);
		if (!catp->node)
			this_is_a_bug();
		patp->node = str_copy(catp->node);
		patp->build_time = catp->build_time;
		patp->test_time = catp->test_time;
		patp->test_baseline_time = catp->test_baseline_time;
		patp->regression_test_time = catp->regression_test_time;
	}

	/*
	 * Make sure they aren't in a nuisance place.
	 * (Many systems can't delete a directory if any process has
	 * its current directory within it.)
	 */
	os_become_orig();
	cwd = os_curdir();
	os_become_undo();
	if (os_below_dir(change_development_directory_get(cp, 1), cwd))
		change_fatal(cp, 0, i18n("leave dev dir"));
	if (os_below_dir(id, cwd))
		change_fatal(cp, 0, i18n("leave int dir"));
	if (os_below_dir(project_baseline_path_get(pp, 1), cwd))
		project_fatal(pp, 0, i18n("leave baseline"));

	/*
	 * merge copyright years
	 */
	project_copyright_years_merge(pp, cp);

	/*
	 * note that the project has no current integration
	 */
	project_current_integration_set(pp, 0);

	/*
	 * Clear the default-change field of the user row.
	 * Remove the change from the list of assigned changes in the user
	 * change table (in the user row).
	 */
	user_own_remove(up, project_name_get(pp), change_number);

	/*
	 * Add all of the generated (build) files in the project to this
	 * change so that their history is remembered.
	 */
	for (j = 0; ; ++j)
	{
		fstate_src	c_src_data;
		fstate_src	p_src_data;

		p_src_data = project_file_nth(pp, j);
		if (!p_src_data)
			break;
		if (p_src_data->usage != file_usage_build)
			continue;
		if (p_src_data->deleted_by)
			continue;
		if (p_src_data->about_to_be_created_by)
			continue;
		c_src_data = change_file_find(cp, p_src_data->file_name);
		if (c_src_data)
			continue;
		c_src_data = change_file_new(cp, p_src_data->file_name);
		assert(p_src_data->edit_number);
		c_src_data->edit_number_origin =
			str_copy(p_src_data->edit_number);
		c_src_data->action = file_action_modify;
		c_src_data->usage = p_src_data->usage;
	}

	/*
	 * Update the edit history of each changed file.
	 * Update the edit number of each file.
	 * Remove the fingerprints for each file.
	 * Unlock each file.
	 */
	log_open(change_logfile_get(cp), pup, log_style);
	ncmds = 0;
	hp = project_history_path_get(pp);
	string_list_constructor(&trashed);
	for (j = 0; ; ++j)
	{
		fstate_src	c_src_data;
		fstate_src	p_src_data;
		string_ty	*absfn;

		c_src_data = change_file_nth(cp, j);
		if (!c_src_data)
			break;
		if (c_src_data->file_fp)
		{
			fingerprint_type.free(c_src_data->file_fp);
			c_src_data->file_fp = 0;
		}
		if (c_src_data->diff_file_fp)
		{
			fingerprint_type.free(c_src_data->diff_file_fp);
			c_src_data->diff_file_fp = 0;
		}
		if (c_src_data->idiff_file_fp)
		{
			fingerprint_type.free(c_src_data->idiff_file_fp);
			c_src_data->idiff_file_fp = 0;
		}
		if (c_src_data->architecture_times)
		{
			fstate_src_architecture_times_list_type.free
			(
				c_src_data->architecture_times
			);
			c_src_data->architecture_times = 0;
		}

		p_src_data = project_file_find(pp, c_src_data->file_name);
		if (!p_src_data)
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "File_Name", "fstate file");
			sub_var_set(scp, "FieLD_Name", "src");
			project_fatal
			(
				pp,
				scp,
			      i18n("$filename: corrupted \"$field_name\" field")
			);
			/* NOTREACHED */
			sub_context_delete(scp);
		}
		p_src_data->locked_by = 0;
		p_src_data->about_to_be_copied_by = 0;
		switch (c_src_data->action)
		{
		case file_action_create:
			/*
			 * Because history is never thrown away,
			 * we could be reusing an existing history file.
			 * Also, branches already have history when the
			 * files they created are integrated.
			 */
			if (p_src_data->deleted_by)
			{
				p_src_data->deleted_by = 0;
				p_src_data->about_to_be_created_by = 0;
				goto reusing_an_old_file;
			}

			/*
			 * it exists, now
			 */
			trace(("create\n"));
			p_src_data->about_to_be_created_by = 0;

			/*
			 * Remember the last-modified-time, so we can
			 * restore it if the history tool messes with it.
			 */
			absfn = change_file_path(cp, c_src_data->file_name);
			if (!absfn)
				absfn = str_format("%S/%S", id, c_src_data->file_name);
			project_become(pp);
			mtime = os_mtime_actual(absfn);
			project_become_undo();

			/*
			 * create the history
			 */
			change_run_history_create_command
			(
				cp,
				c_src_data->file_name
			);

			/*
			 * Update the head revision number.
			 * (Create also sets edit_number_origin,
			 * if not already set)
			 */
			if (p_src_data->edit_number)
				str_free(p_src_data->edit_number);
			p_src_data->edit_number =
				change_run_history_query_command
				(
					cp,
					c_src_data->file_name
				);
			if (!p_src_data->edit_number_origin)
				p_src_data->edit_number_origin =
					str_copy(p_src_data->edit_number);
			
			/*
			 * Set the last-modified-time, just in case the
			 * history tool changed it, even if it didn't
			 * change the file content.  This reduces the
			 * build burden imposed by an integration.
			 *
			 * We do this before we check the fingerprint.
			 * This has the side-effect of forcing the
			 * fingerprint, but it also gets the fingerprint
			 * right in the project file's attributes.
			 */
			project_become(pp);
			os_mtime_set_errok(absfn, mtime);

			/*
			 * Many history tools (e.g. RCS) can modify the
			 * contents of the file when it is committed.
			 * While there are usually options to turn this
			 * off, they are seldom used.  The problem is:
			 * if the commit changes the file, the source
			 * in the repository now no longer matches the
			 * object file in the repository - i.e. the
			 * history tool has compromised the referential
			 * integrity of the repository.
			 *
			 * Keep track of them, we will generate an
			 * error message after all of the commands have
			 * been run.
			 */
			if (p_src_data->file_fp)
			{
				assert(p_src_data->file_fp->youngest >= 0);
				assert(p_src_data->file_fp->oldest >= 0);
				if (!change_fingerprint_same(p_src_data->file_fp, absfn, 1))
				{
					string_list_append
					(
						&trashed,
						c_src_data->file_name
					);
				}
				assert(p_src_data->file_fp->youngest > 0);
				assert(p_src_data->file_fp->oldest > 0);
			}
			str_free(absfn);
			project_become_undo();

			/*
			 * In the completed state, edit_number
			 * of a change is the version of the file.
			 * Leave edit_number_origin alone, as a record
			 * of where it came from.
			 * (Create also sets edit_number_origin,
			 * if not already set)
			 */
			if (c_src_data->edit_number)
				str_free(c_src_data->edit_number);
			c_src_data->edit_number =
				str_copy(p_src_data->edit_number);
			if (!p_src_data->edit_number_origin)
				p_src_data->edit_number_origin =
					str_copy(p_src_data->edit_number);
			break;

		case file_action_modify:
			reusing_an_old_file:
			trace(("modify\n"));

			if (p_src_data->edit_number)
			{
				str_free(p_src_data->edit_number);
				p_src_data->edit_number = 0;
			}
			if (c_src_data->edit_number)
			{
				str_free(c_src_data->edit_number);
				c_src_data->edit_number = 0;
			}

			/*
			 * Remember the last-modified-time, so we can
			 * restore it if the history tool messes with it.
			 */
			absfn = change_file_path(cp, c_src_data->file_name);
			if (!absfn)
				absfn = str_format("%S/%S", id, c_src_data->file_name);
			project_become(pp);
			mtime = os_mtime_actual(absfn);
			project_become_undo();

			/*
			 * update the history
			 */
			change_run_history_put_command
			(
				cp,
				c_src_data->file_name
			);
			
			/*
			 * Update the head revision number.
			 * (Leave edit_number_origin alone.)
			 */
			p_src_data->edit_number =
				change_run_history_query_command
				(
					cp,
					c_src_data->file_name
				);

			/*
			 * Set the last-modified-time, just in case the
			 * history tool changed it, even if it didn't
			 * change the file content.  This reduces the
			 * build burden imposed by an integration.
			 *
			 * We do this before we check the fingerprint.
			 * This has the side-effect of forcing the
			 * fingerprint, but it also gets the fingerprint
			 * right in the project file's attributes.
			 */
			project_become(pp);
			os_mtime_set_errok(absfn, mtime);

			/*
			 * Many history tools (e.g. RCS) can modify the
			 * contents of the file when it is committed.
			 * While there are usually options to turn this
			 * off, they are seldom used.  The problem is:
			 * if the commit changes the file, the source
			 * in the repository now no longer matches the
			 * object file in the repository - i.e. the
			 * history tool has compromised the referential
			 * integrity of the repository.
			 *
			 * Keep track of them, we will generate an
			 * error message after all of the commands have
			 * been run.
			 */
			if (p_src_data->file_fp)
			{
				if (!change_fingerprint_same(p_src_data->file_fp, absfn, 1))
				{
					string_list_append
					(
						&trashed,
						c_src_data->file_name
					);
				}
				assert(p_src_data->file_fp->youngest > 0);
				assert(p_src_data->file_fp->oldest > 0);
			}
			str_free(absfn);
			project_become_undo();

			/*
			 * In the completed state, edit_number
			 * of a change is the version of the file.
			 * Leave edit_number_origin alone, as a record
			 * of where it came from.
			 */
			c_src_data->edit_number =
				str_copy(p_src_data->edit_number);
			break;

		case file_action_remove:
			/*
			 * don't need to do much for deletions.
			 * Note that we never throw the history file away.
			 */
			trace(("remove\n"));
			p_src_data->deleted_by = change_number;

			/*
			 * This can need clearing when a branch creates
			 * and then deletes a new file.
			 */
			p_src_data->about_to_be_created_by = 0;
			break;

		case file_action_insulate:
			/*
			 * This should never happen: aede will fail if
			 * there are any insulation files.
			 */
			assert(0);
			break;
		}

		/*
		 * make sure the branch action is appropriate
		 */
		if (c_src_data->action == file_action_remove)
			p_src_data->action = file_action_remove;
		else if (pp->parent)
		{
			fstate_src	pp_src_data;

			pp_src_data = project_file_find(pp->parent, c_src_data->file_name);
			if
			(
				pp_src_data
			&&
				!pp_src_data->about_to_be_created_by
			&&
				!pp_src_data->deleted_by
			)
				p_src_data->action = file_action_modify;
			else
				p_src_data->action = file_action_create;

		}
		else
			p_src_data->action = file_action_create;

		/*
		 * For the project trunk, the edit number and the edit
		 * number origin are always identical.  Otherwise, when
		 * branches inherit the project files of their parents,
		 * the parent's files will appear to be out of date,
		 * even though they can not be.
		 */
		if (!pp->parent)
		{
			/* Historical 2.3 -> 3.0 transition. */
			if (!p_src_data->edit_number_origin)
				p_src_data->edit_number_origin =
					str_copy(p_src_data->edit_number);
			assert(p_src_data->edit_number);
			if
			(
				!str_equal
				(
					p_src_data->edit_number,
					p_src_data->edit_number_origin
				)
			)
			{
				str_free(p_src_data->edit_number_origin);
				p_src_data->edit_number_origin =
					str_copy(p_src_data->edit_number);
			}
		}
	}

	/*
	 * Many history tools (e.g. RCS) can modify the contents of the
	 * file when it is committed.  While there are usually options to
	 * turn this off, they are seldom used.  The problem is: if the
	 * commit changes the file, the source in the repository now no
	 * longer matches the object file in the repository - i.e. the
	 * history tool has compromised the referential integrity of
	 * the repository.
	 *
	 * Check here if this is the case, and emit an error message if
	 * so.	(It could be a fatal error, just a warning, or completely
	 * ignored, depending on the history_put_trashes_file field of
	 * the project config file.
	 */
	change_history_trashed_fingerprints(cp, &trashed);
	string_list_destructor(&trashed);

	/*
	 * Advance the change to the 'completed' state.
	 * Clear the build-time field.
	 * Clear the test-time field.
	 * Clear the test-baseline-time field.
	 */
	cstate_data->state = cstate_state_completed;
	change_build_times_clear(cp);
	dev_dir = str_copy(change_development_directory_get(cp, 1));
	change_development_directory_clear(cp);
	new_baseline = str_copy(change_integration_directory_get(cp, 1));
	change_integration_directory_clear(cp);
	int_name = change_integrator_name(cp);
	rev_name = change_reviewer_name(cp);
	dev_name = change_developer_name(cp);

	/*
	 * we are going to keep the delta,
	 * and throw away the old baseline
	 */
	old_baseline =
		str_format
		(
			"%S.D%3.3ld",
			project_baseline_path_get(pp, 1),
			cstate_data->delta_number - 1
		);
	new_baseline = change_integration_directory_get(cp, 1);
	project_become(pp);
	commit_rename(project_baseline_path_get(pp, 1), old_baseline);
	commit_rename(new_baseline, project_baseline_path_get(pp, 1));
	commit_rmdir_tree_bg(old_baseline);
	project_become_undo();
	str_free(old_baseline);

	/*
	 * throw away the development directory
	 */
	if (change_was_a_branch(cp))
	{
		project_become(pp);
		commit_rmdir_tree_bg(dev_dir);
		project_become_undo();
	}
	else
	{
		devup = user_symbolic(pp, dev_name);
		user_become(devup);
		commit_rmdir_tree_bg(dev_dir);
		user_become_undo();
		user_free(devup);
	}
	str_free(dev_dir);

	/*
	 * Collect the modify times of all the files in the integration
	 * directory.  Sort the file times in ascending order, and then
	 * renumber from when we got the lock.
	 */
	change_verbose(cp, 0, i18n("adjust file modification times"));
	tml.list = 0;
	tml.len = 0;
	tml.max = 0;
	project_become(pp);
	dir_walk(id, time_map_get, &tml);

	/*
	 * This is not a valid assertion because a change could remove a
	 * file which has no ancestors, and thus not affect any other
	 * file's modification time.
	 *
	 * if (tml.len < 1)
	 *	this_is_a_bug();
	 */

#if MTIME_BLURB
	if (tml.len > 0)
	{
		char	buf1[30];
		char	buf2[30];
		strcpy(buf1, ctime(&tml.list[0].old));
		strcpy(buf2, ctime(&tml.list[tml.len - 1].old));
		error_raw
		(
			"original times range from %.24s to %.24s = %d seconds",
			buf1,
			buf2,
			(int)(1 + tml.list[tml.len - 1].old - tml.list[0].old)
		);
	}
#endif
	for (j = 0; j < tml.len; ++j)
	{
		assert(tml.list[j].old >= tml.time_aeib);
		assert(j == 0 || tml.list[j].old > tml.list[j - 1].old);
		tml.list[j].new = tml.time_aeip + j;
	}
#if MTIME_BLURB
	if (tml.len > 0)
	{
		char	buf1[30];
		char	buf2[30];
		strcpy(buf1, ctime(&tml.list[0].new));
		strcpy(buf2, ctime(&tml.list[tml.len - 1].new));
		error_raw
		(
			"adjusted times range from %.24s to %.24s = %d seconds",
			buf1,
			buf2,
			(int)(1 + tml.list[tml.len - 1].new - tml.list[0].new)
		);
	}
#endif
	dir_walk(id, time_map_set, &tml);
	project_become_undo();

	/*
	 * Write the change table row.
	 * Write the user table row.
	 * Release advisory locks.
	 */
	change_cstate_write(cp);
	user_ustate_write(up);
	project_pstate_write(pp);
	change_verbose(cp, 0, i18n("discard old directories"));
	commit();
	lock_release();

	/*
	 * warn the user if some files have been timed into the future
	 */
	time(&time_final);
	if (tml.len > 0 && time_final < tml.list[tml.len - 1].new)
	{
		sub_context_ty	*scp;
		long		nsec;

		scp = sub_context_new();
		nsec = tml.list[tml.len - 1].new - time_final;
		sub_var_set(scp, "Number", "%ld", nsec);
		sub_var_optional(scp, "Number");
		error_intl(scp, i18n("warning: file times in future"));
		sub_context_delete(scp);
	}

	/*
	 * run the notify command
	 */
	change_run_integrate_pass_notify_command(cp);

	/*
	 * verbose success message
	 */
	change_verbose(cp, 0, i18n("integrate pass complete"));
	change_free(cp);
	user_free(pup);
	project_free(pp);
	user_free(up);

	/*
	 * Give the tests time to finish removing the old baseline and
	 * the old development directory.
	 */
	os_throttle();
	trace((/*{*/"}\n"));
}


void
integrate_pass()
{
	trace(("integrate_pass()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		integrate_pass_main();
		break;

	case arglex_token_help:
		integrate_pass_help();
		break;

	case arglex_token_list:
		integrate_pass_list(integrate_pass_usage);
		break;
	}
	trace((/*{*/"}\n"));
}

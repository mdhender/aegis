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
 * MANIFEST: functions to manipulate change state data
 */

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#include <change.h>
#include <commit.h>
#include <error.h>
#include <lock.h>
#include <mem.h>
#include <option.h>
#include <os.h>
#include <project.h>
#include <s-v-arg.h>
#include <sub.h>
#include <trace.h>
#include <undo.h>
#include <user.h>
#include <word.h>


change_ty *
change_alloc(pp, number)
	project_ty	*pp;
	long		number;
{
	change_ty	*cp;

	trace(("change_alloc(pp = %08lX, number = %ld)\n{\n"/*}*/, pp, number));
	assert(number >= 1);
	cp = (change_ty *)mem_alloc_clear(sizeof(change_ty));
	cp->reference_count = 1;
	cp->pp = project_copy(pp);
	cp->number = number;
	cp->filename = project_change_path_get(pp, number);
	trace(("return %08lX;\n", cp));
	trace((/*{*/"}\n"));
	return cp;
}


void
change_free(cp)
	change_ty	*cp;
{
	trace(("change_free(cp = %08lX)\n{\n"/*}*/, cp));
	assert(cp->reference_count >= 1);
	cp->reference_count--;
	if (cp->reference_count <= 0)
	{
		assert(cp->pp);
		project_free(cp->pp);
		assert(cp->filename);
		str_free(cp->filename);
		if (cp->cstate_data)
			cstate_type.free(cp->cstate_data);
		if (cp->development_directory)
			str_free(cp->development_directory);
		if (cp->integration_directory)
			str_free(cp->integration_directory);
		if (cp->logfile)
			str_free(cp->logfile);
		if (cp->pconf_path)
			str_free(cp->pconf_path);
		if (cp->pconf_data)
			pconf_type.free(cp->pconf_data);
		mem_free((char *)cp);
	}
	trace((/*{*/"}\n"));
}


change_ty *
change_copy(cp)
	change_ty	*cp;
{
	trace(("change_copy(cp = %08lX)\n{\n"/*}*/, cp));
	assert(cp->reference_count >= 1);
	cp->reference_count++;
	trace(("return %08lX;\n", cp));
	trace((/*{*/"}\n"));
	return cp;
}


static void improve _((cstate));

static void
improve(d)
	cstate		d;
{
	trace(("improve(d = %08lX)\n{\n"/*}*/, (long)d));
	if (!d->history)
		d->history =
			(cstate_history_list)
			cstate_history_list_type.alloc();
	if (!d->src)
		d->src =
			(cstate_src_list)
			cstate_src_list_type.alloc();
	if (!(d->mask & cstate_regression_test_exempt_mask))
	{
		d->regression_test_exempt =
			(
				d->cause != change_cause_internal_improvement
			&&
				d->cause != change_cause_external_improvement
			);
	}
	trace((/*{*/"}\n"));
}


static void lock_sync _((change_ty *));

static void
lock_sync(cp)
	change_ty	*cp;
{
	long		n;

	n = lock_magic();
	if (cp->lock_magic == n)
		return;
	cp->lock_magic = n;

	if (cp->cstate_data && !cp->is_a_new_file)
	{
		cstate_type.free(cp->cstate_data);
		cp->cstate_data = 0;
	}
	if (cp->pconf_path)
	{
		str_free(cp->pconf_path);
		cp->pconf_data = 0;
	}
}


cstate
change_cstate_get(cp)
	change_ty	*cp;
{
	trace(("change_cstate_get(cp = %08lX)\n{\n"/*}*/, cp));
	lock_sync(cp);
	if (!cp->cstate_data)
	{
		assert(cp->filename);
		change_become(cp);
		cp->cstate_data = cstate_read_file(cp->filename->str_text);
		change_become_undo();
		if (!cp->cstate_data->brief_description)
		{
			change_fatal
			(
				cp,
				"%S: corrupted brief_description field",
				cp->filename
			);
		}
		if (!cp->cstate_data->description)
		{
			change_fatal
			(
				cp,
				"%S: corrupted description field",
				cp->filename
			);
		}
		if (!(cp->cstate_data->mask & cstate_state_mask))
		{
			change_fatal
			(
				cp,
				"%S: corrupted state field",
				cp->filename
			);
		}
		if
		(
			cp->cstate_data->state >= cstate_state_being_developed
		&&
			cp->cstate_data->state <= cstate_state_being_integrated
		&&
			!cp->cstate_data->development_directory
		)
		{
			change_fatal
			(
				cp,
				"%S: no development_directory field",
				cp->filename
			);
		}
		if
		(
			cp->cstate_data->state == cstate_state_being_integrated
		&&
			!cp->cstate_data->integration_directory
		)
		{
			change_fatal
			(
				cp,
				"%S: no integration_directory field",
				cp->filename
			);
		}
		if
		(
			cp->cstate_data->state == cstate_state_completed
		&&
			!cp->cstate_data->delta_number
		)
		{
			change_fatal
			(
				cp,
				"%S: no delta_number field",
				cp->filename
			);
		}
		improve(cp->cstate_data);
		if (cp->cstate_data->state == cstate_state_completed)
		{
			long	j;

			for (j = 0; j < cp->cstate_data->src->length; ++j)
			{
				static long mask =
					(
						cstate_src_file_name_mask
					|
						cstate_src_action_mask
					|
						cstate_src_edit_number_mask
					|
						cstate_src_usage_mask
					);

				if ((cp->cstate_data->src->list[j]->mask & mask) != mask)
				{
					change_fatal
					(
						cp,
						"%S: corrupted src field",
						cp->filename
					);
				}
			}
		}
	}
	trace(("return %08lX;\n", cp->cstate_data));
	trace((/*{*/"}\n"));
	return cp->cstate_data;
}


void
change_bind_new(cp)
	change_ty	*cp;
{
	trace(("change_bind_new(cp = %08lX)\n{\n"/*}*/, cp));
	assert(!cp->cstate_data);
	cp->is_a_new_file = 1;
	cp->cstate_data = (cstate)cstate_type.alloc();
	improve(cp->cstate_data);
	trace((/*{*/"}\n"));
}


static int src_cmp _((const void *, const void *));

static int
src_cmp(s1p, s2p)
	const void	*s1p;
	const void	*s2p;
{
	cstate_src	s1;
	cstate_src	s2;

	s1 = *(cstate_src *)s1p;
	s2 = *(cstate_src *)s2p;
	return strcmp(s1->file_name->str_text, s2->file_name->str_text);
}


void
change_cstate_write(cp)
	change_ty	*cp;
{
	string_ty	*filename_new;
	string_ty	*filename_old;
	pstate		pstate_data;
	static int	count;

	trace(("change_cstate_write(cp = %08lX)\n{\n"/*}*/, cp));
	assert(cp->pp);
	assert(cp->cstate_data);
	assert(cp->filename);
	if (!cp->cstate_data->brief_description)
		cp->cstate_data->brief_description = str_from_c("");
	if (!cp->cstate_data->description)
		cp->cstate_data->description =
			str_copy(cp->cstate_data->brief_description);

	/*
	 * sort the files by name
	 */
	assert(cp->cstate_data->src);
	if (cp->cstate_data->src->length >= 2)
	{
		assert(cp->cstate_data->src->list);
		qsort
		(
			cp->cstate_data->src->list,
			cp->cstate_data->src->length,
			sizeof(*cp->cstate_data->src->list),
			src_cmp
		);
	}

	/*
	 * write out the file
	 */
	pstate_data = project_pstate_get(cp->pp);
	filename_new = str_format("%S,%d", cp->filename, ++count);
	filename_old = str_format("%S,%d", cp->filename, ++count);
	change_become(cp);
	if (cp->is_a_new_file)
	{
		string_ty	*s1;
		string_ty	*s2;

		s1 = project_home_path_get(cp->pp);
		s2 = os_below_dir(s1, cp->filename);
		os_mkdir_between(s1, s2, 02755);
		str_free(s2);
		undo_unlink_errok(filename_new);
		cstate_write_file(filename_new->str_text, cp->cstate_data);
		commit_rename(filename_new, cp->filename);
	}
	else
	{
		undo_unlink_errok(filename_new);
		cstate_write_file(filename_new->str_text, cp->cstate_data);
		commit_rename(cp->filename, filename_old);
		commit_rename(filename_new, cp->filename);
		commit_unlink_errok(filename_old);
	}

	/*
	 * Change the file mode as appropriate.
	 * (Only need to do this for new files, but be paranoid.)
	 */
	os_chmod(filename_new, 0644 & ~change_umask(cp));
	change_become_undo();
	str_free(filename_new);
	str_free(filename_old);
	trace((/*{*/"}\n"));
}


cstate_src
change_src_find(cp, file_name)
	change_ty	*cp;
	string_ty	*file_name;
{
	cstate		cstate_data;
	int		j;
	cstate_src	result;

	trace(("change_src_find(cp = %08lX, file_name = \"%s\")\n{\n"/*}*/, cp, file_name->str_text));
	cstate_data = change_cstate_get(cp);
	assert(cstate_data->src);
	result = 0;
	for (j = 0; j < cstate_data->src->length; ++j)
	{
		cstate_src	src_data;

		src_data = cstate_data->src->list[j];
		if (str_equal(src_data->file_name, file_name))
		{
			result = src_data;
			break;
		}
	}
	trace(("return %08lX;\n", result));
	trace((/*{*/"}\n"));
	return result;
}


void
change_src_remove(cp, file_name)
	change_ty	*cp;
	string_ty	*file_name;
{
	cstate		cstate_data;
	int		j;
	cstate_src	src_data;

	trace(("change_src_remove(cp = %08lX, file_name = \"%s\")\n{\n"/*}*/, cp, file_name->str_text));
	cstate_data = change_cstate_get(cp);
	assert(cstate_data->src);
	for (j = 0; j < cstate_data->src->length; ++j)
	{
		src_data = cstate_data->src->list[j];
		if (!str_equal(src_data->file_name, file_name))
			continue;
		cstate_src_type.free(src_data);
		cstate_data->src->list[j] =
			cstate_data->src->list[--cstate_data->src->length];
		break;
	}
	trace((/*{*/"}\n"));
}


cstate_src
change_src_new(cp)
	change_ty	*cp;
{
	cstate		cstate_data;
	cstate_src	src_data;
	cstate_src	*src_data_p;
	type_ty		*type_p;

	trace(("change_src_new(cp = %08lX)\n{\n"/*}*/, cp));
	cstate_data = change_cstate_get(cp);
	assert(cstate_data->src);
	cstate_src_list_type.list_parse
	(
		cstate_data->src,
		&type_p,
		(void **)&src_data_p
	);
	src_data = (cstate_src)cstate_src_type.alloc();
	*src_data_p = src_data;
	trace(("return %08lX;\n", src_data));
	trace((/*{*/"}\n"));
	return src_data;
}


cstate_history
change_history_new(cp, up)
	change_ty	*cp;
	user_ty		*up;
{
	cstate		cstate_data;
	cstate_history	history_data;
	cstate_history	*history_data_p;
	type_ty		*type_p;

	trace(("change_history_new(cp = %08lX)\n{\n"/*}*/, cp));
	cstate_data = change_cstate_get(cp);
	assert(cstate_data->history);
	cstate_history_list_type.list_parse
	(
		cstate_data->history,
		&type_p,
		(void **)&history_data_p
	);
	history_data = (cstate_history)cstate_history_type.alloc();
	*history_data_p = history_data;
	time(&history_data->when);
	history_data->who = str_copy(user_name(up));
	trace(("return %08lX;\n", history_data));
	trace((/*{*/"}\n"));
	return history_data;
}


string_ty *
change_developer_name(cp)
	change_ty	*cp;
{
	cstate		cstate_data;
	cstate_history	history_data;
	long		pos;

	trace(("change_developer_name(cp = %08lX)\n{\n"/*}*/, cp));
	cstate_data = change_cstate_get(cp);
	assert(cstate_data->history);
	history_data = 0;
	for (pos = cstate_data->history->length - 1; pos >= 0 ; --pos)
	{
		history_data = cstate_data->history->list[pos];
		switch (history_data->what)
		{
		default:
			history_data = 0;
			continue;

		case cstate_history_what_develop_begin:
		case cstate_history_what_develop_begin_undo:
		case cstate_history_what_develop_end:
		case cstate_history_what_develop_end_undo:
			break;
		}
		break;
	}
	trace(("return \"%s\";\n",
		history_data ? history_data->who->str_text : ""));
	trace((/*{*/"}\n"));
	return (history_data ? history_data->who : 0);
}


string_ty *
change_reviewer_name(cp)
	change_ty	*cp;
{
	cstate		cstate_data;
	cstate_history	history_data;
	long		pos;

	trace(("change_reviewer_name(cp = %08lX)\n{\n"/*}*/, cp));
	cstate_data = change_cstate_get(cp);
	assert(cstate_data->history);
	assert(cstate_data->state >= cstate_state_awaiting_integration);
	history_data = 0;
	for (pos = cstate_data->history->length - 1; pos >= 0 ; --pos)
	{
		history_data = cstate_data->history->list[pos];
		switch (history_data->what)
		{
		default:
			history_data = 0;
			continue;

		case cstate_history_what_review_pass:
		case cstate_history_what_review_pass_undo:
			break;
		}
		break;
	}
	trace(("return \"%s\";\n",
		history_data ? history_data->who->str_text : ""));
	trace((/*{*/"}\n"));
	return (history_data ? history_data->who : 0);
}


string_ty *
change_integrator_name(cp)
	change_ty	*cp;
{
	cstate		cstate_data;
	cstate_history	history_data;
	long		pos;

	trace(("change_integrator_name(cp = %08lX)\n{\n"/*}*/, cp));
	cstate_data = change_cstate_get(cp);
	assert(cstate_data->history);
	history_data = 0;
	for (pos = cstate_data->history->length - 1; pos >= 0 ; --pos)
	{
		history_data = cstate_data->history->list[pos];
		switch (history_data->what)
		{
		default:
			history_data = 0;
			continue;

		case cstate_history_what_integrate_pass:
		case cstate_history_what_integrate_fail:
		case cstate_history_what_integrate_begin:
		case cstate_history_what_integrate_begin_undo:
			break;
		}
		break;
	}
	trace(("return \"%s\";\n",
		history_data ? history_data->who->str_text : ""));
	trace((/*{*/"}\n"));
	return (history_data ? history_data->who : 0);
}


void
change_bind_existing(cp)
	change_ty	*cp;
{
	pstate		pstate_data;
	int		j;

	/*
	 * verify the change number given on the command line
	 */
	trace(("change_bind_existing(cp = %08lX)\n{\n"/*}*/, cp));
	pstate_data = project_pstate_get(cp->pp);
	assert(!cp->cstate_data);
	assert(pstate_data->change);
	for (j = 0; j < pstate_data->change->length; ++j)
	{
		if (pstate_data->change->list[j] == cp->number)
			break;
	}
	if (j >= pstate_data->change->length)
		change_fatal(cp, "unknown");
	trace((/*{*/"}\n"));
}


void
change_development_directory_set(cp, s)
	change_ty	*cp;
	string_ty	*s;
{
	cstate		cstate_data;

	/*
	 * To cope with automounters, directories are stored as given,
	 * or are derived from the home directory in the passwd file.
	 * Within aegis, pathnames have their symbolic links resolved,
	 * and any comparison of paths is done on this "system idea"
	 * of the pathname.
	 */
	trace(("change_development_directory_set(cp = %08lX, s = \"%s\")\n{\n"/*}*/, cp, s->str_text));
	if (cp->development_directory)
		fatal("duplicate -DIRectory option");
	assert(s->str_text[0] == '/');
	change_become(cp);
	cp->development_directory = os_pathname(s, 1);
	change_become_undo();
	cstate_data = change_cstate_get(cp);
	if (!cstate_data->development_directory)
		cstate_data->development_directory = str_copy(s);
	trace((/*{*/"}\n"));
}


string_ty *
change_development_directory_get(cp, resolve)
	change_ty	*cp;
	int		resolve;
{
	string_ty	*result;
	cstate		cstate_data;

	/*
	 * To cope with automounters, directories are stored as given,
	 * or are derived from the home directory in the passwd file.
	 * Within aegis, pathnames have their symbolic links resolved,
	 * and any comparison of paths is done on this "system idea"
	 * of the pathname.
	 */
	trace(("change_development_directory_get(cp = %08lX)\n{\n"/*}*/, cp));
	if (!resolve)
	{
		cstate_data = change_cstate_get(cp);
		result = cstate_data->development_directory;
	}
	else
	{
		if (!cp->development_directory)
		{
			cstate_data = change_cstate_get(cp);
			if (!cstate_data->development_directory)
			{
				change_fatal
				(
					cp,
	    "this change is in the %s state, there is no development directory",
					cstate_state_ename(cstate_data->state)
				);
			}
			change_become(cp);
			cp->development_directory =
				os_pathname
				(
					cstate_data->development_directory,
					1
				);
			change_become_undo();
		}
		result = cp->development_directory;
	}
	trace_string(result->str_text);
	trace((/*{*/"}\n"));
	return result;
}


void
change_integration_directory_set(cp, s)
	change_ty	*cp;
	string_ty	*s;
{
	cstate		cstate_data;

	/*
	 * To cope with automounters, directories are stored as given,
	 * or are derived from the home directory in the passwd file.
	 * Within aegis, pathnames have their symbolic links resolved,
	 * and any comparison of paths is done on this "system idea"
	 * of the pathname.
	 */
	trace(("change_integration_directory_set(cp = %08lX, s = \"%s\")\n{\n"/*}*/, cp, s->str_text));
	if (cp->integration_directory)
		fatal("duplicate -DIRectory option");
	change_become(cp);
	cp->integration_directory = os_pathname(s, 1);
	change_become_undo();
	cstate_data = change_cstate_get(cp);
	if (!cstate_data->integration_directory)
		cstate_data->integration_directory = str_copy(s);
	trace((/*{*/"}\n"));
}


string_ty *
change_integration_directory_get(cp, resolve)
	change_ty	*cp;
	int		resolve;
{
	string_ty	*result;
	cstate		cstate_data;

	/*
	 * To cope with automounters, directories are stored as given,
	 * or are derived from the home directory in the passwd file.
	 * Within aegis, pathnames have their symbolic links resolved,
	 * and any comparison of paths is done on this "system idea"
	 * of the pathname.
	 */
	trace(("change_integration_directory_get(cp = %08lX)\n{\n"/*}*/, cp));
	if (!resolve)
	{
		cstate_data = change_cstate_get(cp);
		result = cstate_data->integration_directory;
	}
	else
	{
		if (!cp->integration_directory)
		{
			cstate_data = change_cstate_get(cp);
			if (!cstate_data->integration_directory)
			{
				change_fatal
				(
					cp,
	    "this change is in the %s state, there is no integration directory",
					cstate_state_ename(cstate_data->state)
				);
			}
			change_become(cp);
			cp->integration_directory =
				os_pathname
				(
					cstate_data->integration_directory,
					1
				);
			change_become_undo();
		}
		result = cp->integration_directory;
	}
	trace_string(result->str_text);
	trace((/*{*/"}\n"));
	return result;
}


string_ty *
change_logfile_get(cp)
	change_ty	*cp;
{
	string_ty	*s1;
	cstate		cstate_data;

	trace(("change_logfile_get(cp = %08lX)\n{\n"/*}*/, cp));
	if (!cp->logfile)
	{
		cstate_data = change_cstate_get(cp);
		switch (cstate_data->state)
		{
		default:
			change_fatal(cp, "no log file");
	
		case cstate_state_being_integrated:
			s1 = change_integration_directory_get(cp, 0);
			break;
	
		case cstate_state_being_developed:
			s1 = change_development_directory_get(cp, 0);
			break;
		}
	
		cp->logfile =
			str_format("%S/%s.log", s1, option_progname_get());
	}
	trace(("return \"%s\";\n", cp->logfile->str_text));
	trace((/*{*/"}\n"));
	return cp->logfile;
}


void
change_cstate_lock_prepare(cp)
	change_ty	*cp;
{
	trace(("change_cstate_lock_prepare(cp = %08lX)\n{\n"/*}*/, cp));
	lock_prepare_cstate(project_name_get(cp->pp), cp->number);
	trace((/*{*/"}\n"));
}


void
change_error(cp, s sva_last)
	change_ty	*cp;
	char		*s;
	sva_last_decl
{
	va_list		ap;
	string_ty	*msg;

	sva_init(ap, s);
	msg = str_vformat(s, ap);
	va_end(ap);
	project_error(cp->pp, "change %ld: %S", cp->number, msg);
}


void
change_fatal(cp, s sva_last)
	change_ty	*cp;
	char		*s;
	sva_last_decl
{
	va_list		ap;
	string_ty	*msg;

	sva_init(ap, s);
	msg = str_vformat(s, ap);
	va_end(ap);
	project_fatal(cp->pp, "change %ld: %S", cp->number, msg);
}


void
change_verbose(cp, s sva_last)
	change_ty	*cp;
	char		*s;
	sva_last_decl
{
	va_list		ap;
	string_ty	*msg;

	sva_init(ap, s);
	msg = str_vformat(s, ap);
	va_end(ap);
	project_verbose(cp->pp, "change %ld: %S", cp->number, msg);
}


string_ty *
change_pconf_path_get(cp)
	change_ty	*cp;
{
	cstate		cstate_data;
	pstate_src	p_src_data;
	cstate_src	c_src_data;
	static string_ty *file_name;

	trace(("change_pconf_path_get(cp = %08lX)\n{\n"/*}*/, cp));
	if (cp->pconf_path)
		goto ret;
	if (!file_name)
		file_name = str_from_c(THE_CONFIG_FILE);
	cstate_data = change_cstate_get(cp);
	assert(cstate_data->src);
	switch (cstate_data->state)
	{
	case cstate_state_being_integrated:
		cp->pconf_path =
			str_format
			(
				"%S/%S",
				change_integration_directory_get(cp, 1),
				file_name
			);
		goto ret;

	case cstate_state_being_developed:
	case cstate_state_being_reviewed:
	case cstate_state_awaiting_integration:
		break;

	default:
		change_fatal
		(
			cp,
			"hunting '%S' file from weird state (bug)",
			file_name
		);
	}
	c_src_data = change_src_find(cp, file_name);
	if (c_src_data || cp->number == 1)
	{
		cp->pconf_path =
			str_format
			(
				"%S/%S",
				change_development_directory_get(cp, 1),
				file_name
			);
		goto ret;
	}
	p_src_data = project_src_find(cp->pp, file_name);
	if (p_src_data)
	{
		cp->pconf_path =
			str_format
			(
				"%S/%S",
				project_baseline_path_get(cp->pp, 1),
				file_name
			);
		goto ret;
	}
	change_fatal(cp, "has no '%S' file (bug)", file_name);
	ret:
	trace(("return \"%s\";\n", cp->pconf_path->str_text));
	trace((/*{*/"}\n"));
	return cp->pconf_path;
}


pconf
change_pconf_get(cp)
	change_ty	*cp;
{
	trace(("change_pconf_get(cp = %08lX)\n{\n"/*}*/, cp));
	lock_sync(cp);
	if (!cp->pconf_data)
	{
		string_ty	*filename;

		filename = change_pconf_path_get(cp);
		change_become(cp);
		if (!os_exists(filename))
			cp->pconf_data = (pconf)pconf_type.alloc();
		else
		{
			cp->pconf_data = pconf_read_file(filename->str_text);
			if (!cp->pconf_data->build_command)
			{
				change_fatal
				(
					cp,
					"%S: no build_command field",
					filename
				);
			}
			if (!cp->pconf_data->development_build_command)
				cp->pconf_data->development_build_command =
					str_copy(cp->pconf_data->build_command);
			if (!cp->pconf_data->history_create_command)
			{
				change_fatal
				(
					cp,
					"%S: no history_create_command field",
					filename
				);
			}
			if (!cp->pconf_data->history_get_command)
			{
				change_fatal
				(
					cp,
					"%S: no history_get_command field",
					filename
				);
			}
			if (!cp->pconf_data->history_put_command)
			{
				change_fatal
				(
					cp,
					"%S: no history_put_command field",
					filename
				);
			}
			if (!cp->pconf_data->history_query_command)
			{
				change_fatal
				(
					cp,
					"%S: no history_query_command field",
					filename
				);
			}
			if (!cp->pconf_data->diff_command)
			{
				change_fatal
				(
					cp,
					"%S: no diff_command field",
					filename
				);
			}
			if (!cp->pconf_data->diff3_command)
			{
				change_fatal
				(
					cp,
					"%S: no diff3_command field",
					filename
				);
			}
		}
		change_become_undo();
	}
	trace(("return %08lX;\n", cp->pconf_data));
	trace((/*{*/"}\n"));
	return cp->pconf_data;
}


void
change_run_change_file_command(cp, wlp, up)
	change_ty	*cp;
	wlist		*wlp;
	user_ty		*up;
{
	pconf		pconf_data;
	string_ty	*the_command;
	string_ty	*the_files;
	pstate		pstate_data;
	string_ty	*dd;

	trace(("change_run_change_file_command(cp = %08lX)\n{\n"/*}*/, cp));
	pconf_data = change_pconf_get(cp);
	if (!pconf_data->change_file_command)
		goto ret;
	pstate_data = project_pstate_get(cp->pp);

	the_files = wl2str(wlp, 0, 32767);
	sub_var_set("File_List", "%S", the_files);
	str_free(the_files);
	sub_var_set("1", "${project}");
	sub_var_set("2", "${change}");
	sub_var_set("3", "${version}");
	sub_var_set("4", "${baseline}");
	sub_var_set("5", "${file_list}");
	the_command = pconf_data->change_file_command;
	the_command = substitute(cp, the_command);
	dd = change_development_directory_get(cp, 0);
	user_become(up);
	os_execute(the_command, OS_EXEC_FLAG_NO_INPUT, dd);
	user_become_undo();
	str_free(the_command);
	ret:
	trace((/*{*/"}\n"));
}


void
change_run_project_file_command(cp)
	change_ty	*cp;
{
	cstate		cstate_data;
	pconf		pconf_data;
	string_ty	*the_command;
	string_ty	*dd;

	/*
	 * make sure are sync'ed with project
	 */
	trace(("change_run_project_file_command(cp = %08lX)\n{\n"/*}*/, cp));
	cstate_data = change_cstate_get(cp);
	if
	(
		cstate_data->project_file_command_sync
	==
		project_last_change_integrated(cp->pp)
	)
		goto done;
	cstate_data->project_file_command_sync =
		project_last_change_integrated(cp->pp);

	/*
	 * make sure there is a project_file command
	 */
	pconf_data = change_pconf_get(cp);
	if (!pconf_data->project_file_command)
		goto done;

	/*
	 * All of the substitutions described in aesub(5) are available.
	 */
	sub_var_set("1", "${project}");
	sub_var_set("2", "${change}");
	sub_var_set("3", "${version}");
	sub_var_set("4", "${baseline}");
	the_command = pconf_data->project_file_command;
	the_command = substitute(cp, the_command);

	/*
	 * execute the command
	 */
	dd = change_development_directory_get(cp, 0);
	change_become(cp);
	os_execute(the_command, OS_EXEC_FLAG_NO_INPUT, dd);
	change_become_undo();
	str_free(the_command);

	/*
	 * here for all exits
	 */
	done:
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	gmatch - match entryname pattern
 *
 * SYNOPSIS
 *	int gmatch(char *formal, char *actual);
 *
 * DESCRIPTION
 *	The formal strings is used as a template to match the given actual
 *	string against.
 *
 *	The pattern elements understood are
 *		*	match zero or more of any character
 *		?	match any single character
 *		[^xxx]	match any single character not in the set given.
 *		[xxx]	match any single character in the set given.
 *			The - character is understood to be a range indicator.
 *			If the ] character is the first of the set it is
 *			considered as part of the set, not the terminator.
 *
 * RETURNS
 *	the gmatch function returns zero if they do not match,
 *	and nonzero if they do.  Returns -1 on error.
 *
 * CAVEAT
 *	This is a limited set of the sh(1) patterns.
 *	Assumes that the `original' global variable has been initialized,
 *	it is used for error reporting.
 */

static int gmatch _((char *, char *));

static int
gmatch(formal, actual)
	char	*formal;
	char	*actual;
{
	char	*cp;
	int	 result;

	trace(("gmatch(formal = %08lX, actual = %08lX)\n{\n"/*}*/,
		formal, actual));
	while (*formal)
	{
		trace(("formal == \"%s\";\n", formal));
		trace(("actual = \"%s\";\n", actual));
		switch (*formal)
		{
		default:
			if (*actual++ != *formal++)
			{
				result = 0;
				goto ret;
			}
			break;

		case '?':
			if (!*actual++)
			{
				result = 0;
				goto ret;
			}
			++formal;
			break;

		case '*':
			cp = actual + strlen(actual);
			++formal;
			for (;;)
			{
				result = gmatch(formal, cp);
				if (result)
				{
					result = 1;
					goto ret;
				}
				--cp;
				if (cp < actual)
				{
					result = 0;
					goto ret;
				}
			}

		case '['/*]*/:
			++formal;
			if (*formal == '^')
			{
				++formal;
				for (;;)
				{
					if (!*formal)
					{
						no_close:
						result = -1;
						goto ret;
					}

					/*
					 * note: this allows leading close
					 * square bracket elegantly
					 */
					if
					(
						formal[1] == '-'
					&&
						formal[2]
					&&
						formal[2] != /*[*/']'
					&&
						formal[3]
					)
					{
						char	c1;
						char	c2;

						c1 = formal[0];
						c2 = formal[2];
						formal += 3;
						if
						(
							c1 <= c2
						?
							(c1 <= *actual && *actual <= c2)
						:
							(c2 <= *actual && *actual <= c1)
						)
						{
							result = 0;
							goto ret;
						}
					}
					else
					if (*actual == *formal++)
					{
						result = 0;
						goto ret;
					}
					if (*formal == /*[*/']')
						break;
				}
				++formal;
			}
			else
			{
				for (;;)
				{
					if (!*formal)
						goto no_close;

					/*
					 * note: this allows leading close
					 * square bracket elegantly
					 */
					trace(("formal == \"%s\";\n", formal));
					trace(("actual = \"%s\";\n", actual));
					if
					(
						formal[1] == '-'
					&&
						formal[2]
					&&
						formal[2] != /*[*/']'
					&&
						formal[3]
					)
					{
						char	c1;
						char	c2;

						c1 = formal[0];
						c2 = formal[2];
						formal += 3;
						if
						(
							c1 <= c2
						?
							(c1 <= *actual && *actual <= c2)
						:
							(c2 <= *actual && *actual <= c1)
						)
							break;
					}
					else
					if (*actual == *formal++)
						break;
					if (*formal == /*[*/']')
					{
						result = 0;
						goto ret;
					}
				}
				for (;;)
				{
					if (!*formal)
						goto no_close;
					trace(("formal == \"%s\";\n", formal));
					trace(("actual = \"%s\";\n", actual));
					if (*formal++ == /*[*/']')
						break;
				}
			}
			++actual;
			break;
		}
	}
	result = (*actual == 0);

	/*
	 * here for all exits
	 */
	ret:
	trace(("return %d;\n", result));
	trace((/*{*/"}\n"));
	return result;
}


string_ty *
change_file_template(cp, name)
	change_ty	*cp;
	string_ty	*name;
{
	string_ty	*result;
	size_t		j, k;
	pconf		pconf_data;

	trace(("change_file_template(name = \"%s\")\n{\n"/*}*/, name->str_text));
	result = 0;
	pconf_data = change_pconf_get(cp);
	if (!pconf_data->file_template)
		goto done;
	for (j = 0; j < pconf_data->file_template->length; ++j)
	{
		pconf_file_template ftp;

		ftp = pconf_data->file_template->list[j];
		if (!ftp->pattern)
			continue;
		for (k = 0; k < ftp->pattern->length; ++k)
		{
			int		m;
			string_ty	*s;

			s = ftp->pattern->list[k];
			m = gmatch(s->str_text, name->str_text);
			if (m < 0)
			{
				change_fatal
				(
					cp,
					"filename pattern \"%S\" is illegal",
					s
				);
			}
			if (m)
				break;
		}
		if (k < ftp->pattern->length)
		{
			result = str_copy(ftp->body);
			break;
		}
	}

	/*
	 * here for all exits
	 */
	done:
	if (result)
	{
		string_ty	*s;

		sub_var_set("File_Name", "%S", name);
		s = substitute(cp, result);
		str_free(result);
		result = s;
	}
	trace((/*{*/"}\n"));
	return result;
}


void
change_become(cp)
	change_ty	*cp;
{
	trace(("change_become(cp = %08lX)\n{\n"/*}*/, cp));
	project_become(cp->pp);
	trace((/*{*/"}\n"));
}


void
change_become_undo()
{
	trace(("change_become_undo()\n{\n"/*}*/));
	project_become_undo();
	trace((/*{*/"}\n"));
}


int
change_umask(cp)
	change_ty	*cp;
{
	return project_umask(cp->pp);
}


void
change_run_develop_end_notify_command(cp)
	change_ty	*cp;
{
	pstate		pstate_data;
	string_ty	*the_command;
	string_ty	*dd;

	/*
	 * make sure there is one
	 */
	trace(("change_run_develop_end_notify_command(cp = %08lX)\n{\n"/*}*/, cp));
	pstate_data = project_pstate_get(cp->pp);
	if (!pstate_data->develop_end_notify_command)
		goto done;

	/*
	 * notify the change is ready for review
	 *	(it could be mail, or an internal bulletin board, etc)
	 * it happens after the data is written and the locks are released,
	 * so we don't much care if the command fails!
	 *
	 * All of the substitutions described in aesub(5) are available.
	 */
	sub_var_set("1", "${project}");
	sub_var_set("2", "${change}");
	sub_var_set("3", "${developer}");
	the_command = pstate_data->develop_end_notify_command;
	the_command = substitute(cp, the_command);

	/*
	 * execute the command
	 */
	dd = change_development_directory_get(cp, 0);
	change_become(cp);
	os_execute(the_command, OS_EXEC_FLAG_NO_INPUT + OS_EXEC_FLAG_ERROK, dd);
	change_become_undo();
	str_free(the_command);

	/*
	 * here for all exits
	 */
	done:
	trace((/*{*/"}\n"));
}


void
change_run_develop_end_undo_notify_command(cp)
	change_ty	*cp;
{
	pstate		pstate_data;
	string_ty	*the_command;
	string_ty	*dd;

	/*
	 * make sure there is one
	 */
	trace(("change_run_develop_end_undo_notify_command(cp = %08lX)\n{\n"/*}*/, cp));
	pstate_data = project_pstate_get(cp->pp);
	if (!pstate_data->develop_end_undo_notify_command)
		goto done;

	/*
	 * notify the change is ready for review
	 *	(it could be mail, or an internal bulletin board, etc)
	 * it happens after the data is written and the locks are released,
	 * so we don't much care if the command fails!
	 *
	 * All of the substitutions described in aesub(5) are available.
	 */
	sub_var_set("1", "${project}");
	sub_var_set("2", "${change}");
	sub_var_set("3", "${developer}");
	the_command = pstate_data->develop_end_undo_notify_command;
	the_command = substitute(cp, the_command);

	/*
	 * execute the command
	 */
	dd = change_development_directory_get(cp, 0);
	change_become(cp);
	os_execute(the_command, OS_EXEC_FLAG_NO_INPUT + OS_EXEC_FLAG_ERROK, dd);
	change_become_undo();
	str_free(the_command);

	/*
	 * here for all exits
	 */
	done:
	trace((/*{*/"}\n"));
}


void
change_run_integrate_fail_notify_command(cp)
	change_ty	*cp;
{
	pstate		pstate_data;
	string_ty	*the_command;
	string_ty	*dd;

	/*
	 * make sure there is one
	 */
	trace(("change_run_integrate_fail_notify_command(cp = %08lX)\n{\n"/*}*/, cp));
	pstate_data = project_pstate_get(cp->pp);
	if (!pstate_data->integrate_fail_notify_command)
		goto done;

	/*
	 * notify the integrate has failed
	 *	(it could be mail, or an internal bulletin board, etc)
	 * it happens after the data is written and the locks are released,
	 * so we don't much care it the command fails!
	 *
	 * In doing it after the locks are released,
	 * the lists will be accurate (e.g. list of files in change).
	 *
	 * All of the substitutions described in aesub(5) are available.
	 */
	sub_var_set("1", "${project}");
	sub_var_set("2", "${change}");
	sub_var_set("3", "${developer}");
	sub_var_set("4", "${reviewer}");
	sub_var_set("5", "${integrator}");
	the_command = pstate_data->integrate_fail_notify_command;
	the_command = substitute(cp, the_command);

	/*
	 * execute the command
	 */
	dd = change_development_directory_get(cp, 0);
	change_become(cp);
	os_execute(the_command, OS_EXEC_FLAG_NO_INPUT + OS_EXEC_FLAG_ERROK, dd);
	change_become_undo();
	str_free(the_command);

	/*
	 * here for all exits
	 */
	done:
	trace((/*{*/"}\n"));
}


void
change_run_integrate_pass_notify_command(cp)
	change_ty	*cp;
{
	pstate		pstate_data;
	string_ty	*the_command;
	string_ty	*bl;

	/*
	 * make sure there is one
	 */
	trace(("change_run_integrate_pass_notify_command(cp = %08lX)\n{\n"/*}*/, cp));
	pstate_data = project_pstate_get(cp->pp);
	if (!pstate_data->integrate_pass_notify_command)
		goto done;

	/*
	 * notify the integrate has passed
	 *	(it could be mail, or an internal bulletin board, etc)
	 * it happens after the data is written and the locks are released,
	 * so we don't much care it the command fails!
	 *
	 * In doing it after the locks are released,
	 * the lists will be accurate (e.g. list of files in change).
	 *
	 * All of the substitutions described in aesub(5) are available.
	 */
	sub_var_set("1", "${project}");
	sub_var_set("2", "${change}");
	sub_var_set("3", "${developer}");
	sub_var_set("4", "${reviewer}");
	sub_var_set("5", "${integrator}");
	the_command = pstate_data->integrate_pass_notify_command;
	the_command = substitute(cp, the_command);

	/*
	 * execute the command
	 */
	bl = project_baseline_path_get(cp->pp, 0);
	project_become(cp->pp);
	os_execute(the_command, OS_EXEC_FLAG_NO_INPUT + OS_EXEC_FLAG_ERROK, bl);
	project_become_undo();
	str_free(the_command);

	/*
	 * here for all exits
	 */
	done:
	trace((/*{*/"}\n"));
}


void
change_run_review_pass_notify_command(cp)
	change_ty	*cp;
{
	pstate		pstate_data;
	string_ty	*the_command;
	string_ty	*dd;

	/*
	 * make sure there is one
	 */
	trace(("change_run_review_pass_notify_command(cp = %08lX)\n{\n"/*}*/, cp));
	pstate_data = project_pstate_get(cp->pp);
	if (!pstate_data->review_pass_notify_command)
		goto done;

	/*
	 * notify the review has passed
	 *	(it could be mail, or an internal bulletin board, etc)
	 * it happens after the data is written and the locks are released,
	 * so we don't much care if the command fails!
	 *
	 * All of the substitutions described in aesub(5) are available.
	 */
	sub_var_set("1", "${project}");
	sub_var_set("2", "${change}");
	sub_var_set("3", "${developer}");
	sub_var_set("4", "${reviewer}");
	the_command = pstate_data->review_pass_notify_command;
	the_command = substitute(cp, the_command);

	/*
	 * execute the command
	 */
	dd = change_development_directory_get(cp, 0);
	change_become(cp);
	os_execute(the_command, OS_EXEC_FLAG_NO_INPUT + OS_EXEC_FLAG_ERROK, dd);
	str_free(the_command);
	change_become_undo();

	/*
	 * here for all exits
	 */
	done:
	trace((/*{*/"}\n"));
}


void
change_run_review_pass_undo_notify_command(cp)
	change_ty	*cp;
{
	pstate		pstate_data;
	string_ty	*the_command;
	string_ty	*dd;
	string_ty	*notify;

	/*
	 * make sure there is one
	 */
	trace(("change_run_review_pass_undo_notify_command(cp = %08lX)\n{\n"/*}*/, cp));
	pstate_data = project_pstate_get(cp->pp);
	notify = pstate_data->review_pass_notify_command;
	if (!notify)
		notify = pstate_data->develop_end_undo_notify_command;
	if (!notify)
		goto done;

	/*
	 * notify the review has had the pass rescinded
	 *	(it could be mail, or an internal bulletin board, etc)
	 * it happens after the data is written and the locks are released,
	 * so we don't much care if the command fails!
	 *
	 * All of the substitutions described in aesub(5) are available.
	 */
	sub_var_set("1", "${project}");
	sub_var_set("2", "${change}");
	sub_var_set("3", "${developer}");
	sub_var_set("4", "${reviewer}");
	the_command = substitute(cp, notify);

	/*
	 * execute the command
	 */
	dd = change_development_directory_get(cp, 0);
	change_become(cp);
	os_execute(the_command, OS_EXEC_FLAG_NO_INPUT + OS_EXEC_FLAG_ERROK, dd);
	str_free(the_command);
	change_become_undo();

	/*
	 * here for all exits
	 */
	done:
	trace((/*{*/"}\n"));
}


void
change_run_review_fail_notify_command(cp)
	change_ty	*cp;
{
	pstate		pstate_data;
	string_ty	*the_command;
	string_ty	*dd;

	/*
	 * make sure there is one
	 */
	trace(("change_run_review_fail_notify_command(cp = %08lX)\n{\n"/*}*/, cp));
	pstate_data = project_pstate_get(cp->pp);
	if (!pstate_data->review_fail_notify_command)
		goto done;

	/*
	 * notify the review has failed
	 *	(it could be mail, or an internal bulletin board, etc)
	 * it happens after the data is written and the locks are released,
	 * so we don't much care it the command fails!
	 *
	 * In doing it after the locks are released,
	 * the lists will be accurate (e.g. list of files in change).
	 *
	 * All of the substitutions described in aesub(5) are available.
	 */
	sub_var_set("1", "${project}");
	sub_var_set("2", "${change}");
	sub_var_set("3", "${developer}");
	sub_var_set("4", "${reviewer}");
	the_command = pstate_data->review_fail_notify_command;
	the_command = substitute(cp, the_command);

	/*
	 * execute the command
	 */
	dd = change_development_directory_get(cp, 0);
	change_become(cp);
	os_execute(the_command, OS_EXEC_FLAG_NO_INPUT + OS_EXEC_FLAG_ERROK, dd);
	str_free(the_command);
	change_become_undo();

	/*
	 * here for all exits
	 */
	done:
	trace((/*{*/"}\n"));
}


void
change_run_history_get_command(cp, file_name, edit_number, output_file, up)
	change_ty	*cp;
	string_ty	*file_name;
	string_ty	*edit_number;
	string_ty	*output_file;
	user_ty		*up;
{
	string_ty	*dir;
	cstate		cstate_data;
	string_ty	*the_command;
	pconf		pconf_data;

	/*
	 * If the edit numbers differ, extract the
	 * appropriate edit from the baseline (use the
	 * history-get-command) into a file in /tmp
	 *
	 * All of the substitutions described in aesub(5) are available;
	 * in addition
	 *
	 * ${History}
	 *	history file
	 *
	 * ${Edit}
	 *	edit number
	 *
	 * ${Output}
	 *	output file
	 */
	trace(("change_run_history_get_command(cp = %08lX)\n{\n"/*}*/, cp));
	sub_var_set
	(
		"History",
		"%S/%S",
		project_history_path_get(cp->pp),
		file_name
	);
	sub_var_set("Edit", "%S", edit_number);
	sub_var_set("Output", "%S", output_file);
	sub_var_set("1", "${history}");
	sub_var_set("2", "${edit}");
	sub_var_set("3", "${output}");
	pconf_data = change_pconf_get(cp);
	the_command = pconf_data->history_get_command;
	the_command = substitute(cp, the_command);

	/*
	 * run the command as the current user
	 * (always output is to /tmp)
	 */
	cstate_data = change_cstate_get(cp);
	if (cstate_data->state == cstate_state_being_integrated)
		dir = change_integration_directory_get(cp, 0);
	else
		dir = change_development_directory_get(cp, 0);
	user_become(up);
	os_execute(the_command, OS_EXEC_FLAG_NO_INPUT, dir);
	user_become_undo();
	str_free(the_command);
	trace((/*{*/"}\n"));
}


void
change_development_directory_clear(cp)
	change_ty	*cp;
{
	cstate		cstate_data;

	cstate_data = change_cstate_get(cp);
	assert(cstate_data->development_directory);
	if (cstate_data->development_directory)
	{
		str_free(cstate_data->development_directory);
		cstate_data->development_directory = 0;
	}
}


void
change_integration_directory_clear(cp)
	change_ty	*cp;
{
	cstate		cstate_data;

	cstate_data = change_cstate_get(cp);
	assert(cstate_data->integration_directory);
	if (cstate_data->integration_directory)
	{
		str_free(cstate_data->integration_directory);
		cstate_data->integration_directory = 0;
	}
}


void
change_run_history_create_command(cp, filename)
	change_ty	*cp;
	string_ty	*filename;
{
	string_ty	*hp;
	string_ty	*id;
	pconf		pconf_data;
	string_ty	*the_command;

	/*
	 * create a new history
	 * All of the substitutions described in aesub(5) are avaliable.
	 * In addition:
	 *
	 * ${Input}
	 *	absolute path of source file
	 *
	 * ${History}
	 *	absolute path of history file
	 */
	trace(("change_run_history_create_command(cp = %08lX, \
filename = \"%s\")\n{\n"/*}*/, (long)cp, filename->str_text));
	pconf_data = change_pconf_get(cp);
	hp = project_history_path_get(cp->pp);
	id = change_integration_directory_get(cp, 0);
	sub_var_set("Input", "%S/%S", id, filename);
	sub_var_set("History", "%S/%S", hp, filename);
	sub_var_set("1", "${input}");
	sub_var_set("2", "${history}");
	the_command = pconf_data->history_create_command;
	the_command = substitute(cp, the_command);
	project_become(cp->pp);
	os_mkdir_between(hp, filename, 02755);
	os_execute(the_command, OS_EXEC_FLAG_NO_INPUT, id);
	project_become_undo();
	str_free(the_command);
	trace((/*{*/"}\n"));
}


void
change_run_history_put_command(cp, filename)
	change_ty	*cp;
	string_ty	*filename;
{
	string_ty	*hp;
	string_ty	*id;
	pconf		pconf_data;
	string_ty	*the_command;

	/*
	 * Update and existing history.
	 * All of the substitutions described in aesub(5) are avaliable.
	 * In addition:
	 *
	 * ${Input}
	 *	absolute path of source file
	 *
	 * ${History}
	 *	absolute path of history file
	 */
	trace(("change_run_history_put_command(cp = %08lX, \
filename = \"%s\")\n{\n"/*}*/, (long)cp, filename->str_text));
	pconf_data = change_pconf_get(cp);
	hp = project_history_path_get(cp->pp);
	id = change_integration_directory_get(cp, 0);
	sub_var_set("Input", "%S/%S", id, filename);
	sub_var_set("History", "%S/%S", hp, filename);
	sub_var_set("1", "${input}");
	sub_var_set("2", "${history}");
	the_command = pconf_data->history_put_command;
	the_command = substitute(cp, the_command);
	project_become(cp->pp);
	os_execute(the_command, OS_EXEC_FLAG_NO_INPUT, id);
	project_become_undo();
	str_free(the_command);
	trace((/*{*/"}\n"));
}


string_ty *
change_run_history_query_command(cp, filename)
	change_ty	*cp;
	string_ty	*filename;
{
	string_ty	*hp;
	string_ty	*bl;
	pconf		pconf_data;
	string_ty	*the_command;
	string_ty	*result;

	/*
	 * Ask the history file what its edit number is.
	 * We use this method because the string
	 * returned is essentially random,
	 * between different history programs.
	 * All of the substitutions described in aesub(5) are available.
	 * In addition
	 *
	 * ${History}
	 *	absolute path of history file
	 */
	trace(("change_run_history_query_command(cp = %08lX, \
filename = \"%s\")\n{\n"/*}*/, (long)cp, filename->str_text));
	pconf_data = change_pconf_get(cp);
	hp = project_history_path_get(cp->pp);
	bl = project_baseline_path_get(cp->pp, 0);
	sub_var_set("History", "%S/%S", hp, filename);
	sub_var_set("1", "${history}");
	the_command = pconf_data->history_query_command;
	the_command = substitute(cp, the_command);
	project_become(cp->pp);
	result =
		os_execute_slurp
		(
			the_command,
			OS_EXEC_FLAG_NO_INPUT,
			bl
		);
	project_become_undo();
	str_free(the_command);
	if (!result->str_length)
	{
		fatal
		(
	  "the history_query_command returned the empty string, this is invalid"
		);
	}
	trace(("return \"%s\";\n", result->str_text));
	trace((/*{*/"}\n"));
	return result;
}


void
change_run_diff_command(cp, up, original, input, output)
	change_ty	*cp;
	user_ty		*up;
	string_ty	*original;
	string_ty	*input;
	string_ty	*output;
{
	pconf		pconf_data;
	string_ty	*dd;
	string_ty	*the_command;

	/*
	 * Run the diff_command.
	 * All of the substitutions described in aesub(5) are available.
	 * In addition
	 *
	 * ${Original}
	 *	absolute path of original file copied from the baseline
	 *	usually, but not always
	 *
	 * ${Input}
	 *	absolute path of current file in the development directory
	 *	usually, but not always
	 *
	 * ${Output}
	 *	absolute path of file in which to write the difference listing
	 *	usually in the development diretcory
	 */
	trace
	((
		"change_run_diff_command(cp = %08lX, up = %08lX, \
original = \"%s\", input = \"%s\", output = \"%s\")\n{\n"/*}*/,
		(long)cp,
		(long)up,
		original->str_text,
		input->str_text,
		output->str_text
	));
	pconf_data = change_pconf_get(cp);
	dd = change_development_directory_get(cp, 0);
	sub_var_set("ORiginal", "%S", original);
	sub_var_set("Input", "%S", input);
	sub_var_set("Output", "%S", output);
	sub_var_set("1", "${original}");
	sub_var_set("2", "${input}");
	sub_var_set("3", "${output}");
	the_command = pconf_data->diff_command;
	the_command = substitute(cp, the_command);
	trace_string(the_command->str_text);
	user_become(up);
	os_execute(the_command, OS_EXEC_FLAG_NO_INPUT, dd);
	user_become_undo();
	str_free(the_command);
	trace((/*{*/"}\n"));
}


void
change_run_diff3_command(cp, up, original, most_recent, input, output)
	change_ty	*cp;
	user_ty		*up;
	string_ty	*original;
	string_ty	*most_recent;
	string_ty	*input;
	string_ty	*output;
{
	pconf		pconf_data;
	string_ty	*the_command;
	string_ty	*dd;

	/*
	 * Run the diff3_command.
	 * All of the substitutions described in aesub(5) are available.
	 * In addition
	 *
	 * ${Original}
	 *	absolute path of original file copied from the baseline
	 *	usually somewhere in /tmp
	 *
	 * ${Most_Recent}
	 *	absolute path of original file currently in the baseline
	 *	usually, but not always
	 *
	 * ${Input}
	 *	absolute path of current file in the development directory
	 *	usually, but not always
	 *
	 * ${Output}
	 *	absolute path of file in which to write the difference listing
	 *	usually in the development diretcory
	 */
	trace
	((
		"change_run_diff3_command(cp = %08lX, up = %08lX, original = \
\"%s\", most_recent = \"%s\", input = \"%s\", output = \"%s\")\n{\n"/*}*/,
		(long)cp,
		(long)up,
		original->str_text,
		most_recent->str_text,
		input->str_text,
		output->str_text
	));
	pconf_data = change_pconf_get(cp);
	dd = change_development_directory_get(cp, 0);
	sub_var_set("ORiginal", "%S", original);
	sub_var_set("Most_Recent", "%S", most_recent);
	sub_var_set("Input", "%S", input);
	sub_var_set("Output", "%S", output);
	sub_var_set("1", "${original}");
	sub_var_set("2", "${most_recent}");
	sub_var_set("3", "${input}");
	sub_var_set("4", "${output}");
	the_command = pconf_data->diff3_command;
	the_command = substitute(cp, the_command);
	user_become(up);
	os_execute(the_command, OS_EXEC_FLAG_NO_INPUT, dd);
	user_become_undo();
	str_free(the_command);
	trace((/*{*/"}\n"));
}


void
change_run_develop_begin_command(cp, up)
	change_ty	*cp;
	user_ty		*up;
{
	pconf		pconf_data;
	string_ty	*the_command;
	string_ty	*dir;

	pconf_data = change_pconf_get(cp);
	if (!pconf_data->develop_begin_command)
		return;

	the_command = pconf_data->develop_begin_command;
	the_command = substitute(cp, the_command);
	dir = change_development_directory_get(cp, 1);
	user_become(up);
	os_execute
	(
		the_command,
		OS_EXEC_FLAG_NO_INPUT + OS_EXEC_FLAG_ERROK,
		dir
	);
	user_become_undo();
	str_free(the_command);
}


void
change_run_integrate_begin_command(cp)
	change_ty	*cp;
{
	pconf		pconf_data;
	string_ty	*the_command;
	string_ty	*dir;

	pconf_data = change_pconf_get(cp);
	if (!pconf_data->integrate_begin_command)
		return;
	sub_var_set("1", "${project}");
	sub_var_set("2", "${change}");
	sub_var_set("3", "${version}");
	the_command = pconf_data->integrate_begin_command;
	the_command = substitute(cp, the_command);
	dir = change_integration_directory_get(cp, 1);
	change_become(cp);
	os_execute
	(
		the_command,
		OS_EXEC_FLAG_NO_INPUT + OS_EXEC_FLAG_ERROK,
		dir
	);
	change_become_undo();
	str_free(the_command);
}

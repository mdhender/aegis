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
 * MANIFEST: functions to manipulate change state data
 */

#include <ctype.h>
#include <stdio.h>
#include <ac/stdlib.h>
#include <ac/string.h>
#include <ac/time.h>
#include <ac/stdarg.h>

#include <change.h>
#include <commit.h>
#include <dir.h>
#include <env.h>
#include <error.h>
#include <fstrcmp.h>
#include <lock.h>
#include <mem.h>
#include <option.h>
#include <os.h>
#include <project.h>
#include <sub.h>
#include <trace.h>
#include <uname.h>
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
	cp = (change_ty *)mem_alloc(sizeof(change_ty));
	cp->reference_count = 1;
	cp->pp = project_copy(pp);
	cp->number = number;
	cp->filename = project_change_path_get(pp, number);
	cp->cstate_data = 0;
	cp->development_directory = 0;
	cp->integration_directory = 0;
	cp->logfile = 0;
	cp->pconf_path = 0;
	cp->pconf_data = 0;
	cp->lock_magic = 0;
	cp->bogus = 0;
	cp->architecture_name = 0;
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
		d->history = cstate_history_list_type.alloc();
	if (!d->src)
		d->src = cstate_src_list_type.alloc();
	if (!(d->mask & cstate_regression_test_exempt_mask))
	{
		d->regression_test_exempt =
			(
				d->cause != change_cause_internal_improvement
			&&
				d->cause != change_cause_external_improvement
			);
	}
	if (!d->architecture)
		d->architecture = cstate_architecture_list_type.alloc();
	if (!d->architecture->length)
	{
		type_ty		*type_p;
		string_ty	**str_p;

		str_p =
			cstate_architecture_list_type.list_parse
			(
				d->architecture,
				&type_p
			);
		assert(type_p == &string_type);
		*str_p = str_from_c("unspecified");
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

				if
				(
					(
						cp->cstate_data->src->list[j]->
							mask
					&
						mask
					)
				!=
					mask
				)
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
	cp->cstate_data = cstate_type.alloc();
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

	trace(("change_src_find(cp = %08lX, file_name = \"%s\")\n{\n"/*}*/,
		(long)cp, file_name->str_text));
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


cstate_src
change_src_find_fuzzy(cp, file_name)
	change_ty	*cp;
	string_ty	*file_name;
{
	cstate		cstate_data;
	int		j;
	cstate_src	best;
	double		best_weight;
	double		weight;

	trace(("change_src_find_fuzzy(cp = %08lX, fn = \"%s\")\n{\n"/*}*/,
		(long)cp, file_name->str_text));
	cstate_data = change_cstate_get(cp);
	assert(cstate_data->src);
	best = 0;
	best_weight = 0.6;
	for (j = 0; j < cstate_data->src->length; ++j)
	{
		cstate_src	src_data;

		src_data = cstate_data->src->list[j];
		weight =
			fstrcmp
			(
				src_data->file_name->str_text,
				file_name->str_text
			);
		if (weight > best_weight)
		{
			best = src_data;
			best_weight = weight;
		}
	}
	trace(("return %08lX;\n", best));
	trace((/*{*/"}\n"));
	return best;
}


void
change_src_remove(cp, file_name)
	change_ty	*cp;
	string_ty	*file_name;
{
	cstate		cstate_data;
	int		j;
	cstate_src	src_data;

	trace(("change_src_remove(cp = %08lX, file_name = \"%s\")\n{\n"/*}*/,
		(long)cp, file_name->str_text));
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
	src_data_p = cstate_src_list_type.list_parse(cstate_data->src, &type_p);
	assert(type_p == &cstate_src_type);
	src_data = cstate_src_type.alloc();
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
	history_data_p =
		cstate_history_list_type.list_parse
		(
			cstate_data->history,
			&type_p
		);
	assert(type_p == &cstate_history_type);
	history_data = cstate_history_type.alloc();
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
	history_data = 0;
	for (pos = cstate_data->history->length - 1; pos >= 0 ; --pos)
	{
		history_data = cstate_data->history->list[pos];
		switch (history_data->what)
		{
		default:
			history_data = 0;
			continue;

		case cstate_history_what_review_fail:
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
	 *
	 * cp->development_directory is resolved
	 * cstate_data->development_directory is unresolved
	 */
	trace(("change_development_directory_set(cp = %08lX, s = \"%s\")\n{\n"
		/*}*/, cp, s->str_text));
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
	trace(("change_integration_directory_set(cp = %08lX, s = \"%s\")\n{\n"
		/*}*/, cp, s->str_text));
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
change_logfile_basename()
{
	static string_ty *s;

	if (!s)
		s = str_format("%.10s.log", option_progname_get());
	return s;
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
			str_format("%S/%S", s1, change_logfile_basename());
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
	if (cp->bogus)
		project_error(cp->pp, "%S", cp->number, msg);
	else
		project_error(cp->pp, "change %ld: %S", cp->number, msg);
	str_free(msg);
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
	if (cp->bogus)
		project_fatal(cp->pp, "%S", msg);
	else
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
	if (cp->bogus)
		project_verbose(cp->pp, "%S", msg);
	else
		project_verbose(cp->pp, "change %ld: %S", cp->number, msg);
	str_free(msg);
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
	switch (cstate_data->state)
	{
	default:
		cp->pconf_path =
			str_format
			(
				"%S/%S",
				project_baseline_path_get(cp->pp, 1),
				file_name
			);
		goto ret;

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
	}
	assert(cstate_data->src);
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


static void set_pconf_symlink_exceptions_defaults _((pconf));

static void
set_pconf_symlink_exceptions_defaults(pconf_data)
	pconf		pconf_data;
{
	type_ty		*type_p;
	string_ty	**str_p;

	/*
	 * make sure the list is there
	 */
	if (!pconf_data->symlink_exceptions)
		pconf_data->symlink_exceptions =
			pconf_symlink_exceptions_list_type.alloc();

	/*
	 * append the logfile to the list
	 */
	str_p =
		pconf_symlink_exceptions_list_type.list_parse
		(
			pconf_data->symlink_exceptions,
			&type_p
		);
	assert(type_p == &string_type);
	*str_p = str_copy(change_logfile_basename());
}


pconf
change_pconf_get(cp, required)
	change_ty	*cp;
	int		required;
{
	static string_ty *star_comma_d;
	size_t		j;

	trace(("change_pconf_get(cp = %08lX)\n{\n"/*}*/, cp));
	lock_sync(cp);
	if (!cp->pconf_data)
	{
		string_ty	*filename;

		filename = change_pconf_path_get(cp);
		change_become(cp);
		if (!os_exists(filename))
		{
			if (required)
			{
				change_fatal
				(
					cp,
					"you must create a \"%s\" file",
					THE_CONFIG_FILE
				);
			}
			cp->pconf_data = pconf_type.alloc();
		}
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

		/*
		 * set the architecture default
		 */
		if (!cp->pconf_data->architecture)
			cp->pconf_data->architecture =
				pconf_architecture_list_type.alloc();
		if (!cp->pconf_data->architecture->length)
		{
			type_ty		*type_p;
			pconf_architecture *app;
			pconf_architecture ap;

			app =
				pconf_architecture_list_type.list_parse
				(
					cp->pconf_data->architecture,
					&type_p
				);
			assert(type_p == &pconf_architecture_type);
			ap = pconf_architecture_type.alloc();
			*app = ap;
			ap->name = str_from_c("unspecified");
			ap->pattern = str_from_c("*");
			ap->mask =
				(
					pconf_architecture_name_mask
				|
					pconf_architecture_pattern_mask
				);
		}
		for (j = 0; j < cp->pconf_data->architecture->length; ++j)
		{
			if
			(
				cp->pconf_data->architecture->list[j]->mask
			!=
				(
					pconf_architecture_name_mask
				|
					pconf_architecture_pattern_mask
				)
			)
			{
				change_fatal
				(
					cp,
	     "%S: architectures must be specified as both a name and a pattern",
					filename
				);
			}
		}

		/*
		 * set the maximum_filename_length default
		 */
		if (cp->pconf_data->mask & pconf_maximum_filename_length_mask)
		{
			if (cp->pconf_data->maximum_filename_length < 9)
				cp->pconf_data->maximum_filename_length = 9;
			if (cp->pconf_data->maximum_filename_length > 255)
				cp->pconf_data->maximum_filename_length = 255;
		}
		else
			cp->pconf_data->maximum_filename_length = 14;

		/*
		 * set the filename_pattern_accept default
		 */
		if (!cp->pconf_data->filename_pattern_accept)
			cp->pconf_data->filename_pattern_accept =
				pconf_filename_pattern_accept_list_type.alloc();
		if (!cp->pconf_data->filename_pattern_accept->length)
		{
			type_ty		*type_p;
			string_ty	**addr_p;

			addr_p =
				pconf_filename_pattern_accept_list_type.list_parse
				(
					cp->pconf_data->filename_pattern_accept,
					&type_p
				);
			assert(type_p == &type_string);
			*addr_p = str_from_c("*");
		}
		if (!star_comma_d)
			star_comma_d = str_from_c("*,D");
		if (!cp->pconf_data->filename_pattern_reject)
			cp->pconf_data->filename_pattern_reject =
				pconf_filename_pattern_reject_list_type.alloc();
		for (j = 0; j < cp->pconf_data->filename_pattern_reject->length; ++j)
		{
			if
			(
				str_equal
				(
					star_comma_d,
					cp->pconf_data->filename_pattern_reject
						->list[j]
				)
			)
				break;
		}
		if (j >= cp->pconf_data->filename_pattern_reject->length)
		{
			type_ty		*type_p;
			string_ty	**addr_p;

			addr_p =
				pconf_filename_pattern_reject_list_type.list_parse
				(
					cp->pconf_data->filename_pattern_reject,
					&type_p
				);
			assert(type_p == &type_string);
			*addr_p = str_copy(star_comma_d);
		}

		/*
		 * make sure symlink_exceptions is there, even if empty
		 */
		set_pconf_symlink_exceptions_defaults(cp->pconf_data);

		/*
		 * set the test_command default
		 */
		if (!cp->pconf_data->test_command)
			cp->pconf_data->test_command =
				str_from_c("$shell $file_name");
		if (!cp->pconf_data->development_test_command)
			cp->pconf_data->development_test_command =
				str_copy(cp->pconf_data->test_command);
	}
	trace(("return %08lX;\n", cp->pconf_data));
	trace((/*{*/"}\n"));
	return cp->pconf_data;
}


static void change_env_set _((change_ty *));

static void
change_env_set(cp)
	change_ty	*cp;
{
	string_ty	*name;
	string_ty	*NAME;
	string_ty	*s;

	/*
	 * set the AEGIS_PROJECT environment cariable
	 */
	name = str_format("%s_PROJECT", option_progname_get());
	NAME = str_upcase(name);
	str_free(name);
	env_set(NAME->str_text, cp->pp->name->str_text);
	str_free(NAME);

	/*
	 * set the AEGIS_CHANGE environment cariable
	 */
	name = str_format("%s_CHANGE", option_progname_get());
	NAME = str_upcase(name);
	str_free(name);
	if (!cp->bogus)
	{
		char		buffer[20];

		sprintf(buffer, "%ld", cp->number);
		env_set(NAME->str_text, buffer);
	}
	else
		env_unset(NAME->str_text);
	str_free(NAME);

	/*
	 * set the AEGIS_ARCH environment cariable
	 */
	name = str_format("%s_ARCH", option_progname_get());
	NAME = str_upcase(name);
	str_free(name);
	s = change_architecture_name(cp);
	env_set(NAME->str_text, s->str_text);
	str_free(NAME);
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
	pconf_data = change_pconf_get(cp, 0);
	if (!pconf_data->change_file_command)
		goto ret;
	pstate_data = project_pstate_get(cp->pp);

	the_files = wl2str(wlp, 0, 32767, (char *)0);
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
	change_env_set(cp);
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
	pconf_data = change_pconf_get(cp, 0);
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
	change_env_set(cp);
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
			for (;;)
			{
				++formal;
				switch (*formal)
				{
				case 0:
					return 1;
				
				case '*':
					continue;

				case '?':
					if (!*actual++)
						return 0;
					continue;

				default:
					break;
				}
				break;
			}
			cp = actual + strlen(actual);
			for (;;)
			{
				result = gmatch(formal, cp);
				if (result)
				{
					result = 1;
					break;
				}
				--cp;
				if (cp < actual)
				{
					result = 0;
					break;
				}
			}
			goto ret;

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
							(
								c1 <= *actual
							&&
								*actual <= c2
							)
						:
							(
								c2 <= *actual
							&&
								*actual <= c1
							)
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
							(
								c1 <= *actual
							&&
								*actual <= c2
							)
						:
							(
								c2 <= *actual
							&&
								*actual <= c1
							)
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

	trace(("change_file_template(name = \"%s\")\n{\n"/*}*/,
		name->str_text));
	result = 0;
	pconf_data = change_pconf_get(cp, 0);
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
change_run_forced_develop_begin_notify_command(cp, up)
	change_ty	*cp;
	user_ty		*up;
{
	pstate		pstate_data;
	string_ty	*the_command;
	string_ty	*dd;

	/*
	 * make sure there is one
	 */
	trace(("change_run_forced_develop_begin_notify_command(cp = %08lX, \
up = %08lX)\n{\n"/*}*/, (long)cp, (long)up));
	pstate_data = project_pstate_get(cp->pp);
	if (!pstate_data->forced_develop_begin_notify_command)
		goto done;

	/*
	 * notify the change is ready for review
	 *	(it could be mail, or an internal bulletin board, etc)
	 * it happens after the data is written and the locks are released,
	 * so we don't much care if the command fails!
	 *
	 * All of the substitutions described in aesub(5) are available.
	 */
	the_command = pstate_data->forced_develop_begin_notify_command;
	the_command = substitute(cp, the_command);

	/*
	 * execute the command
	 */
	dd = change_development_directory_get(cp, 0);
	change_env_set(cp);
	user_become(up);
	os_execute(the_command, OS_EXEC_FLAG_NO_INPUT + OS_EXEC_FLAG_ERROK, dd);
	user_become_undo();
	str_free(the_command);

	/*
	 * here for all exits
	 */
	done:
	trace((/*{*/"}\n"));
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
	trace(("change_run_develop_end_notify_command(cp = %08lX)\n{\n"/*}*/,
		cp));
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
	change_env_set(cp);
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
	trace(("change_run_develop_end_undo_notify_command(cp = %08lX)\n{\n"
		/*}*/, cp));
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
	change_env_set(cp);
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
	trace(("change_run_integrate_fail_notify_command(cp = %08lX)\n{\n"/*}*/,
		cp));
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
	change_env_set(cp);
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
	trace(("change_run_integrate_pass_notify_command(cp = %08lX)\n{\n"/*}*/,
		cp));
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
	change_env_set(cp);
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
	trace(("change_run_review_pass_notify_command(cp = %08lX)\n{\n"/*}*/,
		cp));
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
	change_env_set(cp);
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
	trace(("change_run_review_pass_undo_notify_command(cp = %08lX)\n{\n"
		/*}*/, cp));
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
	change_env_set(cp);
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
	trace(("change_run_review_fail_notify_command(cp = %08lX)\n{\n"/*}*/,
		cp));
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
	change_env_set(cp);
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
	pconf_data = change_pconf_get(cp, 1);
	the_command = pconf_data->history_get_command;
	if (!the_command)
	{
		change_fatal
		(
			cp,
	     "the history_get_command field must be defined in the \"%s\" file",
			THE_CONFIG_FILE
		);
	}
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
	change_env_set(cp);
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
	pconf_data = change_pconf_get(cp, 1);
	hp = project_history_path_get(cp->pp);
	id = change_integration_directory_get(cp, 0);
	sub_var_set("Input", "%S/%S", id, filename);
	sub_var_set("History", "%S/%S", hp, filename);
	sub_var_set("1", "${input}");
	sub_var_set("2", "${history}");
	the_command = pconf_data->history_create_command;
	if (!the_command)
	{
		change_fatal
		(
			cp,
	  "the history_create_command field must be defined in the \"%s\" file",
			THE_CONFIG_FILE
		);
	}
	the_command = substitute(cp, the_command);
	change_env_set(cp);
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
	pconf_data = change_pconf_get(cp, 1);
	hp = project_history_path_get(cp->pp);
	id = change_integration_directory_get(cp, 0);
	sub_var_set("Input", "%S/%S", id, filename);
	sub_var_set("History", "%S/%S", hp, filename);
	sub_var_set("1", "${input}");
	sub_var_set("2", "${history}");
	the_command = pconf_data->history_put_command;
	if (!the_command)
	{
		change_fatal
		(
			cp,
	     "the history_put_command field must be defined in the \"%s\" file",
			THE_CONFIG_FILE
		);
	}
	the_command = substitute(cp, the_command);
	change_env_set(cp);
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
	pconf_data = change_pconf_get(cp, 1);
	hp = project_history_path_get(cp->pp);
	bl = project_baseline_path_get(cp->pp, 0);
	sub_var_set("History", "%S/%S", hp, filename);
	sub_var_set("1", "${history}");
	the_command = pconf_data->history_query_command;
	if (!the_command)
	{
		change_fatal
		(
			cp,
	   "the history_query_command field must be defined in the \"%s\" file",
			THE_CONFIG_FILE
		);
	}
	the_command = substitute(cp, the_command);
	change_env_set(cp);
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
	pconf_data = change_pconf_get(cp, 1);
	dd = change_development_directory_get(cp, 0);
	sub_var_set("ORiginal", "%S", original);
	sub_var_set("Input", "%S", input);
	sub_var_set("Output", "%S", output);
	sub_var_set("1", "${original}");
	sub_var_set("2", "${input}");
	sub_var_set("3", "${output}");
	the_command = pconf_data->diff_command;
	if (!the_command)
	{
		change_fatal
		(
			cp,
		    "the diff_command field must be defined in the \"%s\" file",
			THE_CONFIG_FILE
		);
	}
	the_command = substitute(cp, the_command);
	trace_string(the_command->str_text);
	change_env_set(cp);
	user_become(up);
	if (os_exists(output))
		os_unlink(output);
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
	pconf_data = change_pconf_get(cp, 1);
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
	if (!the_command)
	{
		change_fatal
		(
			cp,
		   "the diff3_command field must be defined in the \"%s\" file",
			THE_CONFIG_FILE
		);
	}
	the_command = substitute(cp, the_command);
	change_env_set(cp);
	user_become(up);
	if (os_exists(output))
		os_unlink(output);
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

	pconf_data = change_pconf_get(cp, 0);
	if (!pconf_data->develop_begin_command)
		return;

	the_command = pconf_data->develop_begin_command;
	the_command = substitute(cp, the_command);
	dir = change_development_directory_get(cp, 1);
	change_env_set(cp);
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

	pconf_data = change_pconf_get(cp, 0);
	if (!pconf_data->integrate_begin_command)
		return;
	sub_var_set("1", "${project}");
	sub_var_set("2", "${change}");
	sub_var_set("3", "${version}");
	the_command = pconf_data->integrate_begin_command;
	the_command = substitute(cp, the_command);
	dir = change_integration_directory_get(cp, 1);
	change_env_set(cp);
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


void
change_architecture_clear(cp)
	change_ty	*cp;
{
	cstate		cstate_data;

	cstate_data = change_cstate_get(cp);
	cstate_architecture_list_type.free(cstate_data->architecture);
	cstate_data->architecture = cstate_architecture_list_type.alloc();
}


void
change_architecture_add(cp, name)
	change_ty	*cp;
	string_ty	*name;
{
	cstate		cstate_data;
	type_ty		*type_p;
	string_ty	**who_p;

	cstate_data = change_cstate_get(cp);
	who_p =
		cstate_architecture_list_type.list_parse
		(
			cstate_data->architecture,
			&type_p
		);
	assert(type_p == &string_type);
	*who_p = str_copy(name);
}


string_ty *
change_architecture_name(cp)
	change_ty	*cp;
{
	trace(("change_architecture_variant(cp = %08lX)\n{\n"/*}*/, (long)cp));
	if (!cp->architecture_name)
	{
		pconf		pconf_data;
		long		j;
		string_ty	*result;
		string_ty	*un;

		pconf_data = change_pconf_get(cp, 0);
		assert(pconf_data->architecture);
		un = uname_variant_get();
		result = 0;
		for (j = 0; j < pconf_data->architecture->length; ++j)
		{
			pconf_architecture ap;

			ap = pconf_data->architecture->list[j];
			assert(ap->name);
			assert(ap->pattern);
			if (gmatch(ap->pattern->str_text, un->str_text))
			{
				result = ap->name;
				break;
			}
		}
		if (!result)
			change_fatal(cp, "architecture \"%S\" unknown", un);
		cp->architecture_name = result;
	}
	trace_string(cp->architecture_name->str_text);
	trace((/*{*/"}\n"));
	return cp->architecture_name;
}


cstate_architecture_times
change_architecture_times_find(cp, un)
	change_ty	*cp;
	string_ty	*un;
{
	cstate		cstate_data;
	long		j;
	cstate_architecture_times tp;

	/*
	 * find this variant in the times list
	 */
	trace(("change_architecture_times_find(cp = %08lX, un = %08lX)\n{\n"
		/*}*/, (long)cp, (long)un));
	cstate_data = change_cstate_get(cp);
	if (!cstate_data->architecture_times)
		cstate_data->architecture_times =
			cstate_architecture_times_list_type.alloc();
	for (j = 0; j < cstate_data->architecture_times->length; ++j)
	{
		if
		(
			str_equal
			(
				un,
				cstate_data->architecture_times->list[j]->
					variant
			)
		)
			break;
	}
	if (j >= cstate_data->architecture_times->length)
	{
		type_ty		*type_p;
		cstate_architecture_times *data_p;

		data_p =
			cstate_architecture_times_list_type.list_parse
			(
				cstate_data->architecture_times,
				&type_p
			);
		assert(type_p == &cstate_architecture_times_type);
		tp = cstate_architecture_times_type.alloc();
		*data_p = tp;
		tp->variant = str_copy(un);
	}
	else
		tp = cstate_data->architecture_times->list[j];
	trace(("return %08lX;\n", (long)tp));
	trace((/*{*/"}\n"));
	return tp;
}


static cstate_architecture_times find_architecture_variant _((change_ty *cp));

static cstate_architecture_times
find_architecture_variant(cp)
	change_ty	*cp;
{
	string_ty	*an;
	cstate_architecture_times tp;

	/*
	 * find the name of the architecture variant
	 *	one of the patterns, not the actual value in architecture
	 */
	trace(("find_architecture_variant(cp = %08lX)\n{\n"/*}*/, (long)cp));
	an = change_architecture_name(cp);

	/*
	 * find this variant in the times list
	 */
	tp = change_architecture_times_find(cp, an);

	/*
	 * adjust the node
	 */
	trace(("mark\n"));
	str_free(tp->node);
	tp->node = str_copy(uname_node_get());
	trace(("return %08lX;\n", (long)tp));
	trace((/*{*/"}\n"));
	return tp;
}


void
change_build_time_set(cp)
	change_ty	*cp;
{
	long		j, k;
	cstate_architecture_times tp;
	cstate		cstate_data;

	/*
	 * set the build_time in the architecture variant record
	 */
	trace(("change_build_time_set(cp = %08lX)\n{\n"/*}*/, (long)cp));
	tp = find_architecture_variant(cp);
	time(&tp->build_time);
	tp->test_time = 0;
	tp->test_baseline_time = 0;
	tp->regression_test_time = 0;

	/*
	 * set the build_time in the change state.
	 * figure the oldest time of all variants.
	 * if one is missing, then is zero.
	 */
	trace(("mark\n"));
	cstate_data = change_cstate_get(cp);
	cstate_data->build_time = tp->build_time;
	for (j = 0; j < cstate_data->architecture->length; ++j)
	{
		for (k = 0; k < cstate_data->architecture_times->length; ++k)
		{
			tp = cstate_data->architecture_times->list[k];
			if
			(
				str_equal
				(
					cstate_data->architecture->list[j],
					tp->variant
				)
			)
				break;
		}
		if (k >= cstate_data->architecture_times->length)
		{
			cstate_data->build_time = 0;
			break;
		}
		if (tp->build_time < cstate_data->build_time)
			cstate_data->build_time = tp->build_time;
	}
	cstate_data->test_time = 0;
	cstate_data->test_baseline_time = 0;
	cstate_data->regression_test_time = 0;
	trace((/*{*/"}\n"));
}


void
change_test_time_set(cp)
	change_ty	*cp;
{
	long		j, k;
	cstate_architecture_times tp;
	cstate		cstate_data;

	/*
	 * set the test_time in the architecture variant record
	 */
	tp = find_architecture_variant(cp);
	time(&tp->test_time);

	/*
	 * set the test_time in the change state.
	 * figure the oldest time of all variants.
	 * if one is missing, then is zero.
	 */
	cstate_data = change_cstate_get(cp);
	cstate_data->test_time = tp->test_time;
	for (j = 0; j < cstate_data->architecture->length; ++j)
	{
		for (k = 0; k < cstate_data->architecture_times->length; ++k)
		{
			tp = cstate_data->architecture_times->list[k];
			if
			(
				str_equal
				(
					cstate_data->architecture->list[j],
					tp->variant
				)
			)
				break;
		}
		if (k >= cstate_data->architecture_times->length)
		{
			cstate_data->test_time = 0;
			break;
		}
		if (tp->test_time < cstate_data->test_time)
			cstate_data->test_time = tp->test_time;
	}
}


void
change_test_baseline_time_set(cp)
	change_ty	*cp;
{
	long		j, k;
	cstate_architecture_times tp;
	cstate		cstate_data;

	/*
	 * set the test_baseline_time in the architecture variant record
	 */
	tp = find_architecture_variant(cp);
	time(&tp->test_baseline_time);

	/*
	 * set the test_baseline_time in the change state.
	 * figure the oldest time of all variants.
	 * if one is missing, then is zero.
	 */
	cstate_data = change_cstate_get(cp);
	cstate_data->test_baseline_time = tp->test_baseline_time;
	for (j = 0; j < cstate_data->architecture->length; ++j)
	{
		for (k = 0; k < cstate_data->architecture_times->length; ++k)
		{
			tp = cstate_data->architecture_times->list[k];
			if
			(
				str_equal
				(
					cstate_data->architecture->list[j],
					tp->variant
				)
			)
				break;
		}
		if (k >= cstate_data->architecture_times->length)
		{
			cstate_data->test_baseline_time = 0;
			break;
		}
		if (tp->test_baseline_time < cstate_data->test_baseline_time)
			cstate_data->test_baseline_time =
				tp->test_baseline_time;
	}
}


void
change_regression_test_time_set(cp)
	change_ty	*cp;
{
	long		j, k;
	cstate_architecture_times tp;
	cstate		cstate_data;

	/*
	 * set the regression_test_time in the architecture variant record
	 */
	tp = find_architecture_variant(cp);
	time(&tp->regression_test_time);

	/*
	 * set the regression_test_time in the change state.
	 * figure the oldest time of all variants.
	 * if one is missing, then is zero.
	 */
	cstate_data = change_cstate_get(cp);
	cstate_data->regression_test_time = tp->regression_test_time;
	for (j = 0; j < cstate_data->architecture->length; ++j)
	{
		for (k = 0; k < cstate_data->architecture_times->length; ++k)
		{
			tp = cstate_data->architecture_times->list[k];
			if
			(
				str_equal
				(
					cstate_data->architecture->list[j],
					tp->variant
				)
			)
				break;
		}
		if (k >= cstate_data->architecture_times->length)
		{
			cstate_data->regression_test_time = 0;
			break;
		}
		if
		(
			tp->regression_test_time
		<
			cstate_data->regression_test_time
		)
			cstate_data->regression_test_time =
				tp->regression_test_time;
	}
}


void
change_test_times_clear(cp)
	change_ty	*cp;
{
	cstate_architecture_times tp;
	cstate		cstate_data;

	/*
	 * reset the test times in the architecture variant record
	 */
	tp = find_architecture_variant(cp);
	tp->test_time = 0;
	tp->test_baseline_time = 0;
	tp->regression_test_time = 0;

	/*
	 * reset the test times in the change state.
	 */
	cstate_data = change_cstate_get(cp);
	cstate_data->test_time = 0;
	cstate_data->test_baseline_time = 0;
	cstate_data->regression_test_time = 0;
}


void
change_build_times_clear(cp)
	change_ty	*cp;
{
	cstate		cstate_data;

	/*
	 * reset the build and test times in all architecture variant records
	 */
	cstate_data = change_cstate_get(cp);
	if (cstate_data->architecture_times)
	{
		cstate_architecture_times_list_type.free
		(
			cstate_data->architecture_times
		);
		cstate_data->architecture_times = 0;
	}

	/*
	 * reset the test times in the change state
	 */
	cstate_data->build_time = 0;
	cstate_data->test_time = 0;
	cstate_data->test_baseline_time = 0;
	cstate_data->regression_test_time = 0;
}


static char *outstanding_commentary _((wlist *, long));

static char *
outstanding_commentary(wlp, narch)
	wlist		*wlp;
	long		narch;
{
	static string_ty *s;

	if (s)
	{
		str_free(s);
		s = 0;
	}
	if (!wlp->wl_nwords || wlp->wl_nwords >= narch)
		s = str_from_c("");
	else if (wlp->wl_nwords == 1)
		s = str_format(" for the \"%S\" architecture", wlp->wl_word[0]);
	else
	{
		string_ty	*t1;
		string_ty	*t2;
		long		j;

		t1 = str_format(" for the \"%S\"", wlp->wl_word[0]);
		for (j = 1; j < wlp->wl_nwords; ++j)
		{
			t2 =
				str_format
				(
					"%S%s \"%S\"",
					t1,
					j == wlp->wl_nwords - 1 ? " and" : ",",
					wlp->wl_word[j]
				);
			str_free(t1);
			t1 = t2;
		}
		s = str_format("%S architectures", t1);
		str_free(t1);
	}
	wl_free(wlp);
	return s->str_text;
}


char *
change_outstanding_builds(cp, t)
	change_ty	*cp;
	time_t		t;
{
	cstate		cstate_data;
	wlist		wl;
	cstate_architecture_times tp;
	long		j;

	cstate_data = change_cstate_get(cp);
	wl_zero(&wl);
	for (j = 0; j < cstate_data->architecture->length; ++j)
	{
		tp =
			change_architecture_times_find
			(
				cp,
				cstate_data->architecture->list[j]
			);
		if (!tp->build_time || tp->build_time < t)
			wl_append_unique(&wl, tp->variant);
	}
	return outstanding_commentary(&wl, cstate_data->architecture->length);
}


char *
change_outstanding_tests(cp, t)
	change_ty	*cp;
	time_t		t;
{
	cstate		cstate_data;
	wlist		wl;
	cstate_architecture_times tp;
	long		j;

	cstate_data = change_cstate_get(cp);
	wl_zero(&wl);
	for (j = 0; j < cstate_data->architecture->length; ++j)
	{
		tp =
			change_architecture_times_find
			(
				cp,
				cstate_data->architecture->list[j]
			);
		if
		(
			!tp->test_time
		||
			tp->test_time < t
		||
			!tp->build_time
		||
			tp->build_time < t
		)
			wl_append_unique(&wl, tp->variant);
	}
	return outstanding_commentary(&wl, cstate_data->architecture->length);
}


char *
change_outstanding_tests_baseline(cp, t)
	change_ty	*cp;
	time_t		t;
{
	cstate		cstate_data;
	wlist		wl;
	cstate_architecture_times tp;
	long		j;

	cstate_data = change_cstate_get(cp);
	wl_zero(&wl);
	for (j = 0; j < cstate_data->architecture->length; ++j)
	{
		tp =
			change_architecture_times_find
			(
				cp,
				cstate_data->architecture->list[j]
			);
		if
		(
			!tp->test_baseline_time
		||
			tp->test_baseline_time < t
		||
			!tp->build_time
		||
			tp->build_time < t
		)
			wl_append_unique(&wl, tp->variant);
	}
	return outstanding_commentary(&wl, cstate_data->architecture->length);
}


char *
change_outstanding_tests_regression(cp, t)
	change_ty	*cp;
	time_t		t;
{
	cstate		cstate_data;
	wlist		wl;
	cstate_architecture_times tp;
	long		j;

	cstate_data = change_cstate_get(cp);
	wl_zero(&wl);
	for (j = 0; j < cstate_data->architecture->length; ++j)
	{
		tp =
			change_architecture_times_find
			(
				cp,
				cstate_data->architecture->list[j]
			);
		if
		(
			!tp->regression_test_time
		||
			tp->regression_test_time < t
		||
			!tp->build_time
		||
			tp->build_time < t
		)
			wl_append_unique(&wl, tp->variant);
	}
	return outstanding_commentary(&wl, cstate_data->architecture->length);
}


void
change_architecture_from_pconf(cp)
	change_ty	*cp;
{
	pconf		pconf_data;
	long		j;

	pconf_data = change_pconf_get(cp, 0);
	change_architecture_clear(cp);
	for (j = 0; j < pconf_data->architecture->length; ++j)
	{
		change_architecture_add
		(
			cp,
			pconf_data->architecture->list[j]->name
		);
	}
}


static int change_maximum_filename_length _((change_ty *));

static int
change_maximum_filename_length(cp)
	change_ty	*cp;
{
	pconf		pconf_data;

	pconf_data = change_pconf_get(cp, 0);
	assert(pconf_data->maximum_filename_length);
	return pconf_data->maximum_filename_length;
}


static int is_a_dos_filename _((change_ty *, string_ty *));

static int
is_a_dos_filename(cp, fn)
	change_ty	*cp;
	string_ty	*fn;
{
	pconf		pconf_data;
	wlist		wl;
	int		result;
	size_t		j;

	pconf_data = change_pconf_get(cp, 0);
	if (!pconf_data->dos_filename_required)
		return 1;

	/*
	 * make sure the filename
	 *	1. contains at most one dot
	 *	2. contains a non-empty prefix of at most 8 characters
	 *	3. optionally contains a non-empty suffix of at
	 *	   most 3 characters (reject empty suffix, because DOS
	 *	   will think that "foo" and "foo." are the same name)
	 */
	str2wl(&wl, fn, ".", 0);
	result = 0;
	if (wl.wl_nwords < 1 || wl.wl_nwords > 2)
		goto done;
	if (wl.wl_word[0]->str_length < 1 || wl.wl_word[0]->str_length > 8)
		goto done;
	if
	(
		wl.wl_nwords > 1
	&&
		(wl.wl_word[1]->str_length < 1 || wl.wl_word[1]->str_length > 3)
	)
		goto done;

	/*
	 * make sure the characters are acceptable
	 * only allow alphanumerics
	 */
	for (j = 0; j < wl.wl_nwords; ++j)
	{
		char	*tp;

		for (tp = wl.wl_word[j]->str_text; *tp; ++tp)
		{
			if (!isalnum((unsigned char)*tp))
				goto done;
		}
	}
	result = 1;
	
	done:
	wl_free(&wl);
	return result;
}


static int filename_pattern_test _((change_ty *, string_ty *));

static int
filename_pattern_test(cp, fn)
	change_ty	*cp;
	string_ty	*fn;
{
	pconf		pconf_data;
	size_t		j;

	pconf_data = change_pconf_get(cp, 0);

	/*
	 * check rejections first
	 */
	assert(pconf_data->filename_pattern_reject);
	for (j = 0; j < pconf_data->filename_pattern_reject->length; ++j)
	{
		string_ty	*s;

		s = pconf_data->filename_pattern_reject->list[j];
		if (gmatch(s->str_text, fn->str_text))
			return 0;
	}

	/*
	 * check acceptable patterns
	 *  (defaults to "*" if not set, or set but empty)
	 */
	assert(pconf_data->filename_pattern_reject);
	assert(pconf_data->filename_pattern_reject->length);
	for (j = 0; j < pconf_data->filename_pattern_reject->length; ++j)
	{
		string_ty	*s;

		s = pconf_data->filename_pattern_accept->list[j];
		if (gmatch(s->str_text, fn->str_text))
			return 1;
	}

	/*
	 * did not match any accept pattern
	 */
	return 0;
}


int
change_pathconf_name_max(cp)
	change_ty	*cp;
{
	string_ty	*bl;
	int		bl_max;
	string_ty	*dd;
	int		dd_max;

	bl = project_baseline_path_get(cp->pp, 0);
	dd = change_development_directory_get(cp, 0);
	change_become(cp);
	bl_max = os_pathconf_name_max(bl);
	dd_max = os_pathconf_name_max(dd);
	change_become_undo();
	return (bl_max < dd_max ? bl_max : dd_max);
}


static int change_filename_in_charset _((change_ty *, string_ty *));

static int
change_filename_in_charset(cp, fn)
	change_ty	*cp;
	string_ty	*fn;
{
	pconf		pconf_data;
	char		*s;

	pconf_data = change_pconf_get(cp, 0);
	if (pconf_data->posix_filename_charset)
	{
		/*
		 * For a filename to be portable across conforming
		 * implementations of IEEE Std 1003.1-1988, it shall
		 * consist only of the following characters.  Hyphen
		 * shall not be used as the first character of a
		 * portable filename.
		 */
		if (fn->str_text[0] == '-')
			return 0;
		for (s = fn->str_text; *s; ++s)
		{
			switch (*s)
			{
			default:
				return 0;

			case 'a': case 'b': case 'c': case 'd': case 'e':
			case 'f': case 'g': case 'h': case 'i': case 'j':
			case 'k': case 'l': case 'm': case 'n': case 'o':
			case 'p': case 'q': case 'r': case 's': case 't':
			case 'u': case 'v': case 'w': case 'x': case 'y':
			case 'z':
			case 'A': case 'B': case 'C': case 'D': case 'E':
			case 'F': case 'G': case 'H': case 'I': case 'J':
			case 'K': case 'L': case 'M': case 'N': case 'O':
			case 'P': case 'Q': case 'R': case 'S': case 'T':
			case 'U': case 'V': case 'W': case 'X': case 'Y':
			case 'Z':
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
			case '-': case '.': case '_':
				break;
			}
		}
	}
	else
	{
		/*
		 * exclude control characters, space characters
		 * and high-bit-on characters
		 */
		for (s = fn->str_text; *s; ++s)
			if (!isgraph((unsigned char)*s))
				return 0;
		/*
		 * also disallow a leading minus
		 * they only make trouble
		 */
		if (fn->str_text[0] == '-')
			return 0;
	}
	return 1;
}


/*
 * NAME
 *	change_filename_check
 *
 * SYNOPSIS
 *	string_ty *change_filename_check(change_ty *cp, string_ty *fn);
 *
 * DESCRIPTION
 *      The change_filename_check function is used to determine if a
 *      filename is acceptable, accirding to the various filename
 *	constraints.
 *	1. it must not have any element which is too long
 *	2. it must not have any element which ends in ,D
 *	3. no directory may duplicate an existing file
 *	4. you can't call it the name of the default log file
 *	5. it must not have any illegal characters
 *
 * ARGUMENTS
 *	cp	- the change the filename is related to
 *	fn	- the filename to check
 *
 * RETURNS
 *	0 on success, otherwise a pointer to an error message.
 *	The error message will need to be freed after printing.
 */

string_ty *
change_filename_check(cp, filename, nodup)
	change_ty	*cp;
	string_ty	*filename;
	int		nodup;
{
	int		name_max1;
	int		name_max2;
	wlist		part;
	int		k;
	string_ty	*result;
	string_ty	*s2;

	/*
	 * figure the limits
	 * name_max1 is for directories
	 * name_max2 is for the basename
	 *
	 * The margin of 2 on the end is for ",D" suffixes,
	 * and for ",v" in RCS, "s." in SCCS, etc.
	 */
	name_max1 = change_maximum_filename_length(cp);
	name_max2 = change_pathconf_name_max(cp);
	if (name_max1 > name_max2)
		name_max1 = name_max2;
	name_max2 -= 2;
	if (name_max2 > name_max1)
		name_max2 = name_max1;
	
	/*
	 * break into path elements
	 * and check each element
	 */
	str2wl(&part, filename, "/", 0);
	result = 0;
	for (k = 0; k < part.wl_nwords; ++k)
	{
		int		max;

		/*
		 * check for collision
		 */
		if (nodup)
		{
			pstate_src	src_data;

			s2 = wl2str(&part, 0, k, "/");
			src_data = project_src_find(cp->pp, s2);
			if
			(
				src_data
			&&
				!src_data->deleted_by
			&&
				!src_data->about_to_be_created_by
			)
			{
				if
				(
					part.wl_nwords == 1
				||
					str_equal(s2, filename)
				)
					result =
						str_format
						(
					"file \"%S\" already exists in project",
							filename
						);
				else
					result =
						str_format
						(
			     "file \"%S\" collides with file \"%S\" in project",
							filename,
							s2
						);
				str_free(s2);
				goto done;
			}
			str_free(s2);
		}

		/*
		 * check name length
		 */
		s2 = part.wl_word[k];
		if (k == part.wl_nwords - 1)
			max = name_max2;
		else
			max = name_max1;
		if (s2->str_length > max)
		{
			if (part.wl_nwords == 1)
				result =
					str_format
					(
						"file \"%S\" too long (by %d)",
						filename,
						(int)(s2->str_length - max)
					);
			else
				result =
					str_format
					(
			     "file \"%S\" path element \"%S\" too long (by %d)",
						filename,
						s2,
						(int)(s2->str_length - max)
					);
			goto done;
		}

		/*
		 * check for logfile
		 */
		if (str_equal(s2, change_logfile_basename()))
		{
			if (part.wl_nwords == 1)
				result =
					str_format
					(
					 "file \"%S\" duplicates log file name",
						filename
					);
			else
				result =
					str_format
					(
		     "file \"%S\" path element \"%S\" duplicates log file name",
						filename,
						s2
					);
			goto done;
		}

		/*
		 * check filename for valid characters
		 */
		if (!change_filename_in_charset(cp, s2))
		{
			if (part.wl_nwords == 1)
				result =
					str_format
					(
				 "file name \"%S\" contains illegal characters",
						filename
					);
			else
				result =
					str_format
					(
	     "file \"%S\" path element name \"%S\" contains illegal characters",
						filename,
						s2
					);
			goto done;
		}
		if (!is_a_dos_filename(cp, s2))
		{
			if (part.wl_nwords == 1)
				result =
					str_format
					(
				     "file name \"%S\" is not suitable for DOS",
						filename
					);
			else
				result =
					str_format
					(
		 "file \"%S\" path element name \"%S\" is not suitable for DOS",
						filename,
						s2
					);
			goto done;
		}
		if (!filename_pattern_test(cp, s2))
		{
			if (part.wl_nwords == 1)
				result =
					str_format
					(
"file name \"%S\" \
is not acceptable to the filename pattern filters",
						filename
					);
			else
				result =
					str_format
					(
"file \"%S\" path element name \"%S\" \
is not acceptable to the filename pattern filters",
						filename,
						s2
					);
			goto done;
		}
	}

	/*
	 * here for all exits
	 */
	done:
	wl_free(&part);
	return result;
}


static void csltbl1 _((void *, dir_walk_message_ty, string_ty *,
	struct stat *));

static void
csltbl1(p, msg, path, st)
	void		*p;
	dir_walk_message_ty msg;
	string_ty	*path;
	struct stat	*st;
{
	change_ty	*cp;
	string_ty	*s1;
	string_ty	*s2;
	string_ty	*bl;
	string_ty	*dd;
	pconf		pconf_data;
	long		j;

	cp = p;
	bl = project_baseline_path_get(cp->pp, 0);
	s1 = os_below_dir(bl, path);
	assert(s1);
	dd = change_development_directory_get(cp, 0);
	s2 = str_format("%S/%S", dd, s1);
	switch (msg)
	{
	case dir_walk_dir_before:
		if (!os_exists(s2))
			os_mkdir(s2, 02755);
		break;

	case dir_walk_dir_after:
		break;

	case dir_walk_file:
	case dir_walk_special:
	case dir_walk_symlink:
		if (os_exists(s2))
			break;
		pconf_data = change_pconf_get(cp, 0);
		for (j = 0; j < pconf_data->symlink_exceptions->length; ++j)
		{
			if
			(
				str_equal
				(
					s1,
					pconf_data->symlink_exceptions->list[j]
				)
			)
				break;
		}
		if (j < pconf_data->symlink_exceptions->length)
			break;
		os_symlink(path, s2);
		break;
	}
	str_free(s1);
	str_free(s2);
}


static void csltbl2 _((void *, dir_walk_message_ty, string_ty *,
	struct stat *));

static void
csltbl2(p, msg, path, st)
	void		*p;
	dir_walk_message_ty msg;
	string_ty	*path;
	struct stat	*st;
{
	change_ty	*cp;
	string_ty	*s1;

	/*
	 * walk the development directory,
	 * removing symlinks which point
	 * to non-existent files
	 */
	cp = p;
	switch (msg)
	{
	case dir_walk_dir_before:
	case dir_walk_dir_after:
	case dir_walk_file:
	case dir_walk_special:
		break;

	case dir_walk_symlink:
		s1 = os_readlink(path);
		if (s1->str_text[0] == '/' && !os_exists(s1))
			os_unlink(path);
		str_free(s1);
		break;
	}
}


void
change_create_symlinks_to_baseline(cp, up)
	change_ty	*cp;
	user_ty		*up;
{
	string_ty	*bl;
	string_ty	*dd;

	trace(("change_create_symlinks_to_baseline(cp = %08lX)\n{\n"/*}*/,
		(long)cp));
	change_verbose(cp, "creating symbolic links to baseline");
	bl = project_baseline_path_get(cp->pp, 0);
	dd = change_development_directory_get(cp, 0);
	user_become(up);
	dir_walk(bl, csltbl1, cp);
	dir_walk(dd, csltbl2, cp);
	user_become_undo();
	trace((/*{*/"}\n"));
}


static void rsltbl _((void *, dir_walk_message_ty, string_ty *, struct stat *));

static void
rsltbl(p, msg, path, st)
	void		*p;
	dir_walk_message_ty msg;
	string_ty	*path;
	struct stat	*st;
{
	change_ty	*cp;
	string_ty	*bl;
	string_ty	*dd;
	string_ty	*s1;
	string_ty	*s2;

	/*
	 * remove symlinks in the development directory
	 * which point to their counterpart in the baseline
	 */
	if (msg != dir_walk_symlink)
		return;
	cp = p;
	dd = change_development_directory_get(cp, 0);
	bl = project_baseline_path_get(cp->pp, 0);
	s1 = os_below_dir(dd, path);
	s2 = str_format("%S/%S", bl, s1);
	str_free(s1);
	s1 = os_readlink(path);
	if (str_equal(s1, s2))
		os_unlink(path);
	str_free(s1);
	str_free(s2);
}


void
change_remove_symlinks_to_baseline(cp, up)
	change_ty	*cp;
	user_ty		*up;
{
	string_ty	*bl;
	string_ty	*dd;

	trace(("change_remove_symlinks_to_baseline(cp = %08lX)\n{\n"/*}*/,
		(long)cp));
	change_verbose(cp, "removing symbolic links to baseline");
	/*
	 * get the baseline now, so when get inside walk func,
	 * do not cause problem with multiple user-id setting.
	 */
	bl = project_baseline_path_get(cp->pp, 0);
	dd = change_development_directory_get(cp, 0);
	user_become(up);
	dir_walk(dd, rsltbl, cp);
	user_become_undo();
	trace((/*{*/"}\n"));
}


static int run_test_command _((change_ty *, user_ty *, string_ty *,
	string_ty *, int, string_ty *));

static int
run_test_command(cp, up, filename, dir, inp, the_command)
	change_ty	*cp;
	user_ty		*up;
	string_ty	*filename;
	string_ty	*dir;
	int		inp;
	string_ty	*the_command;
{
	int		flags;
	int		result;

	sub_var_set("File_Name", "%S", filename);
	the_command = substitute(cp, the_command);
	flags = inp ? OS_EXEC_FLAG_INPUT : OS_EXEC_FLAG_NO_INPUT;
	change_env_set(cp);
	user_become(up);
	result = os_execute_retcode(the_command, flags, dir);
	os_become_undo();
	str_free(the_command);
	return result;
}


int
change_run_test_command(cp, up, filename, dir, inp)
	change_ty	*cp;
	user_ty		*up;
	string_ty	*filename;
	string_ty	*dir;
	int		inp;
{
	pconf		pconf_data;
	string_ty	*the_command;

	pconf_data = change_pconf_get(cp, 0);
	assert(pconf_data);
	the_command = pconf_data->test_command;
	assert(the_command);
	return run_test_command(cp, up, filename, dir, inp, the_command);
}


int
change_run_development_test_command(cp, up, filename, dir, inp)
	change_ty	*cp;
	user_ty		*up;
	string_ty	*filename;
	string_ty	*dir;
	int		inp;
{
	pconf		pconf_data;
	string_ty	*the_command;

	pconf_data = change_pconf_get(cp, 0);
	assert(pconf_data);
	the_command = pconf_data->development_test_command;
	assert(the_command);
	return run_test_command(cp, up, filename, dir, inp, the_command);
}


void
change_run_build_command(cp)
	change_ty	*cp;
{
	pconf		pconf_data;
	string_ty	*the_command;
	string_ty	*id;

	assert(cp->cstate_data->state == cstate_state_being_integrated);
	pconf_data = change_pconf_get(cp, 1);
	assert(pconf_data);
	the_command = pconf_data->build_command;
	assert(the_command);
	if (!the_command)
	{
		change_fatal
		(
			cp,
		   "the build_command field must be defined in the \"%s\" file",
			THE_CONFIG_FILE
		);
	}

	/*
	 * %1 = project name
	 * %2 = change number
	 * %3 = identifying string, in the form "a.b.Dnnn"
	 *	where 'a' is the major version number,
	 *	'b' is the minor version number,
	 *	and 'nnn' is the build number.
	 */
	sub_var_set("1", "${project}");
	sub_var_set("2", "${change}");
	sub_var_set("3", "${version}");
	the_command = substitute(cp, the_command);

	id = change_integration_directory_get(cp, 0);
	change_env_set(cp);
	project_become(cp->pp);
	os_execute(the_command, OS_EXEC_FLAG_NO_INPUT, id);
	project_become_undo();
	str_free(the_command);
}


void
change_run_development_build_command(cp, up, partial)
	change_ty	*cp;
	user_ty		*up;
	wlist		*partial;
{
	pconf		pconf_data;
	string_ty	*the_command;
	string_ty	*dd;

	assert(cp->cstate_data->state == cstate_state_being_developed);
	pconf_data = change_pconf_get(cp, 1);
	assert(pconf_data);
	the_command = pconf_data->development_build_command;
	if (!the_command)
	{
		change_fatal
		(
			cp,
		   "the build_command field must be defined in the \"%s\" file",
			THE_CONFIG_FILE
		);
	}

	/*
	 * %1 = project name
	 * %2 = change number
	 * %3 = identifying string, in the form "a.b.Cnnn"
	 *	where 'a' is the major version number,
	 *	'b' is the minor version number,
	 *	and 'nnn' is the change number.
	 * %4 = the absolute path of the project baseline directory.
	 */
	sub_var_set("1", "${project}");
	sub_var_set("2", "${change}");
	sub_var_set("3", "${version}");
	sub_var_set("4", "${baseline}");
	the_command = substitute(cp, the_command);

	if (partial->wl_nwords)
	{
		string_ty	*s;
		string_ty	*s2;

		s = wl2str(partial, 0, partial->wl_nwords, (char *)0);
		s2 = str_format("%S %S", the_command, s);
		str_free(s);
		str_free(the_command);
		the_command = s2;
	}

	dd = change_development_directory_get(cp, 0);
	change_env_set(cp);
	user_become(up);
	os_execute(the_command, OS_EXEC_FLAG_NO_INPUT, dd);
	os_become_undo();
	str_free(the_command);
}

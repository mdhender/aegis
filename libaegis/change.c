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
 * MANIFEST: functions to manipulate change state data
 */

#include <ac/ctype.h>
#include <stdio.h>
#include <ac/stdlib.h>
#include <ac/string.h>
#include <ac/time.h>

#include <abbreviate.h>
#include <arglex2.h>
#include <change.h>
#include <change_file.h>
#include <commit.h>
#include <dir.h>
#include <env.h>
#include <error.h>
#include <gmatch.h>
#include <lock.h>
#include <mem.h>
#include <os.h>
#include <progname.h>
#include <project.h>
#include <project_file.h>
#include <project_hist.h>
#include <str_list.h>
#include <sub.h>
#include <symtab.h>
#include <trace.h>
#include <uname.h>
#include <undo.h>
#include <user.h>


change_ty *
change_alloc(pp, number)
	project_ty	*pp;
	long		number;
{
	change_ty	*cp;

	trace(("change_alloc(pp = %8.8lX, number = %ld)\n{\n"/*}*/, pp, number));
	assert(number >= 1 || number == MAGIC_ZERO);
	cp = (change_ty *)mem_alloc(sizeof(change_ty));
	cp->reference_count = 1;
	cp->pp = project_copy(pp);
	cp->number = number;

	cp->cstate_data = 0;
	cp->cstate_filename = 0;
	cp->cstate_is_a_new_file = 0;
	cp->fstate_data = 0;
	cp->fstate_stp = 0;
	cp->fstate_filename = 0;
	cp->fstate_is_a_new_file = 0;

	cp->architecture_name = 0;
	cp->bogus = 0;
	cp->development_directory_unresolved = 0;
	cp->development_directory_resolved = 0;
	cp->integration_directory_unresolved = 0;
	cp->integration_directory_resolved = 0;
	cp->lock_magic = 0;
	cp->logfile = 0;
	cp->pconf_data = 0;
	cp->pconf_path = 0;
	trace(("return %8.8lX;\n", cp));
	trace((/*{*/"}\n"));
	return cp;
}


void
change_free(cp)
	change_ty	*cp;
{
	trace(("change_free(cp = %8.8lX)\n{\n"/*}*/, cp));
	assert(cp->reference_count >= 1);
	cp->reference_count--;
	if (cp->reference_count <= 0)
	{
		assert(cp->pp);
		project_free(cp->pp);
		if (cp->cstate_filename)
			str_free(cp->cstate_filename);
		if (cp->fstate_filename)
			str_free(cp->fstate_filename);
		if (cp->cstate_data)
			cstate_type.free(cp->cstate_data);
		if (cp->fstate_data)
			fstate_type.free(cp->fstate_data);
		if (cp->fstate_stp)
			symtab_free(cp->fstate_stp);
		if (cp->development_directory_unresolved)
			str_free(cp->development_directory_unresolved);
		if (cp->development_directory_resolved)
			str_free(cp->development_directory_resolved);
		if (cp->integration_directory_unresolved)
			str_free(cp->integration_directory_unresolved);
		if (cp->integration_directory_resolved)
			str_free(cp->integration_directory_resolved);
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
	trace(("change_copy(cp = %8.8lX)\n{\n"/*}*/, cp));
	assert(cp->reference_count >= 1);
	cp->reference_count++;
	trace(("return %8.8lX;\n", cp));
	trace((/*{*/"}\n"));
	return cp;
}


static void improve _((cstate));

static void
improve(d)
	cstate		d;
{
	trace(("improve(d = %8.8lX)\n{\n"/*}*/, (long)d));
	if (!d->history)
		d->history = cstate_history_list_type.alloc();
	assert(!d->src);
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
	if (d->branch)
	{
		if (!(d->branch->mask & cstate_branch_umask_mask))
			d->branch->umask = DEFAULT_UMASK;
		d->branch->umask = (d->branch->umask & 5) | 022;
		if (d->branch->umask == 023)
			d->branch->umask = 022;
		/* 022, 026 and 027 are OK */
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

	if (cp->cstate_data && !cp->cstate_is_a_new_file)
	{
		cstate_type.free(cp->cstate_data);
		cp->cstate_data = 0;
	}
	if (cp->fstate_data && !cp->fstate_is_a_new_file)
	{
		fstate_type.free(cp->fstate_data);
		cp->fstate_data = 0;
	}
	if (cp->fstate_stp)
	{
		symtab_free(cp->fstate_stp);
		cp->fstate_stp = 0;
	}
	if (cp->pconf_path)
	{
		str_free(cp->pconf_path);
		cp->pconf_data = 0;
	}
}


static void cstate_to_fstate _((change_ty *));

static void
cstate_to_fstate(cp)
 	change_ty	*cp;
{
	long		j;

	trace(("cstate_to_fstate(cp = %8.8lX)\n{\n"/*}*/, cp));
	assert(cp->reference_count >= 1);
	assert(cp->cstate_data);
	assert(cp->cstate_data->src);
	assert(!cp->fstate_data);
	cp->fstate_data = fstate_type.alloc();
	cp->fstate_data->src = fstate_src_list_type.alloc();
	cp->fstate_is_a_new_file = 1;
	cp->fstate_stp = symtab_alloc(5);

	/*
	 * copy the file records from cstate to fstate
	 */
	for (j = 0; j < cp->cstate_data->src->length; ++j)
	{
		cstate_src	src1;
		fstate_src	src2;
		fstate_src	*addr_p;
		type_ty		*type_p;

		src1 = cp->cstate_data->src->list[j];
		if
		(
			!(src1->mask & cstate_src_action_mask)
		||
			!(src1->mask & cstate_src_usage_mask)
		||
			!src1->file_name
		||
			(
				cp->cstate_data->state == cstate_state_completed
			&&
				!src1->edit_number
			)
		)
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "File_Name", "%S", cp->cstate_filename);
			sub_var_set(scp, "FieLD_Name", "src");
			change_fatal
			(
				cp,
				scp,
			      i18n("$filename: corrupted \"$field_name\" field")
			);
			/* NOTREACHED */
			sub_context_delete(scp);
		}

		src2 = fstate_src_type.alloc();
		addr_p = fstate_src_list_type.list_parse(cp->fstate_data->src, &type_p);
		assert(type_p == &fstate_src_type);
		*addr_p = src2;

		/*
		 * copy all of the attributes across
		 */
		src2->action = src1->action;
		src2->mask |= fstate_src_action_mask;
		src2->usage = src1->usage;
		src2->mask |= fstate_src_usage_mask;
		src2->file_name = str_copy(src1->file_name);
		if (src1->edit_number)
		{
			src2->edit_number = str_copy(src1->edit_number);
			src2->edit_number_origin =
				str_copy(src1->edit_number);
		}
		if (src1->move)
			src2->move = str_copy(src1->move);

		/*
		 * index to track also
		 */
		symtab_assign(cp->fstate_stp, src2->file_name, src2);
	}

	/*
	 * now release the src field of the cstate file
	 */
	cstate_src_list_type.free(cp->cstate_data->src);
	cp->cstate_data->src = 0;
	trace((/*{*/"}\n"));
}


string_ty *
change_cstate_filename_get(cp)
	change_ty	*cp;
{
	assert(cp->reference_count >= 1);
	if (!cp->cstate_filename)
		cp->cstate_filename =
			project_change_path_get(cp->pp, cp->number);
	return cp->cstate_filename;
}


string_ty *
change_fstate_filename_get(cp)
	change_ty	*cp;
{
	assert(cp->reference_count >= 1);
	if (!cp->fstate_filename)
	{
		cp->fstate_filename =
			str_format("%S.fs", change_cstate_filename_get(cp));
	}
	return cp->fstate_filename;
}


cstate
change_cstate_get(cp)
	change_ty	*cp;
{
	string_ty	*fn;

	trace(("change_cstate_get(cp = %8.8lX)\n{\n"/*}*/, cp));
	assert(cp->reference_count >= 1);
	lock_sync(cp);
	if (!cp->cstate_data)
	{
		fn = change_cstate_filename_get(cp);
		change_become(cp);
		cp->cstate_data = cstate_read_file(fn->str_text);
		change_become_undo();
		if (!cp->cstate_data->brief_description)
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "File_Name", "%S", cp->cstate_filename);
			sub_var_set(scp, "FieLD_Name", "brief_description");
			change_fatal
			(
				cp,
				scp,
			      i18n("$filename: corrupted \"$field_name\" field")
			);
			/* NOTREACHED */
			sub_context_delete(scp);
		}
		if (!cp->cstate_data->description)
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "File_Name", "%S", cp->cstate_filename);
			sub_var_set(scp, "FieLD_Name", "description");
			change_fatal
			(
				cp,
				scp,
			      i18n("$filename: corrupted \"$field_name\" field")
			);
			/* NOTREACHED */
			sub_context_delete(scp);
		}
		if (!(cp->cstate_data->mask & cstate_state_mask))
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "File_Name", "%S", cp->cstate_filename);
			sub_var_set(scp, "FieLD_Name", "state");
			change_fatal
			(
				cp,
				scp,
			      i18n("$filename: corrupted \"$field_name\" field")
			);
			/* NOTREACHED */
			sub_context_delete(scp);
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
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "File_Name", "%S", cp->cstate_filename);
			sub_var_set(scp, "FieLD_Name", "development_directory");
			change_fatal
			(
				cp,
				scp,
			    i18n("$filename: contains no \"$field_name\" field")
			);
			/* NOTREACHED */
			sub_context_delete(scp);
		}
		if
		(
			cp->cstate_data->state == cstate_state_being_integrated
		&&
			!cp->cstate_data->integration_directory
		)
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "File_Name", "%S", cp->cstate_filename);
			sub_var_set(scp, "FieLD_Name", "integration_directory");
			change_fatal
			(
				cp,
				scp,
			    i18n("$filename: contains no \"$field_name\" field")
			);
			/* NOTREACHED */
			sub_context_delete(scp);
		}
		if
		(
			cp->cstate_data->state == cstate_state_completed
		&&
			!cp->cstate_data->delta_number
		)
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "File_Name", "%S", cp->cstate_filename);
			sub_var_set(scp, "FieLD_Name", "delta_number");
			change_fatal
			(
				cp,
				scp,
			    i18n("$filename: contains no \"$field_name\" field")
			);
			/* NOTREACHED */
			sub_context_delete(scp);
		}
		if (cp->cstate_data->src)
		{
			/*
			 * convert from old format to new fstate format
			 */
			assert(!cp->fstate_data);
			cstate_to_fstate(cp);
		}
		improve(cp->cstate_data);
	}
	trace(("return %8.8lX;\n", cp->cstate_data));
	trace((/*{*/"}\n"));
	return cp->cstate_data;
}


void
change_bind_new(cp)
	change_ty	*cp;
{
	trace(("change_bind_new(cp = %8.8lX)\n{\n"/*}*/, cp));
	assert(cp->reference_count >= 1);
	assert(!cp->cstate_data);
	cp->cstate_is_a_new_file = 1;
	cp->cstate_data = cstate_type.alloc();
	cp->fstate_is_a_new_file = 1;
	cp->fstate_data = fstate_type.alloc();
	improve(cp->cstate_data);
	trace((/*{*/"}\n"));
}


static int src_cmp _((const void *, const void *));

static int
src_cmp(s1p, s2p)
	const void	*s1p;
	const void	*s2p;
{
	fstate_src	s1;
	fstate_src	s2;

	s1 = *(fstate_src *)s1p;
	s2 = *(fstate_src *)s2p;
	return strcmp(s1->file_name->str_text, s2->file_name->str_text);
}


static int long_cmp _((const void *, const void *));

static int
long_cmp(s1p, s2p)
	const void	*s1p;
	const void	*s2p;
{
	long		n1;
	long		n2;

	n1 = *(long *)s1p;
	n2 = *(long *)s2p;
	if (n1 < n2)
		return -1;
	if (n1 > n2)
		return 1;
	return 0;
}


void
change_cstate_write(cp)
	change_ty	*cp;
{
	string_ty	*filename_new;
	string_ty	*filename_old;
	static int	count;
	string_ty	*fn;
	int		mode;

	trace(("change_cstate_write(cp = %8.8lX)\n{\n"/*}*/, cp));
	assert(cp->reference_count >= 1);
	assert(cp->pp);
	assert(cp->cstate_data);
	if (!cp->cstate_data->brief_description)
		cp->cstate_data->brief_description = str_from_c("");
	if (!cp->cstate_data->description)
		cp->cstate_data->description =
			str_copy(cp->cstate_data->brief_description);

	/*
	 * force various project related files to be read in,
	 * if they are not already
	 */
	mode = 0644 & ~change_umask(cp);

	/*
	 * write out the fstate file
	 */
	assert(!cp->cstate_data->src);
	if (cp->fstate_data)
	{
		/*
		 * sort the files by name
		 */
		if (!cp->fstate_data->src)
			cp->fstate_data->src = fstate_src_type.alloc();
		if (cp->fstate_data->src->length >= 2)
		{
			assert(cp->fstate_data->src->list);
			qsort
			(
				cp->fstate_data->src->list,
				cp->fstate_data->src->length,
				sizeof(*cp->fstate_data->src->list),
				src_cmp
			);
		}

		fn = change_fstate_filename_get(cp);
		filename_new = str_format("%S,%d", fn, ++count);
		filename_old = str_format("%S,%d", fn, ++count);
		change_become(cp);
		if (cp->fstate_is_a_new_file)
		{
			string_ty	*s1;
			string_ty	*s2;

			s1 = project_Home_path_get(cp->pp);
			s2 = os_below_dir(s1, fn);
			os_mkdir_between(s1, s2, 02755);
			str_free(s2);
			undo_unlink_errok(filename_new);
			fstate_write_file(filename_new->str_text, cp->fstate_data);
			commit_rename(filename_new, fn);
			cp->fstate_is_a_new_file = 0;
		}
		else
		{
			undo_unlink_errok(filename_new);
			fstate_write_file(filename_new->str_text, cp->fstate_data);
			commit_rename(fn, filename_old);
			commit_rename(filename_new, fn);
			commit_unlink_errok(filename_old);
		}
	
		/*
		 * Change the file mode as appropriate.
		 * (Only need to do this for new files, but be paranoid.)
		 */
		os_chmod(filename_new, mode);
		change_become_undo();
		str_free(filename_new);
		str_free(filename_old);
	}

	/*
	 * force the change list to be sorted
	 */
	assert(cp->cstate_data);
	if (cp->cstate_data->branch && cp->cstate_data->branch->change)
	{
		cstate_branch_change_list lp;

		lp = cp->cstate_data->branch->change;
		qsort(lp->list, lp->length, sizeof(lp->list[0]), long_cmp);
	}

	/*
	 * write out the cstate file
	 */
	fn = change_cstate_filename_get(cp);
	assert(!cp->cstate_data->src);
	filename_new = str_format("%S,%d", fn, ++count);
	filename_old = str_format("%S,%d", fn, ++count);
	change_become(cp);
	if (cp->cstate_is_a_new_file)
	{
		string_ty	*s1;
		string_ty	*s2;

		s1 = project_Home_path_get(cp->pp);
		s2 = os_below_dir(s1, fn);
		os_mkdir_between(s1, s2, 02755);
		str_free(s2);
		undo_unlink_errok(filename_new);
		cstate_write_file(filename_new->str_text, cp->cstate_data);
		commit_rename(filename_new, fn);
		cp->cstate_is_a_new_file = 0;
	}
	else
	{
		undo_unlink_errok(filename_new);
		cstate_write_file(filename_new->str_text, cp->cstate_data);
		commit_rename(fn, filename_old);
		commit_rename(filename_new, fn);
		commit_unlink_errok(filename_old);
	}

	/*
	 * Change the file mode as appropriate.
	 * (Only need to do this for new files, but be paranoid.)
	 */
	os_chmod(filename_new, mode);
	change_become_undo();
	str_free(filename_new);
	str_free(filename_old);
	trace((/*{*/"}\n"));
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

	trace(("change_history_new(cp = %8.8lX)\n{\n"/*}*/, cp));
	assert(cp->reference_count >= 1);
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
	trace(("return %8.8lX;\n", history_data));
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

	trace(("change_developer_name(cp = %8.8lX)\n{\n"/*}*/, cp));
	assert(cp->reference_count >= 1);
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

	trace(("change_reviewer_name(cp = %8.8lX)\n{\n"/*}*/, cp));
	assert(cp->reference_count >= 1);
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

	trace(("change_integrator_name(cp = %8.8lX)\n{\n"/*}*/, cp));
	assert(cp->reference_count >= 1);
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
	change_ty	*pcp;
	cstate		pcsp;
	cstate_branch_change_list lp;
	size_t		j;

	/*
	 * verify the change number given on the command line
	 */
	trace(("change_bind_existing(cp = %8.8lX)\n{\n"/*}*/, cp));
	assert(cp->reference_count >= 1);
	if (cp->number == TRUNK_CHANGE_NUMBER)
	{
		trace((/*{*/"}\n"));
		return;
	}
	pcp = project_change_get(cp->pp);
	pcsp = change_cstate_get(pcp);
	if (!pcsp->branch)
	{
		assert(0);
		pcsp->branch = cstate_branch_type.alloc();
	}
	if (!pcsp->branch->change)
		pcsp->branch->change = cstate_branch_change_list_type.alloc();
	lp = pcsp->branch->change;
	for (j = 0; j < lp->length; ++j)
	{
		if (lp->list[j] == cp->number)
			break;
	}
	if (j >= lp->length)
		change_fatal(cp, 0, i18n("unknown change"));
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
	trace(("change_development_directory_set(cp = %8.8lX, s = \"%s\")\n{\n"
		/*}*/, cp, s->str_text));
	assert(cp->reference_count >= 1);
	if (cp->development_directory_unresolved)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "Name", "%s", arglex_token_name(arglex_token_directory));
		fatal_intl(scp, i18n("duplicate $name option"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}
	assert(s->str_text[0] == '/');
	cp->development_directory_unresolved = str_copy(s);
	change_become(cp);
	cp->development_directory_resolved = os_pathname(s, 1);
	change_become_undo();
	cstate_data = change_cstate_get(cp);
	if (!cstate_data->development_directory)
	{
		string_ty	*dir;

		dir = os_below_dir(project_Home_path_get(cp->pp), s);
		if (dir)
		{
			if (!dir->str_length)
			{
				str_free(dir);
				dir = str_from_c(".");
			}
			cstate_data->development_directory = dir;
		}
		else
			cstate_data->development_directory = str_copy(s);
	}
	trace((/*{*/"}\n"));
}


string_ty *
change_development_directory_get(cp, resolve)
	change_ty	*cp;
	int		resolve;
{
	string_ty	*result;

	/*
	 * To cope with automounters, directories are stored as given,
	 * or are derived from the home directory in the passwd file.
	 * Within aegis, pathnames have their symbolic links resolved,
	 * and any comparison of paths is done on this "system idea"
	 * of the pathname.
	 */
	trace(("change_development_directory_get(cp = %8.8lX)\n{\n"/*}*/, cp));
	assert(cp->reference_count >= 1);
	if (!cp->development_directory_unresolved)
	{
		cstate		cstate_data;
		string_ty	*dir;

		cstate_data = change_cstate_get(cp);
		dir = cstate_data->development_directory;
		if (!dir)
			change_fatal(cp, 0, i18n("no dev dir"));
		if (dir->str_text[0] == '/')
			cp->development_directory_unresolved = str_copy(dir);
		else
		{
			cp->development_directory_unresolved =
				os_path_cat(project_Home_path_get(cp->pp), dir);
		}
	}
	if (!resolve)
		result = cp->development_directory_unresolved;
	else
	{
		if (!cp->development_directory_resolved)
		{
			change_become(cp);
			cp->development_directory_resolved =
				os_pathname
				(
					cp->development_directory_unresolved,
					1
				);
			change_become_undo();
		}
		result = cp->development_directory_resolved;
	}
	trace(("result = \"%s\"\n", result->str_text));
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
	trace(("change_integration_directory_set(cp = %8.8lX, s = \"%s\")\n{\n"
		/*}*/, cp, s->str_text));
	assert(cp->reference_count >= 1);
	if (cp->integration_directory_resolved)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "Name", "%s", arglex_token_name(arglex_token_directory));
		fatal_intl(scp, i18n("duplicate $name option"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}
	cp->integration_directory_unresolved = str_copy(s);
	change_become(cp);
	cp->integration_directory_resolved = os_pathname(s, 1);
	change_become_undo();
	cstate_data = change_cstate_get(cp);
	if (!cstate_data->integration_directory)
	{
		string_ty	*dir;

		dir = os_below_dir(project_Home_path_get(cp->pp), s);
		if (dir)
		{
			if (!dir->str_length)
			{
				assert(0);
				str_free(dir);
				dir = str_from_c(".");
			}
			cstate_data->integration_directory = dir;
		}
		else
			cstate_data->integration_directory = str_copy(s);
	}
	trace((/*{*/"}\n"));
}


string_ty *
change_integration_directory_get(cp, resolve)
	change_ty	*cp;
	int		resolve;
{
	string_ty	*result;

	/*
	 * To cope with automounters, directories are stored as given,
	 * or are derived from the home directory in the passwd file.
	 * Within aegis, pathnames have their symbolic links resolved,
	 * and any comparison of paths is done on this "system idea"
	 * of the pathname.
	 */
	trace(("change_integration_directory_get(cp = %8.8lX)\n{\n"/*}*/, cp));
	assert(cp->reference_count >= 1);
	if (!cp->integration_directory_unresolved)
	{
		cstate		cstate_data;
		string_ty	*dir;

		cstate_data = change_cstate_get(cp);
		dir = cstate_data->integration_directory;
		if (!dir)
			change_fatal(cp, 0, i18n("no int dir"));
		if (dir->str_text[0] == '/')
			cp->integration_directory_unresolved = str_copy(dir);
		else
		{
			cp->integration_directory_unresolved =
				os_path_cat(project_Home_path_get(cp->pp), dir);
		}
	}
	if (!resolve)
		result = cp->integration_directory_unresolved;
	else
	{
		if (!cp->integration_directory_resolved)
		{
			change_become(cp);
			cp->integration_directory_resolved =
				os_pathname
				(
					cp->integration_directory_unresolved,
					1
				);
			change_become_undo();
		}
		result = cp->integration_directory_resolved;
	}
	trace(("result = \"%s\"\n", result));
	trace((/*{*/"}\n"));
	return result;
}


string_ty *
change_logfile_basename()
{
	static string_ty *s;

	if (!s)
		s = str_format("%.10s.log", progname_get());
	return s;
}


string_ty *
change_logfile_get(cp)
	change_ty	*cp;
{
	string_ty	*s1;
	cstate		cstate_data;

	trace(("change_logfile_get(cp = %8.8lX)\n{\n"/*}*/, cp));
	assert(cp->reference_count >= 1);
	if (!cp->logfile)
	{
		cstate_data = change_cstate_get(cp);
		switch (cstate_data->state)
		{
		default:
			change_fatal(cp, 0, i18n("no log file"));

		case cstate_state_being_integrated:
			s1 = change_integration_directory_get(cp, 0);
			break;

		case cstate_state_being_developed:
			s1 = change_development_directory_get(cp, 0);
			break;
		}

		cp->logfile =
			os_path_cat(s1, change_logfile_basename());
	}
	trace(("return \"%s\";\n", cp->logfile->str_text));
	trace((/*{*/"}\n"));
	return cp->logfile;
}


static void waiting_callback _((void*));

static void
waiting_callback(p)
	void		*p;
{
	change_ty	*cp;

	cp = p;
	if (user_lock_wait(0))
		change_verbose(cp, 0, i18n("waiting for lock"));
	else
		change_verbose(cp, 0, i18n("lock not available"));
}


void
change_cstate_lock_prepare(cp)
	change_ty	*cp;
{
	trace(("change_cstate_lock_prepare(cp = %8.8lX)\n{\n"/*}*/, cp));
	assert(cp->reference_count >= 1);
	lock_prepare_cstate
	(
		project_name_get(cp->pp),
		cp->number,
		waiting_callback,
		(void *)cp
	);
	trace((/*{*/"}\n"));
}


void
change_error(cp, scp, s)
	change_ty	*cp;
	sub_context_ty	*scp;
	char		*s;
{
	string_ty	*msg;
	int		need_to_delete;

	if (!scp)
	{
		scp = sub_context_new();
		need_to_delete = 1;
	}
	else
		need_to_delete = 0;

	/*
	 * asemble the message
	 */
	subst_intl_change(scp, cp);
	msg = subst_intl(scp, s);

	/*
	 * get ready to pass the message to the project error function
	 */
	/* re-use substitution context */
	sub_var_set(scp, "Message", "%S", msg);
	str_free(msg);

	/*
	 * the form of the project error message depends on what kind of
	 * change this is
	 */
	if (cp->bogus)
		project_error(cp->pp, scp, i18n("$message"));
	else if (cp->number == TRUNK_CHANGE_NUMBER)
		project_error(cp->pp, scp, i18n("trunk: $message"));
	else
	{
		subst_intl_change(scp, cp);
		project_error(cp->pp, scp, i18n("change $change: $message"));
	}

	if (need_to_delete)
		sub_context_delete(scp);
}


void
change_fatal(cp, scp, s)
	change_ty	*cp;
	sub_context_ty	*scp;
	char		*s;
{
	string_ty	*msg;
	int		need_to_delete;

	if (!scp)
	{
		scp = sub_context_new();
		need_to_delete = 1;
	}
	else
		need_to_delete = 0;

	/*
	 * asemble the message
	 */
	subst_intl_change(scp, cp);
	msg = subst_intl(scp, s);

	/*
	 * get ready to pass the message to the project error function
	 */
	/* re-use substitution context */
	sub_var_set(scp, "Message", "%S", msg);
	str_free(msg);

	/*
	 * the form of the project error message depends on what kind of
	 * change this is
	 */
	if (cp->bogus)
		project_fatal(cp->pp, scp, i18n("$message"));
	else if (cp->number == TRUNK_CHANGE_NUMBER)
		project_fatal(cp->pp, scp, i18n("trunk: $message"));
	else
	{
		subst_intl_change(scp, cp);
		project_fatal(cp->pp, scp, i18n("change $change: $message"));
	}
	/* NOTREACHED */

	if (need_to_delete)
		sub_context_delete(scp);
}


void
change_verbose(cp, scp, s)
	change_ty	*cp;
	sub_context_ty	*scp;
	char		*s;
{
	string_ty	*msg;
	int		need_to_delete;

	if (!scp)
	{
		scp = sub_context_new();
		need_to_delete = 1;
	}
	else
		need_to_delete = 0;

	/*
	 * asemble the message
	 */
	subst_intl_change(scp, cp);
	msg = subst_intl(scp, s);

	/*
	 * get ready to pass the message to the project error function
	 */
	/* re-use substitution context */
	sub_var_set(scp, "Message", "%S", msg);
	str_free(msg);

	/*
	 * the form of the project error message depends on what kind of
	 * change this is
	 */
	if (cp->bogus)
		project_verbose(cp->pp, scp, i18n("$message"));
	else if (cp->number == TRUNK_CHANGE_NUMBER)
		project_verbose(cp->pp, scp, i18n("trunk: $message"));
	else
	{
		subst_intl_change(scp, cp);
		project_verbose(cp->pp, scp, i18n("change $change: $message"));
	}

	if (need_to_delete)
		sub_context_delete(scp);
}


string_ty *
change_pconf_path_get(cp)
	change_ty	*cp;
{
	static string_ty *file_name;

	trace(("change_pconf_path_get(cp = %8.8lX)\n{\n"/*}*/, cp));
	assert(cp->reference_count >= 1);
	if (!cp->pconf_path)
	{
		if (!file_name)
			file_name = str_from_c(THE_CONFIG_FILE);
		cp->pconf_path = change_file_source(cp, file_name);
		if (!cp->pconf_path)
		{
			cstate		cstate_data;
			string_ty	*dir;

			/*
			 * The file does not yet exist in either the
			 * change or the project.  Fake it.
			 */
			cstate_data = change_cstate_get(cp);
			switch (cstate_data->state)
			{
			case cstate_state_being_developed:
			case cstate_state_being_reviewed:
			case cstate_state_awaiting_integration:
				dir = change_development_directory_get(cp, 0);
				if (!dir)
				{
					/*
					 * Development directory may not
					 * be set yet, particularly if
					 * called by aeib.  Use the
					 * baseline instead.
					 */
					dir = project_baseline_path_get(cp->pp, 0);
				}
				break;

			case cstate_state_being_integrated:
				dir = change_integration_directory_get(cp, 0);
				break;

			default:
				dir = project_baseline_path_get(cp->pp, 0);
				break;
			}
			trace(("dir = \"%s\"\n", (dir ? dir->str_text : "")));
			cp->pconf_path = os_path_cat(dir, file_name);
			trace(("cp->pconf_path = \"%s\"\n", cp->pconf_path->str_text));
		}
	}
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


static void pconf_improve _((change_ty *, pconf, string_ty *));

static void
pconf_improve(cp, d, filename)
	change_ty	*cp;
	pconf		d;
	string_ty	*filename;
{
	sub_context_ty	*scp;

	if (!d->build_command)
	{
		scp = sub_context_new();
		sub_var_set(scp, "File_Name", "%S", filename);
		sub_var_set(scp, "FieLD_Name", "build_command");
		change_fatal
		(
			cp,
			scp,
			i18n("$filename: contains no \"$field_name\" field")
		);
		/* NOTREACHED */
		sub_context_delete(scp);
	}
	if (!d->development_build_command)
		d->development_build_command = str_copy(d->build_command);
	if (!d->history_create_command)
	{
		scp = sub_context_new();
		sub_var_set(scp, "File_Name", "%S", filename);
		sub_var_set(scp, "FieLD_Name", "history_create_command");
		change_fatal
		(
			cp,
			scp,
			i18n("$filename: contains no \"$field_name\" field")
		);
		/* NOTREACHED */
		sub_context_delete(scp);
	}
	if (!d->history_get_command)
	{
		scp = sub_context_new();
		sub_var_set(scp, "File_Name", "%S", filename);
		sub_var_set(scp, "FieLD_Name", "history_get_command");
		change_fatal
		(
			cp,
			scp,
			i18n("$filename: contains no \"$field_name\" field")
		);
		/* NOTREACHED */
		sub_context_delete(scp);
	}
	if (!d->history_put_command)
	{
		scp = sub_context_new();
		sub_var_set(scp, "File_Name", "%S", filename);
		sub_var_set(scp, "FieLD_Name", "history_put_command");
		change_fatal
		(
			cp,
			scp,
			i18n("$filename: contains no \"$field_name\" field")
		);
		/* NOTREACHED */
		sub_context_delete(scp);
	}
	if (!d->history_query_command)
	{
		scp = sub_context_new();
		sub_var_set(scp, "File_Name", "%S", filename);
		sub_var_set(scp, "FieLD_Name", "history_query_command");
		change_fatal
		(
			cp,
			scp,
			i18n("$filename: contains no \"$field_name\" field")
		);
		/* NOTREACHED */
		sub_context_delete(scp);
	}
	if (!d->diff_command)
	{
		scp = sub_context_new();
		sub_var_set(scp, "File_Name", "%S", filename);
		sub_var_set(scp, "FieLD_Name", "diff_command");
		change_fatal
		(
			cp,
			scp,
			i18n("$filename: contains no \"$field_name\" field")
		);
		/* NOTREACHED */
		sub_context_delete(scp);
	}
	if (!d->diff3_command && !d->merge_command)
	{
		scp = sub_context_new();
		sub_var_set(scp, "File_Name", "%S", filename);
		sub_var_set(scp, "FieLD_Name", "merge_command");
		change_fatal
		(
			cp,
			scp,
			i18n("$filename: contains no \"$field_name\" field")
		);
		/* NOTREACHED */
		sub_context_delete(scp);
	}
	if (!(d->mask & pconf_shell_safe_filenames_mask))
	{
		d->shell_safe_filenames = 1;
		d->mask |= pconf_shell_safe_filenames_mask;
	}
}


pconf
change_pconf_get(cp, required)
	change_ty	*cp;
	int		required;
{
	static string_ty *star_comma_d;
	size_t		j;

	trace(("change_pconf_get(cp = %8.8lX, required = %d)\n{\n"/*}*/,
		(long)cp, required));
	assert(cp->reference_count >= 1);
	lock_sync(cp);
	if (!cp->pconf_data)
	{
		string_ty	*filename;

		filename = change_pconf_path_get(cp);
		assert(filename);
		change_become(cp);
		if (!os_exists(filename))
		{
			if (required)
			{
				sub_context_ty	*scp;

				scp = sub_context_new();
				sub_var_set(scp, "File_Name", "%s", THE_CONFIG_FILE);
				change_fatal
				(
					cp,
					scp,
				    i18n("you must create a \"$filename\" file")
				);
				/* NOTREACHED */
				sub_context_delete(scp);
			}
			cp->pconf_data = pconf_type.alloc();
			cp->pconf_data->shell_safe_filenames = 1;
		}
		else
		{
			cp->pconf_data = pconf_read_file(filename->str_text);
			pconf_improve(cp, cp->pconf_data, filename);
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
				sub_context_ty	*scp;

				scp = sub_context_new();
				sub_var_set(scp, "File_Name", "%S", filename);
				change_fatal
				(
					cp,
					scp,
				  i18n("$filename: arch needs name and pattern")
				);
				/* NOTREACHED */
				sub_context_delete(scp);
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
			assert(type_p == &string_type);
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
			assert(type_p == &string_type);
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

		/*
		 * set the development directory template default
		 */
		if (!cp->pconf_data->development_directory_template)
		{
			cp->pconf_data->development_directory_template =
				str_from_c
				(
"$ddd/${left $project ${expr ${namemax $ddd} - ${length .$magic${zpad $c 3}}}}\
.$magic${zpad $c 3}"
				);
		}

		/*
		 * Set the test filename template
		 */
		if (!cp->pconf_data->new_test_filename)
		{
			cp->pconf_data->new_test_filename =
				str_from_c
				(
		  "test/${zpad $hundred 2}/t${zpad $number 4}${left $type 1}.sh"
				);
		}
	}
	trace(("return %8.8lX;\n", cp->pconf_data));
	trace((/*{*/"}\n"));
	return cp->pconf_data;
}


static void change_env_set _((change_ty *, int));

static void
change_env_set(cp, with_arch)
	change_ty	*cp;
	int		with_arch;
{
	string_ty	*s;

	/*
	 * set the AEGIS_PROJECT environment cariable
	 */
	env_set("AEGIS_PROJECT", cp->pp->name->str_text);

	/*
	 * set the AEGIS_CHANGE environment cariable
	 */
	if (!cp->bogus)
	{
		char		buffer[20];

		sprintf(buffer, "%ld", magic_zero_decode(cp->number));
		env_set("AEGIS_CHANGE", buffer);
	}
	else
		env_unset("AEGIS_CHANGE");

	/*
	 * set the AEGIS_ARCH environment variable
	 */
	s = change_architecture_name(cp, with_arch);
	if (s)
		env_set("AEGIS_ARCH", s->str_text);
	else
		env_unset("AEGIS_ARCH");
}


void
change_run_change_file_command(cp, wlp, up)
	change_ty	*cp;
	string_list_ty		*wlp;
	user_ty		*up;
{
	sub_context_ty	*scp;
	pconf		pconf_data;
	string_ty	*the_command;
	string_ty	*the_files;
	string_ty	*dd;

	trace(("change_run_change_file_command(cp = %8.8lX)\n{\n"/*}*/, cp));
	assert(cp->reference_count >= 1);
	pconf_data = change_pconf_get(cp, 0);
	if (!pconf_data->change_file_command)
		goto ret;

	scp = sub_context_new();
	the_files = wl2str(wlp, 0, 32767, (char *)0);
	sub_var_set(scp, "File_List", "%S", the_files);
	sub_var_optional(scp, "File_List");
	str_free(the_files);
	sub_var_set(scp, "1", "${project}");
	sub_var_set(scp, "2", "${change}");
	sub_var_set(scp, "3", "${version}");
	sub_var_set(scp, "4", "${baseline}");
	sub_var_set(scp, "5", "${file_list}");
	the_command = pconf_data->change_file_command;
	the_command = substitute(scp, cp, the_command);
	sub_context_delete(scp);

	dd = change_development_directory_get(cp, 0);
	change_env_set(cp, 0);
	user_become(up);
	os_execute(the_command, OS_EXEC_FLAG_NO_INPUT | OS_EXEC_FLAG_ERROK, dd);
	user_become_undo();
	str_free(the_command);
	ret:
	trace((/*{*/"}\n"));
}


int
change_run_project_file_command_needed(cp)
	change_ty	*cp;
{
	cstate		cstate_data;

	assert(cp->reference_count >= 1);
	cstate_data = change_cstate_get(cp);
	return
		(
			cstate_data->project_file_command_sync
		!=
			project_last_change_integrated(cp->pp)
		);
}


void
change_run_project_file_command(cp, up)
	change_ty	*cp;
	user_ty		*up;
{
	sub_context_ty	*scp;
	cstate		cstate_data;
	pconf		pconf_data;
	string_ty	*the_command;
	string_ty	*dd;

	/*
	 * make sure are sync'ed with project
	 */
	trace(("change_run_project_file_command(cp = %8.8lX)\n{\n"/*}*/, cp));
	assert(cp->reference_count >= 1);
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
	scp = sub_context_new();
	sub_var_set(scp, "1", "${project}");
	sub_var_set(scp, "2", "${change}");
	sub_var_set(scp, "3", "${version}");
	sub_var_set(scp, "4", "${baseline}");
	the_command = pconf_data->project_file_command;
	the_command = substitute(scp,cp, the_command);
	sub_context_delete(scp);

	/*
	 * execute the command
	 */
	dd = change_development_directory_get(cp, 0);
	change_env_set(cp, 0);
	user_become(up);
	os_execute(the_command, OS_EXEC_FLAG_NO_INPUT | OS_EXEC_FLAG_ERROK, dd);
	user_become_undo();
	str_free(the_command);

	/*
	 * here for all exits
	 */
	done:
	trace((/*{*/"}\n"));
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
	assert(cp->reference_count >= 1);
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
				sub_context_ty	*scp;

				scp = sub_context_new();
				sub_var_set(scp,"File_Name", "%S", s);
				change_fatal(cp, scp, i18n("bad pattern $filename"));
				/* NOTREACHED */
				sub_context_delete(scp);
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
		sub_context_ty	*scp;
		string_ty	*s;

		scp = sub_context_new();
		sub_var_set(scp, "File_Name", "%S", name);
		sub_var_optional(scp, "File_Name");
		s = substitute(scp, cp, result);
		sub_context_delete(scp);
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
	trace(("change_become(cp = %8.8lX)\n{\n"/*}*/, cp));
	assert(cp->reference_count >= 1);
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
	assert(cp->reference_count >= 1);
	return project_umask_get(cp->pp);
}


void
change_run_forced_develop_begin_notify_command(cp, up)
	change_ty	*cp;
	user_ty		*up;
{
	string_ty	*the_command;
	string_ty	*dd;

	/*
	 * make sure there is one
	 */
	trace(("change_run_forced_develop_begin_notify_command(cp = %8.8lX, \
up = %8.8lX)\n{\n"/*}*/, (long)cp, (long)up));
	assert(cp->reference_count >= 1);
	the_command = project_forced_develop_begin_notify_command_get(cp->pp);
	if (!the_command)
		goto done;

	/*
	 * notify the change is ready for review
	 *	(it could be mail, or an internal bulletin board, etc)
	 * it happens after the data is written and the locks are released,
	 * so we don't much care if the command fails!
	 *
	 * All of the substitutions described in aesub(5) are available.
	 */
	the_command = substitute(0, cp, the_command);

	/*
	 * execute the command
	 */
	dd = change_development_directory_get(cp, 0);
	change_env_set(cp, 0);
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
	sub_context_ty	*scp;
	string_ty	*the_command;
	string_ty	*dd;

	/*
	 * make sure there is one
	 */
	trace(("change_run_develop_end_notify_command(cp = %8.8lX)\n{\n"/*}*/,
		cp));
	assert(cp->reference_count >= 1);
	the_command = project_develop_end_notify_command_get(cp->pp);
	if (!the_command)
		goto done;

	/*
	 * notify the change is ready for review
	 *	(it could be mail, or an internal bulletin board, etc)
	 * it happens after the data is written and the locks are released,
	 * so we don't much care if the command fails!
	 *
	 * All of the substitutions described in aesub(5) are available.
	 */
	scp = sub_context_new();
	sub_var_set(scp, "1", "${project}");
	sub_var_set(scp, "2", "${change}");
	sub_var_set(scp, "3", "${developer}");
	the_command = substitute(scp, cp, the_command);
	sub_context_delete(scp);

	/*
	 * execute the command
	 */
	dd = change_development_directory_get(cp, 0);
	change_env_set(cp, 0);
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
	sub_context_ty	*scp;
	string_ty	*the_command;
	string_ty	*dd;

	/*
	 * make sure there is one
	 */
	trace(("change_run_develop_end_undo_notify_command(cp = %8.8lX)\n{\n"
		/*}*/, cp));
	assert(cp->reference_count >= 1);
	the_command = project_develop_end_undo_notify_command_get(cp->pp);
	if (!the_command)
		goto done;

	/*
	 * notify the change is ready for review
	 *	(it could be mail, or an internal bulletin board, etc)
	 * it happens after the data is written and the locks are released,
	 * so we don't much care if the command fails!
	 *
	 * All of the substitutions described in aesub(5) are available.
	 */
	scp = sub_context_new();
	sub_var_set(scp, "1", "${project}");
	sub_var_set(scp, "2", "${change}");
	sub_var_set(scp, "3", "${developer}");
	the_command = substitute(scp, cp, the_command);
	sub_context_delete(scp);

	/*
	 * execute the command
	 */
	dd = change_development_directory_get(cp, 0);
	change_env_set(cp, 0);
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
	sub_context_ty	*scp;
	string_ty	*the_command;
	string_ty	*dd;

	/*
	 * make sure there is one
	 */
	trace(("change_run_integrate_fail_notify_command(cp = %8.8lX)\n{\n"/*}*/,
		cp));
	assert(cp->reference_count >= 1);
	the_command = project_integrate_fail_notify_command_get(cp->pp);
	if (!the_command)
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
	scp = sub_context_new();
	sub_var_set(scp, "1", "${project}");
	sub_var_set(scp, "2", "${change}");
	sub_var_set(scp, "3", "${developer}");
	sub_var_set(scp, "4", "${reviewer}");
	sub_var_set(scp, "5", "${integrator}");
	the_command = substitute(scp, cp, the_command);
	sub_context_delete(scp);

	/*
	 * execute the command
	 */
	dd = change_development_directory_get(cp, 0);
	change_env_set(cp, 0);
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
	sub_context_ty	*scp;
	string_ty	*the_command;
	string_ty	*bl;

	/*
	 * make sure there is one
	 */
	trace(("change_run_integrate_pass_notify_command(cp = %8.8lX)\n{\n"/*}*/,
		cp));
	assert(cp->reference_count >= 1);
	the_command = project_integrate_pass_notify_command_get(cp->pp);
	if (!the_command)
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
	scp = sub_context_new();
	sub_var_set(scp, "1", "${project}");
	sub_var_set(scp, "2", "${change}");
	sub_var_set(scp, "3", "${developer}");
	sub_var_set(scp, "4", "${reviewer}");
	sub_var_set(scp, "5", "${integrator}");
	the_command = substitute(scp, cp, the_command);
	sub_context_delete(scp);

	/*
	 * execute the command
	 */
	bl = project_baseline_path_get(cp->pp, 0);
	change_env_set(cp, 0);
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
	sub_context_ty	*scp;
	string_ty	*the_command;
	string_ty	*dd;

	/*
	 * make sure there is one
	 */
	trace(("change_run_review_pass_notify_command(cp = %8.8lX)\n{\n"/*}*/,
		cp));
	assert(cp->reference_count >= 1);
	the_command = project_review_pass_notify_command_get(cp->pp);
	if (!the_command)
		goto done;

	/*
	 * notify the review has passed
	 *	(it could be mail, or an internal bulletin board, etc)
	 * it happens after the data is written and the locks are released,
	 * so we don't much care if the command fails!
	 *
	 * All of the substitutions described in aesub(5) are available.
	 */
	scp = sub_context_new();
	sub_var_set(scp, "1", "${project}");
	sub_var_set(scp, "2", "${change}");
	sub_var_set(scp, "3", "${developer}");
	sub_var_set(scp, "4", "${reviewer}");
	the_command = substitute(scp, cp, the_command);
	sub_context_delete(scp);

	/*
	 * execute the command
	 */
	dd = change_development_directory_get(cp, 0);
	change_env_set(cp, 0);
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
	sub_context_ty	*scp;
	string_ty	*the_command;
	string_ty	*dd;
	string_ty	*notify;

	/*
	 * make sure there is one
	 */
	trace(("change_run_review_pass_undo_notify_command(cp = %8.8lX)\n{\n"
		/*}*/, cp));
	assert(cp->reference_count >= 1);
	notify = project_review_pass_undo_notify_command_get(cp->pp);
	if (!notify)
		notify = project_develop_end_undo_notify_command_get(cp->pp);
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
	scp = sub_context_new();
	sub_var_set(scp, "1", "${project}");
	sub_var_set(scp, "2", "${change}");
	sub_var_set(scp, "3", "${developer}");
	sub_var_set(scp, "4", "${reviewer}");
	the_command = substitute(scp, cp, notify);
	sub_context_delete(scp);

	/*
	 * execute the command
	 */
	dd = change_development_directory_get(cp, 0);
	change_env_set(cp, 0);
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
	sub_context_ty	*scp;
	string_ty	*the_command;
	string_ty	*dd;

	/*
	 * make sure there is one
	 */
	trace(("change_run_review_fail_notify_command(cp = %8.8lX)\n{\n"/*}*/,
		cp));
	assert(cp->reference_count >= 1);
	the_command = project_review_fail_notify_command_get(cp->pp);
	if (!the_command)
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
	scp = sub_context_new();
	sub_var_set(scp, "1", "${project}");
	sub_var_set(scp, "2", "${change}");
	sub_var_set(scp, "3", "${developer}");
	sub_var_set(scp, "4", "${reviewer}");
	the_command = substitute(scp, cp, the_command);
	sub_context_delete(scp);

	/*
	 * execute the command
	 */
	dd = change_development_directory_get(cp, 0);
	change_env_set(cp, 0);
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
	sub_context_ty	*scp;
	string_ty	*dir;
	cstate		cstate_data;
	string_ty	*the_command;
	pconf		pconf_data;

	/*
	 * If the edit numbers differ, extract the
	 * appropriate edit from the baseline (use the
	 * history-get-command) into a file in /tmp
	 *
	 * Current directory set to the base of the history tree.
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
	trace(("change_run_history_get_command(cp = %8.8lX)\n{\n"/*}*/, cp));
	assert(cp->reference_count >= 1);
	scp = sub_context_new();
	sub_var_set
	(
		scp,
		"History",
		"%S/%S",
		project_history_path_get(cp->pp),
		file_name
	);
	sub_var_set(scp, "Edit", "%S", edit_number);
	sub_var_set(scp, "Output", "%S", output_file);
	sub_var_set(scp, "1", "${history}");
	sub_var_set(scp, "2", "${edit}");
	sub_var_set(scp, "3", "${output}");
	pconf_data = change_pconf_get(cp, 1);

	the_command = pconf_data->history_get_command;
	if (!the_command)
	{
		sub_context_ty	*scp2;

		scp2 = sub_context_new();
		sub_var_set(scp2, "File_Name", "%s", THE_CONFIG_FILE);
		sub_var_set(scp2, "FieLD_Name", "history_get_command");
		change_fatal
		(
			cp,
			scp2,
			i18n("$filename: contains no \"$field_name\" field")
		);
		sub_context_delete(scp2);
	}
	the_command = substitute(scp, cp, the_command);
	sub_context_delete(scp);

	/*
	 * run the command as the current user
	 * (always output is to /tmp)
	 */
	cstate_data = change_cstate_get(cp);
	dir = project_history_path_get(cp->pp);
	change_env_set(cp, 0);
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

	assert(cp->reference_count >= 1);
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

	assert(cp->reference_count >= 1);
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
	sub_context_ty	*scp;
	string_ty	*hp;
	string_ty	*s;
	pconf		pconf_data;
	string_ty	*the_command;

	/*
	 * Create a new history.  Only ever executed in the ``being
	 * integrated'' state.  Current directory will be set to the
	 * base of the history tree.  All of the substitutions described
	 * in aesub(5) are avaliable.  In addition:
	 *
	 * ${Input}
	 *	absolute path of source file
	 *
	 * ${History}
	 *	absolute path of history file
	 */
	trace(("change_run_history_create_command(cp = %8.8lX, \
filename = \"%s\")\n{\n"/*}*/, (long)cp, filename->str_text));
	assert(cp->reference_count >= 1);

	/*
	 * Get the path of the source file.
	 */
	scp = sub_context_new();
	s = change_file_path(cp, filename);
	sub_var_set(scp, "Input", "%S", s);
	str_free(s);

	/*
	 * Get the path of the history file.
	 */
	hp = project_history_path_get(cp->pp);
	sub_var_set(scp, "History", "%S/%S", hp, filename);

	/*
	 * Ancient compatibility.
	 * I sure hope no-one is still using this.
	 */
	sub_var_set(scp, "1", "${input}");
	sub_var_set(scp, "2", "${history}");

	/*
	 * Make sure the command has been set.
	 */
	pconf_data = change_pconf_get(cp, 1);
	the_command = pconf_data->history_create_command;
	if (!the_command)
	{
		sub_context_ty	*scp2;

		scp2 = sub_context_new();
		sub_var_set(scp2, "File_Name", "%s", THE_CONFIG_FILE);
		sub_var_set(scp2, "FieLD_Name", "history_create_command");
		change_fatal
		(
			cp,
			scp2,
			i18n("$filename: contains no \"$field_name\" field")
		);
		/* NOTREACHED */
		sub_context_delete(scp2);
	}

	/*
	 * Perform the substitutions and run the command.
	 */
	the_command = substitute(scp, cp, the_command);
	sub_context_delete(scp);
	change_env_set(cp, 0);
	project_become(cp->pp);
	os_mkdir_between(hp, filename, 02755);
	os_execute(the_command, OS_EXEC_FLAG_NO_INPUT, hp);
	project_become_undo();
	str_free(the_command);
	trace((/*{*/"}\n"));
}


void
change_run_history_put_command(cp, filename)
	change_ty	*cp;
	string_ty	*filename;
{
	sub_context_ty	*scp;
	string_ty	*hp;
	pconf		pconf_data;
	string_ty	*the_command;
	string_ty	*s;

	/*
	 * Update and existing history.  Only ever happens in the
	 * ``being integrated'' state.  Current directory will be set to
	 * the base of the history tree.  All of the substitutions
	 * described in aesub(5) are avaliable.  In addition:
	 *
	 * ${Input}
	 *	absolute path of source file
	 *
	 * ${History}
	 *	absolute path of history file
	 */
	trace(("change_run_history_put_command(cp = %8.8lX, \
filename = \"%s\")\n{\n"/*}*/, (long)cp, filename->str_text));
	assert(cp->reference_count >= 1);

	/*
	 * Get the path of the source file.
	 */
	scp = sub_context_new();
	s = change_file_path(cp, filename);
	sub_var_set(scp, "Input", "%S", s);
	str_free(s);

	/*
	 * Get the path of the history file.
	 */
	hp = project_history_path_get(cp->pp);
	sub_var_set(scp, "History", "%S/%S", hp, filename);

	/*
	 * Ancient compatibility.
	 * I sure hope no-one is still using this.
	 */
	sub_var_set(scp, "1", "${input}");
	sub_var_set(scp, "2", "${history}");

	/*
	 * Make sure the command has been set.
	 */
	pconf_data = change_pconf_get(cp, 1);
	the_command = pconf_data->history_put_command;
	if (!the_command)
	{
		sub_context_ty	*scp2;

		scp2 = sub_context_new();
		sub_var_set(scp2, "File_Name", "%s", THE_CONFIG_FILE);
		sub_var_set(scp2, "FieLD_Name", "history_get_command");
		change_fatal
		(
			cp,
			scp2,
			i18n("$filename: contains no \"$field_name\" field")
		);
		/* NOTREACHED */
		sub_context_delete(scp2);
	}

	/*
	 * Perform the substitutions and run the command.
	 */
	the_command = substitute(scp, cp, the_command);
	sub_context_delete(scp);
	change_env_set(cp, 0);
	project_become(cp->pp);
	os_execute(the_command, OS_EXEC_FLAG_NO_INPUT, hp);
	project_become_undo();
	str_free(the_command);
	trace((/*{*/"}\n"));
}


string_ty *
change_run_history_query_command(cp, filename)
	change_ty	*cp;
	string_ty	*filename;
{
	sub_context_ty	*scp;
	string_ty	*hp;
	pconf		pconf_data;
	string_ty	*the_command;
	string_ty	*result;

	/*
	 * Ask the history file what its edit number is.  We use this
	 * method because the string returned is essentially random,
	 * between different history programs.  Only ever executed in
	 * the ``being integrated'' state.  Current directory will be
	 * set to the base of the history tree.  All of the
	 * substitutions described in aesub(5) are available.  In
	 * addition
	 *
	 * ${History}
	 *	absolute path of history file
	 */
	trace(("change_run_history_query_command(cp = %8.8lX, \
filename = \"%s\")\n{\n"/*}*/, (long)cp, filename->str_text));
	assert(cp->reference_count >= 1);
	pconf_data = change_pconf_get(cp, 1);
	hp = project_history_path_get(cp->pp);
	scp = sub_context_new();
	sub_var_set(scp, "History", "%S/%S", hp, filename);
	sub_var_set(scp, "1", "${history}");
	the_command = pconf_data->history_query_command;
	if (!the_command)
	{
		sub_context_ty	*scp2;

		scp2 = sub_context_new();
		sub_var_set(scp2, "File_Name", "%s", THE_CONFIG_FILE);
		sub_var_set(scp2, "FieLD_Name", "history_query_command");
		change_fatal
		(
			cp,
			scp2,
			i18n("$filename: contains no \"$field_name\" field")
		);
		/* NOTREACHED */
		sub_context_delete(scp2);
	}
	the_command = substitute(scp, cp, the_command);
	sub_context_delete(scp);

	change_env_set(cp, 0);
	project_become(cp->pp);
	result =
		os_execute_slurp
		(
			the_command,
			OS_EXEC_FLAG_NO_INPUT,
			hp
		);
	project_become_undo();
	str_free(the_command);
	if (!result->str_length)
		fatal_intl(0, i18n("history_query_command return empty"));
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
	sub_context_ty	*scp;
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
		"change_run_diff_command(cp = %8.8lX, up = %8.8lX, \
original = \"%s\", input = \"%s\", output = \"%s\")\n{\n"/*}*/,
		(long)cp,
		(long)up,
		original->str_text,
		input->str_text,
		output->str_text
	));
	assert(cp->reference_count >= 1);
	pconf_data = change_pconf_get(cp, 1);
	if (change_cstate_get(cp)->state == cstate_state_being_developed)
		dd = change_development_directory_get(cp, 0);
	else
		dd = change_integration_directory_get(cp, 0);
	assert(dd);
	scp = sub_context_new();
	sub_var_set(scp, "ORiginal", "%S", original);
	sub_var_set(scp, "Input", "%S", input);
	sub_var_set(scp, "Output", "%S", output);
	sub_var_set(scp, "1", "${original}");
	sub_var_set(scp, "2", "${input}");
	sub_var_set(scp, "3", "${output}");
	the_command = pconf_data->diff_command;
	if (!the_command)
	{
		sub_context_ty	*scp2;

		scp2 = sub_context_new();
		sub_var_set(scp2, "File_Name", "%s", THE_CONFIG_FILE);
		sub_var_set(scp2, "FieLD_Name", "diff_command");
		change_fatal
		(
			cp,
			scp2,
			i18n("$filename: contains no \"$field_name\" field")
		);
		/* NOTREACHED */
		sub_context_delete(scp2);
	}
	the_command = substitute(scp, cp, the_command);
	sub_context_delete(scp);
	trace_string(the_command->str_text);
	change_env_set(cp, 0);
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
	sub_context_ty	*scp;
	pconf		pconf_data;
	string_ty	*the_command;
	string_ty	*dd;

	/*
	 * Run the diff3_command - or the merge_command.
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
		"change_run_diff3_command(cp = %8.8lX, up = %8.8lX, original = \
\"%s\", most_recent = \"%s\", input = \"%s\", output = \"%s\")\n{\n"/*}*/,
		(long)cp,
		(long)up,
		original->str_text,
		most_recent->str_text,
		input->str_text,
		output->str_text
	));
	assert(cp->reference_count >= 1);
	pconf_data = change_pconf_get(cp, 1);
	dd = change_development_directory_get(cp, 0);
	scp = sub_context_new();
	sub_var_set(scp, "ORiginal", "%S", original);
	sub_var_set(scp, "Most_Recent", "%S", most_recent);
	sub_var_set(scp, "Input", "%S", input);
	sub_var_set(scp, "Output", "%S", output);
	sub_var_set(scp, "1", "${original}");
	sub_var_set(scp, "2", "${most_recent}");
	sub_var_set(scp, "3", "${input}");
	sub_var_set(scp, "4", "${output}");
	the_command = pconf_data->diff3_command;
	if (!the_command)
	{
		sub_context_ty	*scp2;

		scp2 = sub_context_new();
		sub_var_set(scp2, "File_Name", "%s", THE_CONFIG_FILE);
		sub_var_set(scp2, "FieLD_Name", "diff3_command");
		change_fatal
		(
			cp,
			scp2,
			i18n("$filename: contains no \"$field_name\" field")
		);
		/* NOTREACHED */
		sub_context_delete(scp2);
	}
	the_command = substitute(scp, cp, the_command);
	sub_context_delete(scp);

	change_env_set(cp, 0);
	user_become(up);
	if (os_exists(output))
		os_unlink(output);
	os_execute(the_command, OS_EXEC_FLAG_NO_INPUT, dd);
	user_become_undo();
	str_free(the_command);
	trace((/*{*/"}\n"));
}


void
change_run_merge_command(cp, up, original, most_recent, input, output)
	change_ty	*cp;
	user_ty		*up;
	string_ty	*original;
	string_ty	*most_recent;
	string_ty	*input;
	string_ty	*output;
{
	sub_context_ty	*scp;
	pconf		pconf_data;
	string_ty	*the_command;
	string_ty	*dd;

	/*
	 * Run the merge_command.
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
		"change_run_merge_command(cp = %8.8lX, up = %8.8lX, original = \
\"%s\", most_recent = \"%s\", input = \"%s\", output = \"%s\")\n{\n"/*}*/,
		(long)cp,
		(long)up,
		original->str_text,
		most_recent->str_text,
		input->str_text,
		output->str_text
	));
	assert(cp->reference_count >= 1);
	pconf_data = change_pconf_get(cp, 1);
	dd = change_development_directory_get(cp, 0);
	scp = sub_context_new();
	sub_var_set(scp, "ORiginal", "%S", original);
	sub_var_set(scp, "Most_Recent", "%S", most_recent);
	sub_var_set(scp, "Input", "%S", input);
	sub_var_set(scp, "Output", "%S", output);
	the_command = pconf_data->merge_command;
	if (!the_command)
	{
		sub_context_ty	*scp2;

		scp2 = sub_context_new();
		sub_var_set(scp2, "File_Name", "%s", THE_CONFIG_FILE);
		sub_var_set(scp2, "FieLD_Name", "merge_command");
		change_fatal
		(
			cp,
			scp2,
			i18n("$filename: contains no \"$field_name\" field")
		);
		/* NOTREACHED */
		sub_context_delete(scp2);
	}
	the_command = substitute(scp, cp, the_command);
	sub_context_delete(scp);

	change_env_set(cp, 0);
	user_become(up);
	if (os_exists(output))
		os_unlink(output);
	os_execute(the_command, OS_EXEC_FLAG_NO_INPUT, dd);
	user_become_undo();
	str_free(the_command);
	trace((/*{*/"}\n"));
}


int
change_has_merge_command(cp)
	change_ty	*cp;
{
	pconf		pconf_data;

	pconf_data = change_pconf_get(cp, 1);
	return !!pconf_data->merge_command;
}


void
change_run_develop_begin_command(cp, up)
	change_ty	*cp;
	user_ty		*up;
{
	pconf		pconf_data;
	string_ty	*the_command;
	string_ty	*dir;

	assert(cp->reference_count >= 1);
	pconf_data = change_pconf_get(cp, 0);
	if (!pconf_data->develop_begin_command)
		return;

	the_command = pconf_data->develop_begin_command;
	the_command = substitute(0, cp, the_command);
	dir = change_development_directory_get(cp, 1);
	change_env_set(cp, 0);
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
	sub_context_ty	*scp;
	pconf		pconf_data;
	string_ty	*the_command;
	string_ty	*dir;

	assert(cp->reference_count >= 1);
	pconf_data = change_pconf_get(cp, 0);
	if (!pconf_data->integrate_begin_command)
		return;
	scp = sub_context_new();
	sub_var_set(scp, "1", "${project}");
	sub_var_set(scp, "2", "${change}");
	sub_var_set(scp, "3", "${version}");
	the_command = pconf_data->integrate_begin_command;
	the_command = substitute(scp, cp, the_command);
	sub_context_delete(scp);

	dir = change_integration_directory_get(cp, 1);
	change_env_set(cp, 0);
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

	assert(cp->reference_count >= 1);
	cstate_data = change_cstate_get(cp);
	assert(cstate_data->architecture);
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

	assert(cp->reference_count >= 1);
	cstate_data = change_cstate_get(cp);
	if (!cstate_data->architecture)
		cstate_data->architecture =
			cstate_architecture_list_type.alloc();
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
change_architecture_name(cp, with_arch)
	change_ty	*cp;
	int		with_arch;
{
	trace(("change_architecture_name(cp = %8.8lX)\n{\n"/*}*/, (long)cp));
	assert(cp->reference_count >= 1);
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
		if (!result && with_arch)
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "Name", "%S", un);
			change_fatal(cp, scp, i18n("architecture \"$name\" unknown"));
			/* NOTREACHED */
			sub_context_delete(scp);
		}
		cp->architecture_name = result;
	}
	trace_string(cp->architecture_name ? cp->architecture_name->str_text : "");
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
	trace(("change_architecture_times_find(cp = %8.8lX, un = %8.8lX)\n{\n"
		/*}*/, (long)cp, (long)un));
	assert(cp->reference_count >= 1);
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
	trace(("return %8.8lX;\n", (long)tp));
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
	trace(("find_architecture_variant(cp = %8.8lX)\n{\n"/*}*/, (long)cp));
	assert(cp->reference_count >= 1);
	an = change_architecture_name(cp, 1);

	/*
	 * find this variant in the times list
	 */
	tp = change_architecture_times_find(cp, an);

	/*
	 * adjust the node
	 */
	str_free(tp->node);
	tp->node = str_copy(uname_node_get());
	trace(("return %8.8lX;\n", (long)tp));
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
	trace(("change_build_time_set(cp = %8.8lX)\n{\n"/*}*/, (long)cp));
	assert(cp->reference_count >= 1);
	tp = find_architecture_variant(cp);
	time(&tp->build_time);
	tp->test_time = 0;
	tp->test_baseline_time = 0; /* XXX */
	tp->regression_test_time = 0;

	/*
	 * set the build_time in the change state.
	 * figure the oldest time of all variants.
	 * if one is missing, then is zero.
	 */
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
	cstate_data->test_baseline_time = 0; /* XXX */
	cstate_data->regression_test_time = 0;
	trace((/*{*/"}\n"));
}


void
change_test_time_set(cp, when)
	change_ty	*cp;
	time_t		when;
{
	long		j, k;
	cstate_architecture_times tp;
	cstate		cstate_data;

	/*
	 * set the test_time in the architecture variant record
	 */
	assert(cp->reference_count >= 1);
	tp = find_architecture_variant(cp);
	tp->test_time = when;

	/*
	 * set the test_time in the change state.
	 * figure the oldest time of all variants.
	 * if one is missing, then is zero.
	 */
	cstate_data = change_cstate_get(cp);
	cstate_data->test_time = tp->test_time;
	if (!when)
		return;
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
change_test_baseline_time_set(cp, when)
	change_ty	*cp;
	time_t		when;
{
	long		j, k;
	cstate_architecture_times tp;
	cstate		cstate_data;

	/*
	 * set the test_baseline_time in the architecture variant record
	 */
	assert(cp->reference_count >= 1);
	tp = find_architecture_variant(cp);
	tp->test_baseline_time = when;

	/*
	 * set the test_baseline_time in the change state.
	 * figure the oldest time of all variants.
	 * if one is missing, then is zero.
	 */
	cstate_data = change_cstate_get(cp);
	cstate_data->test_baseline_time = tp->test_baseline_time;
	if (!when)
		return;
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
	assert(cp->reference_count >= 1);
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


/*
 * NAME
 *	change_test_times_clear
 *
 * SYNOPSIS
 *	void change_test_times_clear(change_ty *);
 *
 * DESCRIPTION
 *	The change_test_times_clear function is called by aeb to reset
 *	the test times after a build.
 */

void
change_test_times_clear(cp)
	change_ty	*cp;
{
	cstate_architecture_times tp;
	cstate		cstate_data;
	size_t		j;
	string_ty	*variant;

	/*
	 * reset the test times in the architecture variant record
	 */
	assert(cp->reference_count >= 1);
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

	/*
	 * reset file test times
	 */
	variant = change_architecture_name(cp, 1);
	for (j = 0; ; ++j)
	{
		fstate_src	src_data;
		fstate_src_architecture_times_list atlp;
		size_t		k;

		src_data = change_file_nth(cp, j);
		if (!src_data)
			break;
		atlp = src_data->architecture_times;
		if (!atlp)
			continue;

		/*
		 * only reset the relevant architecture
		 */
		for (k = 0; k < atlp->length; ++k)
		{
			fstate_src_architecture_times atp;

			atp = atlp->list[k];
			if
			(
				/* bug if not set */
				atp->variant
			&&
				str_equal(atp->variant, variant)
			)
			{
				atp->test_time = 0;
				atp->test_baseline_time = 0;
				break;
			}
		}
	}
}


void
change_build_times_clear(cp)
	change_ty	*cp;
{
	cstate		cstate_data;
	size_t		j;

	/*
	 * reset the build and test times in all architecture variant records
	 */
	assert(cp->reference_count >= 1);
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
	cstate_data->test_baseline_time = 0; /* XXX */
	cstate_data->regression_test_time = 0;

	/*
	 * reset file test times
	 */
	for (j = 0; ; ++j)
	{
		fstate_src	src_data;

		src_data = change_file_nth(cp, j);
		if (!src_data)
			break;
		if (src_data->architecture_times)
		{
			fstate_src_architecture_times_list_type.free
			(
				src_data->architecture_times
			);
			src_data->architecture_times = 0;
		}
	}
}


static char *outstanding_commentary _((string_list_ty *, long));

static char *
outstanding_commentary(wlp, narch)
	string_list_ty		*wlp;
	long		narch;
{
	static string_ty *s;

	if (s)
	{
		str_free(s);
		s = 0;
	}
	if (!wlp->nstrings || wlp->nstrings >= narch)
		s = str_from_c("");
	else
	{
		sub_context_ty	*scp;
		string_ty	*t1;
		string_ty	*t2;
		size_t		j;

		t1 = str_format("\"%S\"", wlp->string[0]);
		for (j = 1; j < wlp->nstrings; ++j)
		{
			scp = sub_context_new();
			sub_var_set(scp, "Name1", "%S", t1);
			sub_var_set(scp, "Name2", "\"%S\"", wlp->string[j]);
			if (j == wlp->nstrings - 1)
				t2 = subst_intl(scp, i18n("$name1 and $name2"));
			else
				t2 = subst_intl(scp, i18n("$name1, $name2"));
			sub_context_delete(scp);
			str_free(t1);
			t1 = t2;
		}
		scp = sub_context_new();
		sub_var_set(scp, "Name_List", "%S", t1);
		sub_var_set(scp, "Number", "%ld", (long)wlp->nstrings);
		sub_var_optional(scp, "Number");
		s = subst_intl(scp, i18n("for the $name_list architectures"));
		sub_context_delete(scp);
		str_free(t1);
	}
	string_list_destructor(wlp);
	return s->str_text;
}


char *
change_outstanding_builds(cp, t)
	change_ty	*cp;
	time_t		t;
{
	cstate		cstate_data;
	string_list_ty		wl;
	cstate_architecture_times tp;
	long		j;

	assert(cp->reference_count >= 1);
	cstate_data = change_cstate_get(cp);
	string_list_constructor(&wl);
	for (j = 0; j < cstate_data->architecture->length; ++j)
	{
		tp =
			change_architecture_times_find
			(
				cp,
				cstate_data->architecture->list[j]
			);
		if (!tp->build_time || tp->build_time < t)
			string_list_append_unique(&wl, tp->variant);
	}
	return outstanding_commentary(&wl, cstate_data->architecture->length);
}


char *
change_outstanding_tests(cp, t)
	change_ty	*cp;
	time_t		t;
{
	cstate		cstate_data;
	string_list_ty		wl;
	cstate_architecture_times tp;
	long		j;

	assert(cp->reference_count >= 1);
	cstate_data = change_cstate_get(cp);
	string_list_constructor(&wl);
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
			string_list_append_unique(&wl, tp->variant);
	}
	return outstanding_commentary(&wl, cstate_data->architecture->length);
}


char *
change_outstanding_tests_baseline(cp, t)
	change_ty	*cp;
	time_t		t;
{
	cstate		cstate_data;
	string_list_ty		wl;
	cstate_architecture_times tp;
	long		j;

	assert(cp->reference_count >= 1);
	cstate_data = change_cstate_get(cp);
	string_list_constructor(&wl);
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
			string_list_append_unique(&wl, tp->variant);
	}
	return outstanding_commentary(&wl, cstate_data->architecture->length);
}


char *
change_outstanding_tests_regression(cp, t)
	change_ty	*cp;
	time_t		t;
{
	cstate		cstate_data;
	string_list_ty		wl;
	cstate_architecture_times tp;
	long		j;

	assert(cp->reference_count >= 1);
	cstate_data = change_cstate_get(cp);
	string_list_constructor(&wl);
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
			string_list_append_unique(&wl, tp->variant);
	}
	return outstanding_commentary(&wl, cstate_data->architecture->length);
}


void
change_architecture_from_pconf(cp)
	change_ty	*cp;
{
	pconf		pconf_data;
	long		j;

	assert(cp->reference_count >= 1);
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

	assert(cp->reference_count >= 1);
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
	string_list_ty		wl;
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
	if (wl.nstrings < 1 || wl.nstrings > 2)
		goto done;
	if (wl.string[0]->str_length < 1 || wl.string[0]->str_length > 8)
		goto done;
	if
	(
		wl.nstrings > 1
	&&
		(wl.string[1]->str_length < 1 || wl.string[1]->str_length > 3)
	)
		goto done;

	/*
	 * make sure the characters are acceptable
	 * only allow alphanumerics
	 */
	for (j = 0; j < wl.nstrings; ++j)
	{
		char	*tp;

		for (tp = wl.string[j]->str_text; *tp; ++tp)
		{
			if (!isalnum((unsigned char)*tp))
				goto done;
		}
	}
	result = 1;
	
	done:
	string_list_destructor(&wl);
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

	assert(cp->reference_count >= 1);
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

	assert(cp->reference_count >= 1);
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

	assert(cp->reference_count >= 1);
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
		 * (This is a "C" locale test.)
		 */
		for (s = fn->str_text; *s; ++s)
			if (!isgraph((unsigned char)*s))
				return 0;
	}
	return 1;
}


static int change_filename_shell_safe _((change_ty *, string_ty *));

static int
change_filename_shell_safe(cp, fn)
	change_ty	*cp;
	string_ty	*fn;
{
	pconf		pconf_data;
	char		*s;

	/*
	 * Though not a shell special character as such, always disallow
	 * a leading minus.  This is because it is the option introducer,
	 * and will cause confusion if allowed into filenames.	Also,
	 * there is no elegant and context-free way to quote it.
	 */
	assert(cp->reference_count >= 1);
	pconf_data = change_pconf_get(cp, 0);
	if (fn->str_text[0] == '-')
		return 0;
	if (!pconf_data->shell_safe_filenames)
		return 1;

	/*
	 * Some shells also treat a leading tilde (~) as meaning ``home
	 * directory of''
	 */
	if (fn->str_text[0] == '~')
		return 0;

	/*
	 * The rest of the restrictions apply to all characters of
	 * the filename.
	 */
	for (s = fn->str_text; ; ++s)
	{
		switch ((unsigned char)*s)
		{
		default:
			continue;

		case 0:
			break;

		case '!': case '"': case '#': case '$': case '&': case '\'':
		case '(': case ')': case '*': case ':': case ';': case '<':
		case '=': case '>': case '?': case '[': case '\\': case ']':
		case '^': case '`': case '{': case '|': case '}':
			return 0;
		}
		break;
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
	string_list_ty		part;
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
	trace(("change_filename_check(cp = %8l8lX, filename = \"%s\", nodup = %d)\n{\n"/*}*/,
		(long)cp, filename->str_text, nodup));
	assert(cp->reference_count >= 1);
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
	for (k = 0; k < part.nstrings; ++k)
	{
		int		max;

		/*
		 * check for collision
		 */
		if (nodup)
		{
			fstate_src	src_data;

			s2 = wl2str(&part, 0, k, "/");
			src_data = project_file_find(cp->pp, s2);
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
					part.nstrings == 1
				||
					str_equal(s2, filename)
				)
				{
					sub_context_ty	*scp;

					scp = sub_context_new();
					sub_var_set(scp, "File_Name", "%S", filename);
					result =
						subst_intl
						(
							scp,
			    i18n("file \"$filename\" already exists in project")
						);
					sub_context_delete(scp);
				}
				else
				{
					sub_context_ty	*scp;

					scp = sub_context_new();
					sub_var_set(scp, "File_Name1", "%S", filename);
					sub_var_set(scp, "File_Name2", "%S", s2);
					result =
						subst_intl
						(
							scp,
	i18n("file \"$filename1\" collides with file \"$filename2\" in project")
						);
					sub_context_delete(scp);
				}
				str_free(s2);
				goto done;
			}
			str_free(s2);
		}

		/*
		 * check DOS-full-ness
		 */
		s2 = part.string[k];
		if (!is_a_dos_filename(cp, s2))
		{
			sub_context_ty	*scp;
			string_ty	*s3;

			s3 = abbreviate_8dos3(s2);
			scp = sub_context_new();
			sub_var_set(scp, "File_Name", "%S", filename);
			sub_var_set(scp, "SUGgest", "%S", s3);
			sub_var_optional(scp, "SUGgest");
			if (part.nstrings == 1)
			{
				result =
					subst_intl
					(
						scp,
			    i18n("file name \"$filename\" not suitable for DOS")
					);
			}
			else
			{
				sub_var_set(scp, "Part", "%S", s2);
				result =
					subst_intl
					(
						scp,
		  i18n("file \"$filename\" part \"$part\" not suitable for DOS")
					);
			}
			sub_context_delete(scp);
			str_free(s3);
			goto done;
		}

		/*
		 * check name length
		 */
		if (k == part.nstrings - 1)
			max = name_max2;
		else
			max = name_max1;
		if (s2->str_length > max)
		{
			sub_context_ty	*scp;
			string_ty	*s3;

			scp = sub_context_new();
			if (k == part.nstrings - 1)
				s3 = abbreviate_filename(s2, max);
			else
				s3 = abbreviate_dirname(s2, max);
			sub_var_set(scp, "File_Name", "%S", filename);
			sub_var_set(scp, "Number", "%d", (int)(s2->str_length - max));
			sub_var_optional(scp, "Number");
			sub_var_set(scp, "SUGgest", "%S", s3);
			sub_var_optional(scp, "SUGgest");
			if (part.nstrings == 1)
			{
				result =
					subst_intl
					(
						scp,
	       i18n("file \"$filename\" too long, suggest \"$suggest\" instead")
					);
			}
			else
			{
				sub_var_set(scp, "Part", "%S", s2);
				result =
					subst_intl
					(
						scp,
i18n("file \"$filename\" part \"$part\" too long, suggest \"$suggest\" instead")
					);
			}
			sub_context_delete(scp);
			str_free(s3);
			goto done;
		}

		/*
		 * check for logfile
		 */
		if (str_equal(s2, change_logfile_basename()))
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "File_Name", "%S", filename);
			if (part.nstrings == 1)
			{
				result =
					subst_intl
					(
						scp,
					  i18n("file \"$filename\" is log file")
					);
			}
			else
			{
				sub_var_set(scp, "Part", "%S", s2);
				result =
					subst_intl
					(
						scp,
			   i18n("file \"$filename\" part \"$part\" is log file")
					);
			}
			sub_context_delete(scp);
			goto done;
		}

		/*
		 * check filename for valid characters
		 */
		if (!change_filename_in_charset(cp, s2))
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "File_Name", "%S", filename);
			if (part.nstrings == 1)
			{
				result =
					subst_intl
					(
						scp,
		     i18n("file name \"$filename\" contains illegal characters")
					);
			}
			else
			{
				sub_var_set(scp, "Part", "%S", s2);
				result =
					subst_intl
					(
						scp,
	   i18n("file \"$filename\" part \"$part\" contains illegal characters")
					);
			}
			sub_context_delete(scp);
			goto done;
		}

		/*
		 * check filename for shellspecial characters
		 */
		if (!change_filename_shell_safe(cp, s2))
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "File_Name", "%S", filename);
			if (part.nstrings == 1)
			{
				result =
					subst_intl
					(
						scp,
	       i18n("file name \"$filename\" contains shell special characters")
					);
			}
			else
			{
				sub_var_set(scp, "Part", "%S", s2);
				result =
					subst_intl
					(
						scp,
     i18n("file \"$filename\" part \"$part\" contains shell special characters")
					);
			}
			sub_context_delete(scp);
			goto done;
		}
		if (!filename_pattern_test(cp, s2))
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "File_Name", "%S", filename);
			if (part.nstrings == 1)
			{
				result =
					subst_intl
					(
						scp,
					i18n("file \"$filename\" bombs filters")
					);
			}
			else
			{
				sub_var_set(scp, "Part", "%S", s2);
				result =
					subst_intl
					(
						scp,
			 i18n("file \"$filename\" part \"$part\" bombs filters")
					);
			}
			sub_context_delete(scp);
			goto done;
		}
	}

	/*
	 * here for all exits
	 */
	done:
	string_list_destructor(&part);
	trace(("return %d;\n", result));
	trace((/*{*/"}\n"));
	return result;
}


typedef struct slink_info_ty slink_info_ty;
struct slink_info_ty
{
	string_ty	*bl;
	string_ty	*dd;
	change_ty	*cp;
	pconf		pconf_data;
	int		minimum;
	user_ty		*up;
};


static void csltbl1 _((void *, dir_walk_message_ty, string_ty *,
	struct stat *));

static void
csltbl1(p, msg, path, st)
	void		*p;
	dir_walk_message_ty msg;
	string_ty	*path;
	struct stat	*st;
{
	slink_info_ty	*sip;
	string_ty	*s1;
	string_ty	*s2;
	long		j;
	pconf_symlink_exceptions_list lp;

	sip = p;
	s1 = os_below_dir(sip->bl, path);
	assert(s1);
	s2 = os_path_cat(sip->dd, s1);
	switch (msg)
	{
	case dir_walk_dir_before:
		if (!os_exists(s2))
			os_mkdir(s2, 02755);
		break;

	case dir_walk_dir_after:
		break;

	case dir_walk_special:
	case dir_walk_symlink:
		/*
		 * The minimum option says to only link project source
		 * files.  These two can't ever be source files.
		 */
		if (sip->minimum)
			break;
		/* fall through... */

	case dir_walk_file:
		/*
		 * The minimum option says to only link project source
		 * files.  This simulates -minimum integration builds.
		 */
		if (sip->minimum)
		{
			int	not_a_project_source_file;

			user_become_undo();
			not_a_project_source_file =
				!project_file_find(sip->cp->pp, s1);
			user_become(sip->up);
			if (not_a_project_source_file)
				break;
		}

		/*
		 * If there is already something there, don't do anything.
		 * It doesn't matter whethyer it's a symlink or a file.
		 */
		if (os_exists(s2))
			break;

		/*
		 * avoid the symlink exceptions
		 */
		lp = sip->pconf_data->symlink_exceptions;
		assert(lp);
		for (j = 0; j < lp->length; ++j)
		{
			if (str_equal(s1, lp->list[j]))
				break;
		}
		if (j < lp->length)
			break;

		/*
		 * make the symbolic link
		 */
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
	slink_info_ty	*sip;
	string_ty	*s1;

	/*
	 * walk the development directory,
	 * removing symlinks which point
	 * to non-existent files
	 */
	sip = p;
	switch (msg)
	{
	case dir_walk_dir_before:
	case dir_walk_dir_after:
	case dir_walk_file:
	case dir_walk_special:
		break;

	case dir_walk_symlink:
		/*
		 * If we are asked to do a minimum job, and the file is
		 * not a project source file, remove the symbolic link.
		 */
		if (sip->minimum)
		{
			int	not_a_project_source_file;

			s1 = os_below_dir(sip->bl, path);
			assert(s1);
			user_become_undo();
			not_a_project_source_file =
				!project_file_find(sip->cp->pp, s1);
			user_become(sip->up);
			if (not_a_project_source_file)
			{
				str_free(s1);
				os_unlink(path);
				break;
			}
			str_free(s1);
		}

		/*
		 * If the link points to a non-existent file, remove
		 * the link.
		 */
		s1 = os_readlink(path);
		if (s1->str_text[0] == '/' && !os_exists(s1))
			os_unlink(path);
		str_free(s1);
		break;
	}
}


void
change_create_symlinks_to_baseline(cp, pp, up, minimum)
	change_ty	*cp;
	project_ty	*pp;
	user_ty		*up;
	int		minimum;
{
	slink_info_ty	si;
	cstate		cstate_data;

	/*
	 * For each ancestor, create symlinks from the development
	 * directory to that ancestor's baseline if the file does not
	 * already exist.
	 */
	trace(("change_create_symlinks_to_baseline(cp = %8.8lX)\n{\n"/*}*/,
		(long)cp));
	assert(cp->reference_count >= 1);
	change_verbose(cp, 0, i18n("creating symbolic links to baseline"));
	cstate_data = change_cstate_get(cp);
	si.cp = cp;
	if (cstate_data->state == cstate_state_being_integrated)
		si.dd = change_integration_directory_get(cp, 0);
	else
		si.dd = change_development_directory_get(cp, 0);
	si.pconf_data = change_pconf_get(cp, 0);
	si.minimum = minimum;
	si.up = up;
	while (pp)
	{
		si.bl = project_baseline_path_get(pp, 0);
		user_become(up);
		dir_walk(si.bl, csltbl1, &si);
		user_become_undo();
		pp = pp->parent;
	}

	/*
	 * For each symlink in the development directory that points to
	 * a non-existent file, remove it.
	 */
	user_become(up);
	si.bl = si.dd;
	dir_walk(si.dd, csltbl2, &si);
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
	slink_info_ty	*sip;
	string_ty	*s1;
	string_ty	*s2;

	/*
	 * remove symlinks in the development directory
	 * which point to their counterpart in the baseline
	 */
	if (msg != dir_walk_symlink)
		return;
	sip = p;
	s1 = os_below_dir(sip->dd, path);
	s2 = os_path_cat(sip->bl, s1);
	str_free(s1);
	s1 = os_readlink(path);
	if (str_equal(s1, s2))
		os_unlink(path);
	str_free(s1);
	str_free(s2);
}


void
change_remove_symlinks_to_baseline(cp, pp, up)
	change_ty	*cp;
	project_ty	*pp;
	user_ty		*up;
{
	slink_info_ty	si;
	cstate		cstate_data;

	trace(("change_remove_symlinks_to_baseline(cp = %8.8lX)\n{\n"/*}*/,
		(long)cp));
	assert(cp->reference_count >= 1);
	change_verbose(cp, 0, i18n("removing symbolic links to baseline"));
	/*
	 * get the baseline now, so when get inside walk func,
	 * do not cause problem with multiple user-id setting.
	 */
	cstate_data = change_cstate_get(cp);
	si.cp = 0;
	if (cstate_data->state == cstate_state_being_integrated)
		si.dd = change_integration_directory_get(cp, 0);
	else
		si.dd = change_development_directory_get(cp, 0);
	si.pconf_data = 0;
	while (pp)
	{
		si.bl = project_baseline_path_get(pp, 0);
		user_become(up);
		dir_walk(si.dd, rsltbl, &si);
		user_become_undo();
		pp = pp->parent;
	}
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
	sub_context_ty	*scp;
	int		flags;
	int		result;

	assert(cp->reference_count >= 1);
	scp = sub_context_new();
	sub_var_set(scp, "File_Name", "%S", filename);
	the_command = substitute(scp, cp, the_command);
	sub_context_delete(scp);

	flags = inp ? OS_EXEC_FLAG_INPUT : OS_EXEC_FLAG_NO_INPUT;
	change_env_set(cp, 1);
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

	assert(cp->reference_count >= 1);
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

	assert(cp->reference_count >= 1);
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
	sub_context_ty	*scp;
	pconf		pconf_data;
	string_ty	*the_command;
	string_ty	*id;

	assert(cp->reference_count >= 1);
	assert(cp->cstate_data);
	assert(cp->cstate_data->state == cstate_state_being_integrated);
	pconf_data = change_pconf_get(cp, 1);
	assert(pconf_data);
	the_command = pconf_data->build_command;
	assert(the_command);
	if (!the_command)
	{
		scp = sub_context_new();
		sub_var_set(scp, "File_Name", "%s", THE_CONFIG_FILE);
		sub_var_set(scp, "FieLD_Name", "build_command");
		change_fatal
		(
			cp,
			scp,
			i18n("$filename: contains no \"$field_name\" field")
		);
		/* NOTREACHED */
		sub_context_delete(scp);
	}

	/*
	 * %1 = project name
	 * %2 = change number
	 * %3 = identifying string, in the form "a.b.Dnnn"
	 *	where 'a' is the major version number,
	 *	'b' is the minor version number,
	 *	and 'nnn' is the build number.
	 */
	scp = sub_context_new();
	sub_var_set(scp, "1", "${project}");
	sub_var_set(scp, "2", "${change}");
	sub_var_set(scp, "3", "${version}");
	sub_var_set(scp, "File_List", "");
	sub_var_optional(scp, "File_List");
	the_command = substitute(scp, cp, the_command);
	sub_context_delete(scp);

	id = change_integration_directory_get(cp, 0);
	change_env_set(cp, 1);
	project_become(cp->pp);
	os_execute(the_command, OS_EXEC_FLAG_NO_INPUT, id);
	project_become_undo();
	str_free(the_command);
}


void
change_run_development_build_command(cp, up, partial)
	change_ty	*cp;
	user_ty		*up;
	string_list_ty		*partial;
{
	sub_context_ty	*scp;
	pconf		pconf_data;
	string_ty	*the_command;
	string_ty	*dd;

	assert(cp->reference_count >= 1);
	assert(cp->cstate_data);
	assert(cp->cstate_data->state == cstate_state_being_developed);
	pconf_data = change_pconf_get(cp, 1);
	assert(pconf_data);
	the_command = pconf_data->development_build_command;
	if (!the_command)
	{
		scp = sub_context_new();
		sub_var_set(scp, "File_Name", "%s", THE_CONFIG_FILE);
		sub_var_set(scp, "FieLD_Name", "build_command");
		change_fatal
		(
			cp,
			scp,
			i18n("$filename: contains no \"$field_name\" field")
		);
		/* NOTREACHED */
		sub_context_delete(scp);
	}

	/*
	 * place the partial file name into the File_List variable
	 */
	scp = sub_context_new();
	if (partial->nstrings)
	{
		string_ty	*s;

		s = wl2str(partial, 0, partial->nstrings, (char *)0);
		sub_var_set(scp, "File_List", "%S", s);
		str_free(s);
		sub_var_append_if_unused(scp, "File_List");
	}
	else
	{
		sub_var_set(scp, "File_List", "");
		sub_var_optional(scp, "File_List");
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
	sub_var_set(scp, "1", "${project}");
	sub_var_set(scp, "2", "${change}");
	sub_var_set(scp, "3", "${version}");
	sub_var_set(scp, "4", "${baseline}");
	the_command = substitute(scp, cp, the_command);
	sub_context_delete(scp);

	dd = change_development_directory_get(cp, 0);
	change_env_set(cp, 1);
	user_become(up);
	os_execute(the_command, OS_EXEC_FLAG_NO_INPUT, dd);
	os_become_undo();
	str_free(the_command);
}


void
change_rescind_test_exemption(cp)
	change_ty	*cp;
{
	cstate		cstate_data;

	assert(cp->reference_count >= 1);
	cstate_data = change_cstate_get(cp);
	cstate_data->test_exempt = 0;
}


void
change_check_architectures(cp)
	change_ty	*cp;
{
	cstate		cstate_data;
	pconf		pconf_data;
	size_t		j, k;
	int		error_count;

	assert(cp->reference_count >= 1);
	cstate_data = change_cstate_get(cp);
	if (!cstate_data->architecture)
		return; /* should not happen */
	pconf_data = change_pconf_get(cp, 1);
	assert(pconf_data->architecture);
	if (!pconf_data->architecture)
		return; /* should not happen */
	error_count = 0;
	for (j = 0; j < cstate_data->architecture->length; ++j)
	{
		string_ty	*variant;

		variant = cstate_data->architecture->list[j];
		for (k = 0; k < pconf_data->architecture->length; ++k)
		{
			if
			(
				str_equal
				(
					variant,
					pconf_data->architecture->list[k]->name
				)
			)
				break;
		}
		if (k >= pconf_data->architecture->length)
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "Name", "%S", variant);
			change_error(cp, scp, i18n("architecture \"$name\" unlisted"));
			sub_context_delete(scp);
			++error_count;
		}
	}
	if (error_count)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "Number", "%d", error_count);
		sub_var_optional(scp, "Number");
		change_fatal(cp, scp, i18n("found unlisted architectures"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}
}


string_ty *
change_new_test_filename_get(cp, n, is_auto)
	change_ty	*cp;
	long		n;
	int		is_auto;
{
	sub_context_ty	*scp;
	pconf		pconf_data;
	string_ty	*result;

	assert(cp->reference_count >= 1);
	scp = sub_context_new();
	pconf_data = change_pconf_get(cp, 1);
	assert(pconf_data->new_test_filename);
	sub_var_set(scp, "Hundred", "%ld", n / 100); 
	sub_var_optional(scp, "Hundred");
	sub_var_set(scp, "Number", "%ld", n); 
	/* do not gettext these next two names */
	sub_var_set(scp, "Type", (is_auto ? "automatic" : "manual")); 
	sub_var_optional(scp, "Type");
	result = substitute(scp, cp, pconf_data->new_test_filename);
	sub_context_delete(scp);
	return result;
}


string_ty *
change_development_directory_template(cp, up)
	change_ty	*cp;
	user_ty		*up;
{
	pconf		pconf_data;
	unsigned long	k;
	string_ty	*ddd;
	string_ty	*devdir;

	/*
	 * Get the project config file.  Don't insist that it exists,
	 * because it will not for the first change.
	 */
	assert(cp->reference_count >= 1);
	pconf_data = change_pconf_get(cp, 0);
	assert(pconf_data->new_test_filename);

	/*
	 * If the user did not give the directory to use,
	 * we must construct one.
	 * The length is limited by the available filename
	 * length limit, trim the project name if necessary.
	 */
	ddd = user_default_development_directory(up);
	assert(ddd);

	devdir = 0;
	for (k = 0;; ++k)
	{
		sub_context_ty	*scp;
		char		suffix[30];
		char		*tp;
		unsigned long	n;
		int		exists;

		/*
		 * Construct the magic string.  This is used to
		 * construct unique file names, should there be
		 * a conflict for some reason (usually to do
		 * with truncating file names, or re-using file
		 * names).
		 */
		scp = sub_context_new();
		tp = suffix;
		n = k;
		for (;;)
		{
			*tp++ = (n & 15) + 'C';
			n >>= 4;
			if (!n)
				break;
		}
		*tp = 0;
		sub_var_set(scp, "Magic", "%s", suffix);

		/*
		 * The default development directory is
		 * directory within which the new development
		 * directory is to be placed.  It is needed by
		 * the substitution if it is to crop file names
		 * by the maximum filename length (see the
		 * ${namemax} substitution).
		 */
		sub_var_set(scp, "Default_Development_Directory", "%S", ddd);

		/*
		 * Perform the substitution to construct the
		 * development directory name.
		 */
		devdir =
			substitute
			(
				scp,
				cp,
				pconf_data->development_directory_template
			);
		sub_context_delete(scp);

		/*
		 * See if this path is unique.
		 */
		user_become(up);
		exists = os_exists(devdir);
		user_become_undo();
		if (!exists)
			break;
		str_free(devdir);
	}
	str_free(ddd);
	assert(devdir);
	return devdir;
}

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
 * MANIFEST: functions to manipulate change file state data
 */

#include <change_file.h>
#include <error.h>
#include <fstrcmp.h>
#include <metrics.h>
#include <os.h>
#include <project_file.h>
#include <str_list.h>
#include <sub.h>
#include <symtab.h>
#include <trace.h>


static void fimprove _((fstate));

static void
fimprove(fstate_data)
	fstate		fstate_data;
{
	size_t		j;

	if (!fstate_data->src)
		fstate_data->src = fstate_src_list_type.alloc();

	/*
	 * This covers a transitional glitch in the edit number
	 * semantics.  Very few installed sites will ever need this.
	 */
	for (j = 0; j < fstate_data->src->length; ++j)
	{
		fstate_src	src;

		src = fstate_data->src->list[j];
		/* Historical 2.3 -> 3.0 transition. */
		if (src->edit_number && !src->edit_number_origin)
			src->edit_number_origin = str_copy(src->edit_number);
	}
}


fstate
change_fstate_get(cp)
	change_ty	*cp;
{
	cstate		cstate_data;
	string_ty	*fn;
	size_t		j;

	/*
	 * make sure the change state has been read in,
	 * in case its src field needed to be converted.
	 * (also to ensure lock_sync has been called for both)
	 */
	trace(("change_fstate_get(cp = %08lX)\n{\n"/*}*/, cp));
	cstate_data = change_cstate_get(cp);

	if (!cp->fstate_data)
	{
		fn = change_fstate_filename_get(cp);
		change_become(cp);
		cp->fstate_data = fstate_read_file(fn->str_text);
		change_become_undo();
		fimprove(cp->fstate_data);
	}
	if (!cp->fstate_data->src)
		cp->fstate_data->src = fstate_src_list_type.alloc();

	/*
	 * Create an O(1) index.
	 * This speeds up just about everything.
	 */
	if (!cp->fstate_stp)
	{
		cp->fstate_stp = symtab_alloc(cp->fstate_data->src->length);
		for (j = 0; j < cp->fstate_data->src->length; ++j)
		{
			fstate_src	p;

			p = cp->fstate_data->src->list[j];
			symtab_assign(cp->fstate_stp, p->file_name, p);
		}
	}
	trace(("return %08lX;\n", cp->fstate_data));
	trace((/*{*/"}\n"));
	return cp->fstate_data;
}


fstate_src
change_file_find(cp, file_name)
	change_ty	*cp;
	string_ty	*file_name;
{
	fstate_src	result;

	trace(("change_file_find(cp = %08lX, file_name = \"%s\")\n{\n"/*}*/,
		(long)cp, file_name->str_text));
	change_fstate_get(cp);
	assert(cp->fstate_stp);
	result = symtab_query(cp->fstate_stp, file_name);
	trace(("return %08lX;\n", result));
	trace((/*{*/"}\n"));
	return result;
}


fstate_src
change_file_find_fuzzy(cp, file_name)
	change_ty	*cp;
	string_ty	*file_name;
{
	string_ty	*best_file_name;
	fstate_src	best;

	trace(("change_file_find_fuzzy(cp = %08lX, fn = \"%s\")\n{\n"/*}*/,
		(long)cp, file_name->str_text));
	change_fstate_get(cp);
	assert(cp->fstate_stp);
	best_file_name = symtab_query_fuzzy(cp->fstate_stp, file_name);
	if (!best_file_name)
		best = 0;
	else
		best = symtab_query(cp->fstate_stp, best_file_name);
	trace(("return %08lX;\n", best));
	trace((/*{*/"}\n"));
	return best;
}


string_ty *
change_file_path(cp, file_name)
	change_ty	*cp;
	string_ty	*file_name;
{
	cstate		cstate_data;
	fstate_src	src_data;
	int		j;
	string_ty	*result;

	trace(("change_file_path(cp = %08lX, file_name = \"%s\")\n{\n"/*}*/,
		(long)cp, file_name->str_text));
	change_fstate_get(cp);
	assert(cp->fstate_stp);
	result = 0;
	src_data = symtab_query(cp->fstate_stp, file_name);
	if (!src_data)
	{
		trace(("return NULL;\n"));
		trace((/*{*/"}\n"));
		return 0;
	}
	if (src_data->about_to_be_copied_by)
	{
		trace(("return NULL;\n"));
		trace((/*{*/"}\n"));
		return 0;
	}

	/*
	 * Files which are built could be any where in the change search
	 * path.  Go hunting for such files.
	 */
	if (src_data->usage == file_usage_build)
	{
		string_list_ty	search_path;

		change_search_path_get(cp, &search_path, 0);
		assert(search_path.nstrings >= 1);
		result = 0;
		os_become_orig();
		for (j = 0; j < search_path.nstrings; ++j)
		{
			result = os_path_cat(search_path.string[j], file_name);
			if (os_exists(result))
				break;
			str_free(result);
			result = 0;
		}
		os_become_undo();
		if (j >= search_path.nstrings)
			result = os_path_cat(search_path.string[0], file_name);
		assert(result);
		string_list_destructor(&search_path);
		trace(("return \"%s\";\n", result->str_text));
		trace((/*{*/"}\n"));
		return result;
	}

	cstate_data = change_cstate_get(cp);
	switch (cstate_data->state)
	{
	default:
		result = project_file_path(cp->pp, file_name);
		break;

	case cstate_state_being_developed:
	case cstate_state_being_reviewed:
	case cstate_state_awaiting_integration:
		if (cstate_data->branch)
			result =
				str_format
				(
					"%S/baseline/%S",
					change_development_directory_get(cp, 0),
					file_name
				);
		else
			result =
				os_path_cat
				(
					change_development_directory_get(cp, 0),
					file_name
				);
		break;

	case cstate_state_being_integrated:
		result =
			os_path_cat
			(
				change_integration_directory_get(cp, 0),
				file_name
			);
		break;
	}

	trace(("return \"%s\";\n", result->str_text));
	trace((/*{*/"}\n"));
	return result;
}


string_ty *
change_file_source(cp, file_name)
	change_ty	*cp;
	string_ty	*file_name;
{
	string_ty	*result;
	cstate		cstate_data;
	fstate_src	src;

	/*
	 * see if the file is in the change
	 */
	trace(("change_file_source(cp = %08lX, file_name = \"%s\")\n{\n"/*}*/,
		(long)cp, file_name->str_text));
	src = change_file_find(cp, file_name);
	if (src && !src->about_to_be_copied_by)
	{
		result = change_file_path(cp, file_name);
		assert(result);
		trace(("return \"%S\";\n", result->str_text));
		trace((/*{*/"}\n"));
		return result;
	}

	/*
	 * If the change is being integrated, and the file would be in
	 * the branch's baseline, then the path to it is in the
	 * integration directory.
	 */
	cstate_data = change_cstate_get(cp);
	if (cstate_data->state == cstate_state_being_integrated)
	{
		change_ty	*pcp;

		pcp = project_change_get(cp->pp);
		src = change_file_find(pcp, file_name);
		if (src && !src->about_to_be_copied_by)
		{
			string_ty	*id;

			id = change_integration_directory_get(cp, 0);
			result = os_path_cat(id, file_name);
			assert(result);
			trace(("return \"%S\";\n", result->str_text));
			trace((/*{*/"}\n"));
			return result;
		}
	}

	/*
	 * see if the file is in the project
	 */
	if (project_file_find(cp->pp, file_name))
	{

		result = project_file_path(cp->pp, file_name);
		assert(result);
		trace(("return \"%S\";\n", result->str_text));
		trace((/*{*/"}\n"));
		return result;
	}

	/*
	 * no such file
	 */
	trace(("return NULL;\n"));
	trace((/*{*/"}\n"));
	return 0;
}


void
change_file_remove(cp, file_name)
	change_ty	*cp;
	string_ty	*file_name;
{
	fstate		fstate_data;
	int		j;
	fstate_src	src_data;

	trace(("change_file_remove(cp = %08lX, file_name = \"%s\")\n{\n"/*}*/,
		(long)cp, file_name->str_text));
	fstate_data = change_fstate_get(cp);
	assert(fstate_data->src);
	assert(cp->fstate_stp);
	symtab_delete(cp->fstate_stp, file_name);
	for (j = 0; j < fstate_data->src->length; ++j)
	{
		src_data = fstate_data->src->list[j];
		if (!str_equal(src_data->file_name, file_name))
			continue;
		fstate_src_type.free(src_data);
		fstate_data->src->list[j] =
			fstate_data->src->list[--fstate_data->src->length];
		break;
	}
	trace((/*{*/"}\n"));
}


void
change_file_remove_all(cp)
	change_ty	*cp;
{
	fstate		fstate_data;

	trace(("change_file_remove_all(cp = %08lX)\n{\n"/*}*/, (long)cp));
	fstate_data = change_fstate_get(cp);
	assert(fstate_data->src);
	assert(cp->fstate_stp);
	if (fstate_data->src->length)
	{
		fstate_src_list_type.free(fstate_data->src);
		fstate_data->src = fstate_src_list_type.alloc();
	}
	symtab_free(cp->fstate_stp);
	cp->fstate_stp = symtab_alloc(0);
	trace((/*{*/"}\n"));
}


fstate_src
change_file_new(cp, file_name)
	change_ty	*cp;
	string_ty	*file_name;
{
	fstate		fstate_data;
	fstate_src	src_data;
	fstate_src	*src_data_p;
	type_ty		*type_p;

	trace(("change_file_new(cp = %08lX)\n{\n"/*}*/, cp));
	fstate_data = change_fstate_get(cp);
	assert(fstate_data->src);
	src_data_p = fstate_src_list_type.list_parse(fstate_data->src, &type_p);
	assert(type_p == &fstate_src_type);
	src_data = fstate_src_type.alloc();
	*src_data_p = src_data;
	src_data->file_name = str_copy(file_name);
	assert(cp->fstate_stp);
	symtab_assign(cp->fstate_stp, file_name, src_data);
	trace(("return %08lX;\n", src_data));
	trace((/*{*/"}\n"));
	return src_data;
}


fstate_src
change_file_nth(cp, n)
	change_ty	*cp;
	size_t		n;
{
	fstate		fstate_data;

	fstate_data = change_fstate_get(cp);
	assert(fstate_data->src);
	if (n >= fstate_data->src->length)
		return 0;
	return fstate_data->src->list[n];
}


size_t
change_file_count(cp)
	change_ty	*cp;
{
	fstate		fstate_data;

	fstate_data = change_fstate_get(cp);
	assert(fstate_data->src);
	return fstate_data->src->length;
}


static int leading_path_prefix _((string_ty *, string_ty *));

static int
leading_path_prefix(s1, s2)
	string_ty	*s1;
	string_ty	*s2;
{
	return
	(
		!s1->str_length
	||
		(
			s1->str_length < s2->str_length
		&&
			!memcmp(s1->str_text, s2->str_text, s1->str_length)
		&&
			s2->str_text[s1->str_length] == '/'
		)
	);
}


void
change_file_dir(cp, file_name, result_in, result_out)
	change_ty	*cp;
	string_ty	*file_name;
	string_list_ty		*result_in;
	string_list_ty		*result_out;
{
	fstate		fstate_data;
	int		j;
	fstate_src	src_data;

	trace(("change_file_dir(cp = %08lX, file_name = \"%s\")\n{\n"/*}*/,
		(long)cp, file_name->str_text));
	assert(result_in);
	string_list_constructor(result_in);
	if (result_out)
		string_list_constructor(result_out);
	fstate_data = change_fstate_get(cp);
	assert(fstate_data->src);
	for (j = 0; j < fstate_data->src->length; ++j)
	{
		src_data = fstate_data->src->list[j];
		if (src_data->about_to_be_created_by && !src_data->deleted_by)
			continue;
		if (src_data->usage == file_usage_build)
			continue;
		if (leading_path_prefix(file_name, src_data->file_name))
		{
			if (!src_data->deleted_by)
				string_list_append(result_in, src_data->file_name);
			else if (result_out)
				string_list_append(result_out, src_data->file_name);
		}
	}
	trace((/*{*/"}\n"));
}


void
change_search_path_get(cp, wlp, resolve)
	change_ty	*cp;
	string_list_ty		*wlp;
	int		resolve;
{
	cstate		cstate_data;
	project_ty	*ppp;

	string_list_constructor(wlp);
	if (cp->bogus)
	{
		ppp = cp->pp->parent;
		if (ppp)
			project_search_path_get(ppp, wlp, resolve);
		return;
	}
	cstate_data = change_cstate_get(cp);
	switch (cstate_data->state)
	{
	default:
		this_is_a_bug();
		break;

	case cstate_state_being_developed:
	case cstate_state_being_reviewed:
	case cstate_state_awaiting_integration:
		string_list_append(wlp, change_development_directory_get(cp, resolve));
		project_search_path_get(cp->pp, wlp, resolve);
		break;

	case cstate_state_being_integrated:
		string_list_append(wlp, change_integration_directory_get(cp, resolve));
		ppp = cp->pp->parent;
		if (ppp)
			project_search_path_get(ppp, wlp, resolve);
		break;
	}
}


void
change_file_test_time_clear(cp, src_data)
	change_ty	*cp;
	fstate_src	src_data;
{
	fstate_src_architecture_times_list atlp;
	fstate_src_architecture_times atp;
	size_t		j;
	string_ty	*variant;

	/*
	 * We are clearing a test time stamp,
	 * so the change summary must also be cleared 
	 */
	change_test_time_set(cp, (time_t)0);

	/*
	 * find the appropriate architecture record
	 */
	atlp = src_data->architecture_times;
	if (!atlp)
		return;
	variant = change_architecture_name(cp, 1);
	for (j = 0; j < atlp->length; ++j)
	{
		atp = atlp->list[j];
		if (!atp->variant)
			continue; /* probably a bug */
		if (!str_equal(atp->variant, variant))
			continue;

		/*
		 * Clear the time stamp
		 */
		atp->test_time = 0;
		break;
	}
}


void
change_file_test_time_set(cp, src_data, when)
	change_ty	*cp;
	fstate_src	src_data;
	time_t		when;
{
	string_ty	*variant;
	fstate_src_architecture_times_list atlp;
	fstate_src_architecture_times atp;
	size_t		j, k;

	/*
	 * Find the appropriate architecture record;
	 * create a new one if necessary.
	 */
	trace(("change_file_test_time_set(cp = %08lX)\n{\n"/*}*/, cp));
	variant = change_architecture_name(cp, 1);
	if (!src_data->architecture_times)
		src_data->architecture_times =
			fstate_src_architecture_times_list_type.alloc();
	atlp = src_data->architecture_times;
	for (j = 0; j < atlp->length; ++j)
	{
		atp = atlp->list[j];
		if
		(
			/* bug if not set */
			atp->variant
		&&
			str_equal(atp->variant, variant)
		)
			break;
	}
	if (j >= atlp->length)
	{
		fstate_src_architecture_times *addr;
		type_ty		*type_p;

		addr =
			fstate_src_architecture_times_list_type.list_parse
			(
				atlp,
				&type_p
			);
		assert(type_p == &fstate_src_architecture_times_type);
		atp = fstate_src_architecture_times_type.alloc();
		*addr = atp;
		atp->variant = str_copy(variant);
	} 

	/*
	 * Remember the test time.
	 */
	assert(when);
	atp->test_time = when;

	/*
	 * We need to make sure that the change summary reflects whether
	 * or not all tests have been run for this architecture.
	 */
	for (j = 0; ; ++j)
	{
		src_data = change_file_nth(cp, j);
		if (!src_data)
			break;
		switch (src_data->usage)
		{
		case file_usage_test:
		case file_usage_manual_test:
			if (src_data->action == file_action_remove)
				continue;
			if (src_data->deleted_by)
				continue;
			if (src_data->about_to_be_created_by)
				continue;
			break;

		default:
			continue;
		}

		atlp = src_data->architecture_times;
		if (!atlp)
		{
			/* All architectures missing.  */
			when = 0;
			break;
		}
		for (k = 0; k < atlp->length; ++k)
		{
			atp = atlp->list[k];
			if
			(
				/* bug if not set */
				atp->variant
			&&
				str_equal(atp->variant, variant)
			)
			{
				if (atp->test_time < when)
					when = atp->test_time;
				break;
			}
		}
		if (k >= atlp->length)
		{
			/* Found a missing architecture.  */
			when = 0;
			break;
		}
	}

	/*
	 * set the change test time
	 */
	change_test_time_set(cp, when);
	trace((/*{*/"}\n"));
}


time_t
change_file_test_time_get(cp, src_data)
	change_ty	*cp;
	fstate_src	src_data;
{
	fstate_src_architecture_times_list atlp;
	fstate_src_architecture_times atp;
	string_ty	*variant;
	size_t		j;

	atlp = src_data->architecture_times;
	if (!atlp)
		return 0;
	variant = change_architecture_name(cp, 1);
	for (j = 0; j < atlp->length; ++j)
	{
		atp = atlp->list[j];
		if
		(
			/* bug if not set */
			atp->variant
		&&
			str_equal(atp->variant, variant)
		)
			return atp->test_time;
	}
	return 0;
}


void
change_file_test_baseline_time_clear(cp, src_data)
	change_ty	*cp;
	fstate_src	src_data;
{
	fstate_src_architecture_times_list atlp;
	fstate_src_architecture_times atp;
	size_t		j;
	string_ty	*variant;

	/*
	 * We are clearing a test time stamp,
	 * so the change summary must also be cleared 
	 */
	change_test_baseline_time_set(cp, (time_t)0);

	/*
	 * find the appropriate architecture record
	 */
	atlp = src_data->architecture_times;
	if (!atlp)
		return;
	variant = change_architecture_name(cp, 1);
	for (j = 0; j < atlp->length; ++j)
	{
		atp = atlp->list[j];
		if (!atp->variant)
			continue; /* probably a bug */
		if (!str_equal(atp->variant, variant))
			continue;

		/*
		 * Clear the time stamp
		 */
		atp->test_baseline_time = 0;
		break;
	}
}


void
change_file_test_baseline_time_set(cp, src_data, when)
	change_ty	*cp;
	fstate_src	src_data;
	time_t		when;
{
	fstate_src_architecture_times_list atlp;
	fstate_src_architecture_times atp;
	string_ty	*variant;
	size_t		j, k;

	/*
	 * Find the appropriate architecture record;
	 * create a new one if necessary.
	 */
	variant = change_architecture_name(cp, 1);
	if (!src_data->architecture_times)
		src_data->architecture_times =
			fstate_src_architecture_times_list_type.alloc();
	atlp = src_data->architecture_times;
	for (j = 0; j < atlp->length; ++j)
	{
		atp = atlp->list[j];
		if
		(
			/* bug if not set */
			atp->variant
		&&
			str_equal(atp->variant, variant)
		)
			break;
	}
	if (j >= atlp->length)
	{
		fstate_src_architecture_times *addr;
		type_ty		*type_p;

		addr =
			fstate_src_architecture_times_list_type.list_parse
			(
				atlp,
				&type_p
			);
		assert(type_p == &fstate_src_architecture_times_type);
		atp = fstate_src_architecture_times_type.alloc();
		*addr = atp;
		atp->variant = str_copy(variant);
	} 

	/*
	 * Remember the test time.
	 */
	assert(when);
	atp->test_baseline_time = when;

	/*
	 * We need to make sure that the change summary reflects whether
	 * or not all tests have been run for this architecture.
	 */
	for (j = 0; ; ++j)
	{
		src_data = change_file_nth(cp, j);
		if (!src_data)
			break;
		switch (src_data->usage)
		{
		case file_usage_test:
		case file_usage_manual_test:
			if (src_data->action == file_action_remove)
				continue;
			if (src_data->action == file_action_modify)
				continue;
			if (src_data->deleted_by)
				continue;
			if (src_data->about_to_be_created_by)
				continue;
			break;

		default:
			continue;
		}

		atlp = src_data->architecture_times;
		if (!atlp)
		{
			/* All architectures missing.  */
			when = 0;
			break;
		}
		for (k = 0; k < atlp->length; ++k)
		{
			atp = atlp->list[k];
			if
			(
				/* bug if not set */
				atp->variant
			&&
				str_equal(atp->variant, variant)
			)
			{
				if (atp->test_baseline_time < when)
					when = atp->test_baseline_time;
				break;
			}
		}
		if (k >= atlp->length)
		{
			/* Found a missing architecture.  */
			when = 0;
			break;
		}
	}

	/*
	 * set the change test time
	 */
	change_test_baseline_time_set(cp, when);
}


time_t
change_file_test_baseline_time_get(cp, src_data)
	change_ty	*cp;
	fstate_src	src_data;
{
	fstate_src_architecture_times_list atlp;
	fstate_src_architecture_times atp;
	string_ty	*variant;
	size_t		j;

	atlp = src_data->architecture_times;
	if (!atlp)
		return 0;
	variant = change_architecture_name(cp, 1);
	for (j = 0; j < atlp->length; ++j)
	{
		atp = atlp->list[j];
		if
		(
			/* bug if not set */
			atp->variant
		&&
			str_equal(atp->variant, variant)
		)
			return atp->test_baseline_time;
	}
	return 0;
}


/*
 * NAME
 *	change_fingerprint_same
 *
 * SYNOPSIS
 *	int change_fingerprint_same(fingerprint fp, string_ty *path);
 *
 * DESCRIPTION
 *	The change_fingerprint_same function is used to test if a file
 *	fingerprint is the same.  This implies the file itself is the
 *	same.  If the file does not exist, it is aas if the fingerprint
 *	hash changed.
 *
 * RETURNS
 *	int;	1 -> the file is the SAME
 *		0 -> the file has changed
 */

int
change_fingerprint_same(fp, path, check_always)
	fingerprint	fp;
	string_ty	*path;
	int		check_always;
{
	time_t		oldest;
	time_t		newest;
	string_ty	*crypto;

	/*
	 * a NULL pointer means something very weird is going on
	 */
	trace(("change_fingerprint_same(fp = %8.8lX, path = \"%s\")\n{\n"/*}*/, (long)fp, path->str_text));
	if (!fp)
	{
		trace(("No existing fingerprint\n"));
		trace(("return 0;\n"));
		trace((/*{*/"}\n"));
		return 0;
	}
	assert(fp->youngest >= 0);
	assert(fp->oldest >= 0);

	/*
	 * If the file does not exist, clear the fingerprint and say
	 * that the file "is not the same".
	 */
	if (!os_exists(path))
	{
		fp->youngest = 0;
		fp->oldest = 0;
		if (fp->crypto)
		{
			str_free(fp->crypto);
			fp->crypto = 0;
		}
		trace(("no file there to fingerprint\n"));
		trace(("return 0;\n"));
		trace((/*{*/"}\n"));
		return 0;
	}

	/*
	 * The youngest field and the file's mtime should be the same;
	 * if it is, don't bother checking the fingerprint, just say the
	 * the file is the same.  (The os_mtime function checks the
	 * ctime, too, just in case the user is trying to fake us out.)
	 */
	os_mtime_range(path, &oldest, &newest);
	assert(oldest > 0);
	assert(newest > 0);
	if
	(
		!check_always
	&&
		fp->crypto
	&&
		fp->oldest
	&&
		fp->youngest
	&&
		fp->youngest == newest
	)
	{
		trace(("file times match\n"));
		if (oldest < fp->oldest)
			fp->oldest = oldest;
		trace(("return 1;\n"));
		trace((/*{*/"}\n"));
		return 1;
	}

	/*
	 * Read the fingerprint.  If it is the same as before,
	 * extend the valid time range, and say the file is the same.
	 */
	crypto = os_fingerprint(path);
	if
	(
		fp->crypto
	&&
		fp->oldest
	&&
		fp->youngest
	&&
		str_equal(crypto, fp->crypto)
	)
	{
		trace(("file fingerprints match\n"));
		str_free(crypto);
		if (newest > fp->youngest)
			fp->youngest = newest;
		if (oldest < fp->oldest)
			fp->oldest = oldest;
		trace(("return 1;\n"));
		trace((/*{*/"}\n"));
		return 1;
	}

	/*
	 * Everything has changed, reset everything and then say the
	 * file is not the same.
	 */
	fp->oldest = oldest;
	fp->youngest = newest;
	if (fp->crypto)
		str_free(fp->crypto);
	fp->crypto = crypto;
	trace(("file fingerprint mis-match\n"));
	trace(("return 0;\n"));
	trace((/*{*/"}\n"));
	return 0;
}


void
change_file_fingerprint_check(cp, src_data)
	change_ty	*cp;
	fstate_src	src_data;
{
	string_ty       *path;
	int		same;
	cstate		cstate_data;

	/*
	 * only useful in the 'being developed' and 'being integrated'
	 * states
	 */
	path = change_file_path(cp, src_data->file_name);
	assert(path);
	if (!src_data->file_fp)
		src_data->file_fp = fingerprint_type.alloc();
	assert(src_data->file_fp->youngest >= 0);
	assert(src_data->file_fp->oldest >= 0);
	change_become(cp);
	same = change_fingerprint_same(src_data->file_fp, path, 0);
	change_become_undo();
	assert(src_data->file_fp->youngest > 0);
	assert(src_data->file_fp->oldest > 0);

	/*
	 * if the file is unchanged, do nothing more
	 */
	if (same)
		return;

	/*
	 * nuke the difference time
	 */
	cstate_data = change_cstate_get(cp);
	if (cstate_data->state != cstate_state_being_integrated)
	{
		if (src_data->diff_file_fp)
		{
			fingerprint_type.free(src_data->diff_file_fp);
			src_data->diff_file_fp = 0;
		}
	}
	else
	{
#if 0
		if (src_data->idiff_file_fp)
		{
			fingerprint_type.free(src_data->idiff_file_fp);
			src_data->idiff_file_fp = 0;
		}
#endif
	}

	/*
	 * nuke the file's test times
	 */
	if (src_data->architecture_times)
	{
		fstate_src_architecture_times_list_type.free
		(
			src_data->architecture_times
		);
		src_data->architecture_times = 0;
	}
}


int
change_file_up_to_date(pp, c_src_data)
	project_ty	*pp;
	fstate_src	c_src_data;
{
	fstate_src	p_src_data;
	int		result;

	/*
	 * No edit number at all implies a new file, and is always
	 * up-to-date.
	 */
	trace(("change_file_up_to_date(pp = %08lX)\n{\n"/*}*/, (long)pp));
	trace(("filename = \"%s\";\n", c_src_data->file_name->str_text));
	if (c_src_data->edit_number && !c_src_data->edit_number_origin)
	{
		/* Historical 2.3 -> 3.0 transition. */
		c_src_data->edit_number_origin =
			str_copy(c_src_data->edit_number);
	}
	if (!c_src_data->edit_number_origin)
	{
		trace(("return 1;\n"));
		trace((/*{*/"}\n"));
		return 1;
	}

	/*
	 * Look for the file in the project.  If it is not there, it
	 * implies a new file, which is always up-to-date.
	 */
	p_src_data = project_file_find(pp, c_src_data->file_name);
	if
	(
		!p_src_data
	||
		p_src_data->deleted_by
	||
		p_src_data->about_to_be_created_by
	||
		p_src_data->about_to_be_copied_by
	||
		!p_src_data->edit_number
	)
	{
		trace(("return 1;\n"));
		trace((/*{*/"}\n"));
		return 1;
	}

	/*
	 * The file is out-of-date if the edit number of the file in the
	 * project is not the same as the edit number of the file when
	 * originally copied from the project.
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
	 *	Updates branch edit_number_origin on integrate pass.
	 */
	result =
		str_equal
		(
			p_src_data->edit_number,
			c_src_data->edit_number_origin
		);
	trace(("return %d;\n", result));
	trace((/*{*/"}\n"));
	return result;
}


static void metric_check _((metric, string_ty *, change_ty *));

static void
metric_check(mp, fn, cp)
	metric		mp;
	string_ty	*fn;
	change_ty	*cp;
{
	sub_context_ty	*scp;

	if (!mp->name)
	{
		scp = sub_context_new();
		sub_var_set(scp, "File_Name", "%S", fn);
		sub_var_set(scp, "FieLD_Name", "name");
		change_fatal
		(
			cp,
			scp,
			i18n("$filename: corrupted \"$field_name\" field")
		);
	}
	if (!(mp->mask & metric_value_mask))
	{
		scp = sub_context_new();
		sub_var_set(scp, "File_Name", "%S", fn);
		sub_var_set(scp, "FieLD_Name", "value");
		change_fatal
		(
			cp,
			scp,
			i18n("$filename: corrupted \"$field_name\" field")
		);
	}
}


static void metric_list_check _((metric_list, string_ty *, change_ty *));

static void
metric_list_check(mlp, fn, cp)
	metric_list	mlp;
	string_ty	*fn;
	change_ty	*cp;
{
	size_t		j;

	for (j = 0; j < mlp->length; ++j)
		metric_check(mlp->list[j], fn, cp);
}


static string_ty * change_file_metrics_filename _((change_ty *, string_ty *));

static string_ty *
change_file_metrics_filename(cp, filename)
	change_ty	*cp;
	string_ty	*filename;
{
	string_ty	*metrics_filename_pattern;
	sub_context_ty	*scp;
	string_ty	*absolute_filename;
	string_ty	*metrics_filename;

	metrics_filename_pattern = change_metrics_filename_pattern_get(cp);
	scp = sub_context_new();
	absolute_filename = change_file_path(cp, filename);
	sub_var_set(scp, "File_Name", "%S", absolute_filename);
	str_free(absolute_filename);
	metrics_filename = subst_intl(scp, metrics_filename_pattern->str_text);
	sub_context_delete(scp);
	return metrics_filename;
}


metric_list
change_file_metrics_get(cp, filename)
	change_ty	*cp;
	string_ty	*filename;
{
	string_ty	*metrics_filename;
	metrics		mp;
	metric_list	mlp;

	/*
	 * Get the name of the file to read.
	 */
	metrics_filename = change_file_metrics_filename(cp, filename);

	/*
	 * Read the metrics file if it is there.
	 */
	change_become(cp);
	if (os_exists(metrics_filename))
	{
		mp = metrics_read_file(metrics_filename->str_text);
	}
	else
		mp = 0;
	change_become_undo();

	/*
	 * Extract the metrics list from the file data,
	 * if present.
	 */
	mlp = 0;
	if (mp && mp->metrics && mp->metrics->length)
	{
		mlp = mp->metrics;
		mp->metrics = 0;
	}
	metrics_type.free(mp);

	/*
	 * If we have a metrics list, check each of the list elements.
	 */
	if (mlp)
		metric_list_check(mlp, metrics_filename, cp);
	str_free(metrics_filename);

	/*
	 * Return the list of metrics.
	 */
	return mlp;
}


void
change_file_list_metrics_check(cp)
	change_ty	*cp;
{
	size_t		j;
	fstate_src	src_data;

	for (j = 0; ; ++j)
	{
		metric_list	mlp;

		src_data = change_file_nth(cp, j);
		if (!src_data)
			break;

		/*
		 * Only verify the metrics for primary source files,
		 * and only for creates and modifies.
		 */
		if
		(
			src_data->usage == file_usage_build
		||
			(
				src_data->action != file_action_create
			&&
				src_data->action != file_action_modify
			)
		)
			continue;

		/*
		 * Read the file.
		 * The contents will be checked.
		 */
		mlp = change_file_metrics_get(cp, src_data->file_name);

		/*
		 * Throw the data away,
		 * we only wanted the checking side-effect.
		 */
		metric_list_type.free(mlp);
	}
}

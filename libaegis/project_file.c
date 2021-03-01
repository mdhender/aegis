/*
 *	aegis - project change supervisor
 *	Copyright (C) 1995, 1996, 1997, 1998, 1999 Peter Miller;
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
 * MANIFEST: functions to manipulate project_files
 */

#include <change_file.h>
#include <error.h>
#include <fstrcmp.h>
#include <mem.h>
#include <project_file.h>
#include <trace.h>
#include <str_list.h>


static string_list_ty *file_list_get _((project_ty *));

static string_list_ty *
file_list_get(pp)
	project_ty	*pp;
{
	trace(("file_list_get(pp = %8.8lX)\n{\n"/*}*/, (long)pp));
	if (!pp->file_list)
	{
		project_ty	*ppp;
		change_ty	*cp;
		long		j;
		fstate_src	fsp;
		string_list_ty		*wlp;

		wlp = mem_alloc(sizeof(string_list_ty));
		string_list_constructor(wlp);
		for (ppp = pp; ppp; ppp = ppp->parent)
		{
			cp = project_change_get(ppp);
			for (j = 0; ; ++j)
			{
				fsp = change_file_nth(cp, j);
				if (!fsp)
					break;
				string_list_append_unique(wlp, fsp->file_name);
			}
		}

		/*
		 * Ensure that the file name list is in lexicographical
		 * order, otherwise the users can see the joins (and its
		 * harder to find the files in a listing).
		 * (C locale)
		 */
		string_list_sort(wlp);
		pp->file_list = wlp;
	}
	trace(("return %8.8lX;\n", (long)pp->file_list));
	trace((/*{*/"}\n"));
	return pp->file_list;
}


fstate_src
project_file_find(pp, file_name)
	project_ty	*pp;
	string_ty	*file_name;
{
	project_ty	*ppp;
	change_ty	*cp;
	fstate_src	src_data;

	trace(("project_file_find(pp = %8.8lX, file_name = \"%s\")\n{\n"/*}*/,
		(long)pp, file_name->str_text));
	for (ppp = pp; ppp; ppp = ppp->parent)
	{
		cp = project_change_get(ppp);
		src_data = change_file_find(cp, file_name);
		if (src_data)
		{
			trace(("return %8.8lX;\n", (long)src_data));
			trace((/*{*/"}\n"));
			return src_data;
		}
	}
	trace(("return NULL;\n"));
	trace((/*{*/"}\n"));
	return 0;
}


fstate_src
project_file_find_fuzzy(pp, file_name)
	project_ty	*pp;
	string_ty	*file_name;
{
	fstate_src	src_data;
	string_list_ty	*wlp;
	size_t		j;
	fstate_src	best_src;
	double		best_weight;

	/*
	 * This is used to find names when project_file_find does not.
	 * Deleted and almost created files are thus ignored.
	 */
	trace(("project_file_find_fuzzy(pp = %8.8lX, fn = \"%s\")\n{\n"/*}*/,
		(long)pp, file_name->str_text));

	/*
	 * get the merged list of file names
	 */
	wlp = file_list_get(pp);

	/*
	 * find the closest name
	 * that actually exists
	 */
	best_src = 0;
	best_weight = 0.6;
	for (j = 0; j < wlp->nstrings; ++j)
	{
		string_ty	*name;
		double		weight;

		name = wlp->string[j];
		weight = fstrcmp(name->str_text, file_name->str_text);
		if (weight > best_weight)
		{
			src_data = project_file_find(pp, name);
			if
			(
				src_data
			&&
				!src_data->deleted_by
			&&
				!src_data->about_to_be_created_by
			)
			{
				best_src = src_data;
				best_weight = weight;
			}
		}
	}

	trace(("return %8.8lX;\n", (long)best_src));
	trace((/*{*/"}\n"));
	return best_src;
}


string_ty *
project_file_path(pp, file_name)
	project_ty	*pp;
	string_ty	*file_name;
{
	project_ty	*ppp;

	trace(("project_file_path(pp = %8.8lX, file_name = \"%s\")\n{\n"/*}*/,
		(long)pp, file_name->str_text));
	for (ppp = pp; ppp; ppp = ppp->parent)
	{
		change_ty	*cp;
		fstate_src	src_data;
		string_ty	*result;

		cp = project_change_get(ppp);
		src_data = change_file_find(cp, file_name);
		if (!src_data)
			continue;
		if (src_data->about_to_be_copied_by)
			continue;
		result = change_file_path(cp, file_name);
		assert(result);
		assert(result->str_text[0] == '/');
		trace(("return \"%s\";\n", result->str_text));
		trace((/*{*/"}\n"));
		return result;
	}
	this_is_a_bug();
	trace(("return NULL;\n"));
	trace((/*{*/"}\n"));
	return 0;
}


void
project_file_dir(pp, file_name, result_in, result_out)
	project_ty	*pp;
	string_ty	*file_name;
	string_list_ty		*result_in;
	string_list_ty		*result_out;
{
	project_ty	*ppp;
	change_ty	*cp;
	string_list_ty		wl_in;
	string_list_ty		wl_out;
	string_list_ty		exclude;

	trace(("project_file_dir(pp = %8.8lX, file_name = \"%s\")\n{\n"/*}*/,
		(long)pp, file_name->str_text));
	assert(result_in);
	string_list_constructor(result_in);
	string_list_constructor(&exclude);
	if (result_out)
		string_list_constructor(result_out);
	else
		result_out = &exclude;
	for (ppp = pp; ppp; ppp = ppp->parent)
	{
		cp = project_change_get(ppp);
		change_file_dir(cp, file_name, &wl_in, &wl_out);
		string_list_remove_list(&wl_in, result_out);
		string_list_append_list_unique(result_in, &wl_in);
		string_list_append_list_unique(result_out, &wl_out);
		string_list_destructor(&wl_in);
		string_list_destructor(&wl_out);
	}
	string_list_destructor(&exclude);
	trace((/*{*/"}\n"));
}


fstate_src
project_file_new(pp, file_name)
	project_ty	*pp;
	string_ty	*file_name;
{
	change_ty	*cp;
	fstate_src	src_data;

	trace(("project_file_new(pp = %8.8lX, file_name = \"%s\")\n{\n"/*}*/,
		(long)pp, file_name->str_text));
	cp = project_change_get(pp);
	src_data = change_file_new(cp, file_name);
	trace(("return %8.8lX;\n", (long)src_data));
	trace((/*{*/"}\n"));
	return src_data;
}


void
project_file_remove(pp, file_name)
	project_ty	*pp;
	string_ty	*file_name;
{
	change_ty	*cp;

	trace(("project_file_remove(pp = %8.8lX, file_name = \"%s\")\n{\n"/*}*/,
		(long)pp, file_name->str_text));
	cp = project_change_get(pp);
	change_file_remove(cp, file_name);
	trace((/*{*/"}\n"));
}


fstate_src
project_file_nth(pp, n)
	project_ty	*pp;
	size_t		n;
{
	string_list_ty		*wlp;
	fstate_src	src_data;

	trace(("project_file_nth(pp = %8.8lX, n = %ld)\n{\n"/*}*/,
		(long)pp, (long)n));
	wlp = file_list_get(pp);
	if (n < wlp->nstrings)
	{
		src_data = project_file_find(pp, wlp->string[n]);
		assert(src_data);
	}
	else
		src_data = 0;
	trace(("return %8.8lX;\n", (long)src_data));
	trace((/*{*/"}\n"));
	return src_data;
}


void
project_search_path_get(pp, wlp, resolve)
	project_ty	*pp;
	string_list_ty		*wlp;
	int		resolve;
{
	project_ty	*ppp;

	/*
	 * do NOT call change_search_path, it will make a mess
	 */
	for (ppp = pp; ppp; ppp = ppp->parent)
		string_list_append(wlp, project_baseline_path_get(ppp, resolve));
}


void
project_file_shallow(pp, file_name, cn)
	project_ty	*pp;
	string_ty	*file_name;
	long		cn;
{
	change_ty	*pcp;
	fstate_src	src1_data;
	fstate_src	src2_data;

	/*
	 * Will never be zero, because already magic zero encoded.
	 */
	assert(cn);

	/*
	 * Look for the file in the project.
	 * If it is there, nothing more needs to be done.
	 */
	if (!pp->parent)
		return;
	pcp = project_change_get(pp);
	src1_data = change_file_find(pcp, file_name);
	if (src1_data)
		return;

	/*
	 * The file is not part of the immediate project, see if
	 * we can find it in one of the grand*parent projects.
	 * If it is not there, nothing more needs to be done.
	 */
	src2_data = project_file_find(pp, file_name);
	if (!src2_data)
		return;

	/*
	 * Create a new file in the project, and mark it ``about to be
	 * copied''.  That way we can throw it away again, if the
	 * review fails or the integration fails.
	 */
	src1_data = change_file_new(pcp, file_name);
	src1_data->action = src2_data->action;
	src1_data->usage = src2_data->usage;
	src1_data->about_to_be_copied_by = cn;

	/*
	 * The value here is bogus (can't use the old one, it refers to
	 * the wrong branch).  The values only lasts long enough to be
	 * replaced (ipass) or removed (ifail).
	 */
	if (src2_data->deleted_by)
		src1_data->deleted_by = cn;

	/*
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
	 * branch's edit_number
	 *	The head revision of the branch.
	 * branch's edit_number_origin
	 *	The version originally copied.
	 */
	assert(src2_data->edit_number);
	if (src2_data->edit_number)
		src1_data->edit_number = str_copy(src2_data->edit_number);
	else
		src1_data->edit_number = str_from_c("bogus");
	src1_data->edit_number_origin =
		str_copy(src1_data->edit_number);

	/*
	 * pull the testing correlations across
	 */
	if (src2_data->test && src2_data->test->length)
	{
		size_t	j;

		src1_data->test = fstate_src_test_list_type.alloc();
		for (j = 0; j < src2_data->test->length; ++j)
		{
			string_ty	**addr_p;
			type_ty		*type_p;

			addr_p =
			    fstate_src_test_list_type.list_parse
				(
					src1_data->test,
					&type_p
				);
			assert(type_p = &string_type);
			*addr_p = str_copy(src2_data->test->list[j]);
		}
	}

	/*
	 * Note: locked_by is deliberatey dropped,
	 * because it refers to a different branch.
	 */
}


int
project_file_shallow_check(pp, file_name)
	project_ty	*pp;
	string_ty	*file_name;
{
	change_ty	*pcp;
	fstate_src	src_data;

	if (!pp->parent)
		/* accelerator */
		return 1;
	pcp = project_change_get(pp);
	src_data = change_file_find(pcp, file_name);
	return (src_data != 0);
}

/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999 Peter Miller;
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
 * MANIFEST: functions to manipulate writes
 */

#include <ac/stdlib.h>

#include <change.h>
#include <commit.h>
#include <error.h> /* for assert */
#include <os.h>
#include <project.h>
#include <trace.h>
#include <undo.h>


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
	int		compress;

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
	compress = project_compress_database_get(cp->pp);

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
			fstate_write_file(filename_new->str_text, cp->fstate_data, compress);
			commit_rename(filename_new, fn);
			cp->fstate_is_a_new_file = 0;
		}
		else
		{
			undo_unlink_errok(filename_new);
			fstate_write_file(filename_new->str_text, cp->fstate_data, compress);
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
		cstate_write_file(filename_new->str_text, cp->cstate_data, compress);
		commit_rename(filename_new, fn);
		cp->cstate_is_a_new_file = 0;
	}
	else
	{
		undo_unlink_errok(filename_new);
		cstate_write_file(filename_new->str_text, cp->cstate_data, compress);
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

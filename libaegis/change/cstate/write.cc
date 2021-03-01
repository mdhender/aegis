//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2001-2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <common/ac/stdlib.h>
#include <common/ac/string.h>

#include <libaegis/change.h>
#include <libaegis/commit.h>
#include <common/error.h> // for assert
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <common/trace.h>
#include <libaegis/undo.h>


static int
src_cmp(const void *s1p, const void *s2p)
{
    fstate_src_ty   *s1;
    fstate_src_ty   *s2;

    s1 = *(fstate_src_ty **)s1p;
    s2 = *(fstate_src_ty **)s2p;
    return strcmp(s1->file_name->str_text, s2->file_name->str_text);
}


static int
long_cmp(const void *s1p, const void *s2p)
{
    long	    n1;
    long	    n2;

    n1 = *(long *)s1p;
    n2 = *(long *)s2p;
    if (n1 < n2)
	return -1;
    if (n1 > n2)
	return 1;
    return 0;
}


static int
string_cmp(const void *v1, const void *v2)
{
    string_ty	    *s1;
    string_ty	    *s2;

    s1 = *(string_ty **)v1;
    s2 = *(string_ty **)v2;
    return strcmp(s1->str_text, s2->str_text);
}


void
change_cstate_write(change::pointer cp)
{
    string_ty	    *filename_new;
    string_ty	    *filename_old;
    static int	    temp_file_counter;
    string_ty	    *fn;
    int		    mode;
    int		    compress;

    trace(("change_cstate_write(cp = %08lX)\n{\n", (long)cp));
    assert(cp->reference_count >= 1);
    assert(cp->pp);
    assert(cp->cstate_data);
    if (!cp->cstate_data->brief_description)
	cp->cstate_data->brief_description = str_from_c("");
    if (!cp->cstate_data->description)
	cp->cstate_data->description =
	    str_copy(cp->cstate_data->brief_description);

    //
    // force various project related files to be read in,
    // if they are not already
    //
    mode = 0644 & ~change_umask(cp);
    compress = project_compress_database_get(cp->pp);

    //
    // write out the fstate file
    //
    assert(!cp->cstate_data->src);
    if (cp->fstate_data)
    {
	fstate_src_list_ty *slp;

	//
	// sort the files by name
	//
	if (!cp->fstate_data->src)
        {
	    cp->fstate_data->src =
                (fstate_src_list_ty *)fstate_src_list_type.alloc();
        }
	slp = cp->fstate_data->src;
	assert(slp);
	assert(slp->length <= slp->maximum);
	assert(!slp->list == !slp->maximum);
	if (slp->length >= 2)
	{
	    qsort(slp->list, slp->length, sizeof(*slp->list), src_cmp);
	}

	fn = change_fstate_filename_get(cp);
	filename_new = str_format("%s,%d", fn->str_text, ++temp_file_counter);
	filename_old = str_format("%s,%d", fn->str_text, ++temp_file_counter);
	change_become(cp);
	if (cp->fstate_is_a_new_file)
	{
	    string_ty	    *s1;
	    string_ty	    *s2;

	    s1 = project_Home_path_get(cp->pp);
	    s2 = os_below_dir(s1, fn);
	    os_mkdir_between(s1, s2, 02755);
	    str_free(s2);
	    undo_unlink_errok(filename_new);
	    fstate_write_file(filename_new, cp->fstate_data, compress);
	    commit_rename(filename_new, fn);
	    cp->fstate_is_a_new_file = 0;
	}
	else
	{
	    undo_unlink_errok(filename_new);
	    fstate_write_file(filename_new, cp->fstate_data, compress);
	    commit_rename(fn, filename_old);
	    commit_rename(filename_new, fn);
	    commit_unlink_errok(filename_old);
	}

	//
	// Change the file mode as appropriate.
	// (Only need to do this for new files, but be paranoid.)
	//
	os_chmod(filename_new, mode);
	change_become_undo(cp);
	str_free(filename_new);
	str_free(filename_old);
    }

    //
    // force the change list to be sorted
    //
    assert(cp->cstate_data);
    if (cp->cstate_data->branch && cp->cstate_data->branch->change)
    {
	cstate_branch_change_list_ty *lp;

	lp = cp->cstate_data->branch->change;
	qsort(lp->list, lp->length, sizeof(lp->list[0]), long_cmp);
    }

    //
    // Force the staff lists to be sorted.
    // (It helps makes the tests pass, if nothing else.)
    //
    if (cp->cstate_data->branch && cp->cstate_data->branch->administrator)
    {
	cstate_branch_administrator_list_ty *lp;

	lp = cp->cstate_data->branch->administrator;
	qsort(lp->list, lp->length, sizeof(lp->list[0]), string_cmp);
    }
    if (cp->cstate_data->branch && cp->cstate_data->branch->developer)
    {
	cstate_branch_developer_list_ty *lp;

	lp = cp->cstate_data->branch->developer;
	qsort(lp->list, lp->length, sizeof(lp->list[0]), string_cmp);
    }
    if (cp->cstate_data->branch && cp->cstate_data->branch->reviewer)
    {
	cstate_branch_reviewer_list_ty *lp;

	lp = cp->cstate_data->branch->reviewer;
	qsort(lp->list, lp->length, sizeof(lp->list[0]), string_cmp);
    }
    if (cp->cstate_data->branch && cp->cstate_data->branch->integrator)
    {
	cstate_branch_integrator_list_ty *lp;

	lp = cp->cstate_data->branch->integrator;
	qsort(lp->list, lp->length, sizeof(lp->list[0]), string_cmp);
    }

    //
    // write out the cstate file
    //
    fn = change_cstate_filename_get(cp);
    assert(!cp->cstate_data->src);
    filename_new = str_format("%s,%d", fn->str_text, ++temp_file_counter);
    filename_old = str_format("%s,%d", fn->str_text, ++temp_file_counter);
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
	cstate_write_file(filename_new, cp->cstate_data, compress);
	commit_rename(filename_new, fn);
	cp->cstate_is_a_new_file = 0;
    }
    else
    {
	undo_unlink_errok(filename_new);
	cstate_write_file(filename_new, cp->cstate_data, compress);
	commit_rename(fn, filename_old);
	commit_rename(filename_new, fn);
	commit_unlink_errok(filename_old);
    }

    //
    // Change the file mode as appropriate.
    // (Only need to do this for new files, but be paranoid.)
    //
    os_chmod(filename_new, mode);
    change_become_undo(cp);
    str_free(filename_new);
    str_free(filename_old);
    trace(("}\n"));
}

//
//      aegis - project change supervisor
//      Copyright (C) 1999, 2001-2008, 2012 Peter Miller
//      Copyright (C) 2008 Walter Franzini
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>

#include <common/trace.h>
#include <libaegis/change.h>
#include <libaegis/commit.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
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
    long            n1;
    long            n2;

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
    string_ty       *s1;
    string_ty       *s2;

    s1 = *(string_ty **)v1;
    s2 = *(string_ty **)v2;
    return strcmp(s1->str_text, s2->str_text);
}


void
change::cstate_write()
{
    string_ty       *filename_new;
    string_ty       *filename_old;
    static int      temp_file_counter;
    string_ty       *fn;
    int             mode;
    int             compress;

    trace(("change_cstate_write(cp = %p)\n{\n", this));
    assert(reference_count >= 1);
    assert(pp);
    assert(cstate_data);
    if (!cstate_data->brief_description)
        cstate_data->brief_description = str_from_c("");
    if (!cstate_data->description)
        cstate_data->description =
            str_copy(cstate_data->brief_description);

    //
    // force various project related files to be read in,
    // if they are not already
    //
    mode = 0644 & ~umask_get();
    compress = project_compress_database_get(pp);

    //
    // write out the fstate file
    //
    assert(!cstate_data->src);
    if (fstate_data)
    {
        fstate_src_list_ty *slp;

        //
        // sort the files by name
        //
        if (!fstate_data->src)
        {
            fstate_data->src =
                (fstate_src_list_ty *)fstate_src_list_type.alloc();
        }
        slp = fstate_data->src;
        assert(slp);
        assert(slp->length <= slp->maximum);
        assert(!slp->list == !slp->maximum);
        if (slp->length >= 2)
        {
            qsort(slp->list, slp->length, sizeof(*slp->list), src_cmp);
        }

        fn = change_fstate_filename_get(this);
        filename_new = str_format("%s,%d", fn->str_text, ++temp_file_counter);
        filename_old = str_format("%s,%d", fn->str_text, ++temp_file_counter);
        change_become(this);
        if (fstate_is_a_new_file)
        {
            string_ty       *s1;
            string_ty       *s2;

            s1 = project_Home_path_get(pp);
            s2 = os_below_dir(s1, fn);
            os_mkdir_between(s1, s2, 02755);
            str_free(s2);
            undo_unlink_errok(filename_new);
            fstate_write_file(filename_new, fstate_data, compress);
            commit_rename(filename_new, fn);
            fstate_is_a_new_file = 0;
        }
        else
        {
            undo_unlink_errok(filename_new);
            fstate_write_file(filename_new, fstate_data, compress);
            commit_rename(fn, filename_old);
            commit_rename(filename_new, fn);
            commit_unlink_errok(filename_old);
        }

        //
        // Change the file mode as appropriate.
        // (Only need to do this for new files, but be paranoid.)
        //
        os_chmod(filename_new, mode);
        change_become_undo(this);
        str_free(filename_new);
        str_free(filename_old);
    }

    //
    // force the change list to be sorted
    //
    assert(cstate_data);
    if (cstate_data->branch && cstate_data->branch->change)
    {
        cstate_branch_change_list_ty *lp;

        lp = cstate_data->branch->change;
        qsort(lp->list, lp->length, sizeof(lp->list[0]), long_cmp);
    }

    //
    // Force the staff lists to be sorted.
    // (It helps makes the tests pass, if nothing else.)
    //
    if (cstate_data->branch && cstate_data->branch->administrator)
    {
        cstate_branch_administrator_list_ty *lp;

        lp = cstate_data->branch->administrator;
        qsort(lp->list, lp->length, sizeof(lp->list[0]), string_cmp);
    }
    if (cstate_data->branch && cstate_data->branch->developer)
    {
        cstate_branch_developer_list_ty *lp;

        lp = cstate_data->branch->developer;
        qsort(lp->list, lp->length, sizeof(lp->list[0]), string_cmp);
    }
    if (cstate_data->branch && cstate_data->branch->reviewer)
    {
        cstate_branch_reviewer_list_ty *lp;

        lp = cstate_data->branch->reviewer;
        qsort(lp->list, lp->length, sizeof(lp->list[0]), string_cmp);
    }
    if (cstate_data->branch && cstate_data->branch->integrator)
    {
        cstate_branch_integrator_list_ty *lp;

        lp = cstate_data->branch->integrator;
        qsort(lp->list, lp->length, sizeof(lp->list[0]), string_cmp);
    }

    //
    // write out the cstate file
    //
    fn = cstate_filename_get();
    assert(!cstate_data->src);
    filename_new = str_format("%s,%d", fn->str_text, ++temp_file_counter);
    filename_old = str_format("%s,%d", fn->str_text, ++temp_file_counter);
    change_become(this);
    if (cstate_is_a_new_file)
    {
        string_ty       *s1;
        string_ty       *s2;

        s1 = project_Home_path_get(pp);
        s2 = os_below_dir(s1, fn);
        os_mkdir_between(s1, s2, 02755);
        str_free(s2);
        undo_unlink_errok(filename_new);
        cstate_write_file(filename_new, cstate_data, compress);
        commit_rename(filename_new, fn);
        cstate_is_a_new_file = 0;
    }
    else
    {
        undo_unlink_errok(filename_new);
        cstate_write_file(filename_new, cstate_data, compress);
        commit_rename(fn, filename_old);
        commit_rename(filename_new, fn);
        commit_unlink_errok(filename_old);
    }

    //
    // Change the file mode as appropriate.
    // (Only need to do this for new files, but be paranoid.)
    //
    os_chmod(filename_new, mode);
    change_become_undo(this);
    str_free(filename_new);
    str_free(filename_old);
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :

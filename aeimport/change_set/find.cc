//
//      aegis - project change supervisor
//      Copyright (C) 2001, 2002, 2004-2008 Peter Miller
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

#include <aeimport/change_set/find.h>
#include <aeimport/change_set/list.h>
#include <common/error.h>
#include <aeimport/format/search_list.h>
#include <aeimport/format/version_list.h>
#include <common/symtab.h>
#include <common/trace.h>


static symtab_ty *stp;


static void
reaper(void *p)
{
    format_version_list_ty *fvlp;

    fvlp = (format_version_list_ty *)p;
    format_version_list_delete(fvlp, 0);
}


static format_version_list_ty *
find_user(string_ty *name)
{
    format_version_list_ty *fvlp;

    if (!stp)
    {
        stp = new symtab_ty(5);
        stp->set_reap(reaper);
    }
    fvlp = (format_version_list_ty *)stp->query(name);
    if (!fvlp)
    {
        fvlp = format_version_list_new();
        stp->assign(name, fvlp);
    }
    return fvlp;
}


static void
walker(const symtab_ty *, const nstring &, void *data, void *aux)
{
    format_version_list_ty *fvlp =  (format_version_list_ty *)data;
    change_set_list_ty *cslp =      (change_set_list_ty *)aux;
    long            window;
    size_t          j;

    //
    // Sort each version list by date.
    //
    trace(("walker()\n{\n"));
    format_version_list_sort_by_date(fvlp);

    //
    // Now walk down the list, looking for clumps.
    //
    window = 300;
    for (j = 0; j < fvlp->length;)
    {
        size_t          k;
        change_set_ty   *csp;

        //
        // Find the end of this clump.
        //
        trace(("j = %ld\n", (long)j));
        for
        (
            k = j + 1;
            (
                k < fvlp->length
            &&
                (fvlp->item[k]->when < fvlp->item[k - 1]->when + window)
            );
            ++k
        )
            ;

        //
        // Turn the clump into a change set.
        //
        string_list_ty sl;
        csp = new change_set_ty();
        csp->who = str_copy(fvlp->item[j]->who);
        csp->when = fvlp->item[k - 1]->when;
        trace(("when = %ld\n", (long)csp->when));
        for (; j < k; ++j)
        {
            format_version_ty *fvp;
            change_set_file_action_ty action;
            string_ty       *edit;

            fvp = fvlp->item[j];
            if (fvp->description && fvp->description->str_length)
                sl.push_back_unique(fvp->description);

            //
            // Figure out what the file action is.
            //
            edit = fvp->edit;
            if (fvp->dead)
            {
                action = change_set_file_action_remove;
                if (fvp->before && fvp->before->edit)
                    edit = fvp->before->edit;
            }
            else
            {
                if (!fvp->before || fvp->before->dead)
                    action = change_set_file_action_create;
                else
                    action = change_set_file_action_modify;
            }

            change_set_file_list_append
            (
                &csp->file,
                fvp->filename_logical,
                edit,
                action,
                &fvp->tag
            );
        }
        csp->description = sl.unsplit("\n");

        //
        // Append the change set to the list.
        //
        change_set_list_append(cslp, csp);
    }
    trace(("}\n"));
}


change_set_list_ty *
change_set_find(format_search_list_ty *fslp)
{
    size_t          j;
    change_set_list_ty *result;

    //
    // Sort the file versions into separate buckets for each user.
    //
    // For now, only traverse the trunk.  I still doen't know how
    // to associate across files to divine where the branbches are.
    //
    trace(("change_set_find()\n{\n"));
    for (j = 0; j < fslp->length; ++j)
    {
        format_search_ty *fsp;
        format_version_ty *fvp;

        fsp = fslp->item[j];
        fvp = fsp->root;
        while (fvp)
        {
            format_version_list_ty *fvlp;

            fvlp = find_user(fvp->who);
            format_version_list_append(fvlp, fvp);
            fvp = fvp->after;
        }
    }

    //
    // Process each bucket.
    //
    trace(("mark\n"));
    result = change_set_list_new();
    stp->walk(walker, result);
    change_set_list_sort_by_date(result);
    delete stp;
    stp = 0;

    //
    // Work out where to attach the tags.
    //
    // We do this by working BACKWARDS in time, assigning file tags
    // to change sets.  But (and this is why we work backwards in
    // time) we don't assign tags which have already been seen.
    //
    // In common usage, the tags serve a similar purpose as Aegis'
    // delta numbers.  They are all (typically) applied in a single
    // CVS command, in order that a particular build may be recreated
    // later.  HOWEVER, because each file will be at a different
    // version, and each will have had its latest version included
    // in various random change sets.
    //
    // Tags are used for other things too.  This is simply a guess,
    // but it's a fairly reasonable one.
    //
    string_list_ty occupied;
    for (j = result->length; j > 0; --j)
    {
        change_set_ty *csp = result->item[j - 1];
        for (size_t k = 0; k < csp->file.size(); ++k)
        {
            size_t          m;

            change_set_file_ty *csfp = csp->file[k];
            for (m = 0; m < csfp->tag.nstrings; ++m)
            {
                string_ty       *tag;

                tag = csfp->tag.string[m];
                if (occupied.member(tag))
                    continue;
                csp->tag.push_back_unique(tag);
                occupied.push_back(tag);
            }
        }
    }
    trace(("}\n"));
    return result;
}

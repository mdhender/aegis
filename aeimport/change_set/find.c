/*
 *      aegis - project change supervisor
 *      Copyright (C) 2001, 2002 Peter Miller;
 *      All rights reserved.
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate finds
 */

#include <change_set/find.h>
#include <change_set/list.h>
#include <error.h>
#include <format/search_list.h>
#include <format/version_list.h>
#include <symtab.h>
#include <trace.h>


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
        stp = symtab_alloc(5);
        stp->reap = reaper;
    }
    fvlp = (format_version_list_ty *)symtab_query(stp, name);
    if (!fvlp)
    {
        fvlp = format_version_list_new();
        symtab_assign(stp, name, fvlp);
    }
    return fvlp;
}


static void
walker(symtab_ty *stpx, string_ty *key, void *data, void *aux)
{
    format_version_list_ty *fvlp =  (format_version_list_ty *)data;
    change_set_list_ty *cslp =      (change_set_list_ty *)aux;
    long            window;
    size_t          j;

    /*
     * Sort each version list by date.
     */
    trace(("walker()\n{\n"));
    format_version_list_sort_by_date(fvlp);

    /*
     * Now walk down the list, looking for clumps.
     */
    window = 300;
    for (j = 0; j < fvlp->length;)
    {
        size_t          k;
        string_list_ty  sl;
        change_set_ty   *csp;

        /*
         * Find the end of this clump.
         */
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

        /*
         * Turn the clump into a change set.
         */
        string_list_constructor(&sl);
        csp = change_set_new();
        csp->who = str_copy(fvlp->item[j]->who);
        csp->when = fvlp->item[k - 1]->when;
        trace(("when = %ld\n", csp->when));
        for (; j < k; ++j)
        {
            format_version_ty *fvp;
            change_set_file_action_ty action;
            string_ty       *edit;

            fvp = fvlp->item[j];
            if (fvp->description && fvp->description->str_length)
                string_list_append_unique(&sl, fvp->description);

            /*
             * Figure out what the file action is.
             */
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
        csp->description = wl2str(&sl, 0, sl.nstrings, "\n");
        string_list_destructor(&sl);

        /*
         * Append the change set to the list.
         */
        change_set_list_append(cslp, csp);
    }
    trace(("}\n"));
}


change_set_list_ty *
change_set_find(format_search_list_ty *fslp)
{
    size_t          j;
    change_set_list_ty *result;
    string_list_ty  occupied;

    /*
     * Sort the file versions into separate buckets for each user.
     *
     * For now, only traverse the trunk.  I still doen't know how
     * to associate across files to divine where the branbches are.
     */
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

    /*
     * Process each bucket.
     */
    trace(("mark\n"));
    result = change_set_list_new();
    symtab_walk(stp, walker, result);
    change_set_list_sort_by_date(result);
    symtab_free(stp);
    stp = 0;

    /*
     * Work out where to attach the tags.
     *
     * We do this by working BACKWARDS in time, assigning file tags
     * to change sets.  But (and this is why we work backwards in
     * time) we don't assign tags which have already been seen.
     *
     * In common usage, the tags serve a similar purpose as Aegis'
     * delta numbers.  They are all (typically) applied in a single
     * CVS command, in order that a particular build may be recreated
     * later.  HOWEVER, because each file will be at a different
     * version, and each will have had its latest version included
     * in various random change sets.
     *
     * Tags are used for other things too.  This is simply a guess,
     * but it's a fairly reasonable one.
     */
    string_list_constructor(&occupied);
    for (j = result->length; j > 0; --j)
    {
        change_set_ty   *csp;
        size_t          k;

        csp = result->item[j - 1];
        for (k = 0; k < csp->file.length; ++k)
        {
            change_set_file_ty *csfp;
            size_t          m;

            csfp = csp->file.item + k;
            for (m = 0; m < csfp->tag.nstrings; ++m)
            {
                string_ty       *tag;

                tag = csfp->tag.string[m];
                if (string_list_member(&occupied, tag))
                    continue;
                string_list_append_unique(&csp->tag, tag);
                string_list_append(&occupied, tag);
            }
        }
    }
    string_list_destructor(&occupied);

    trace(("}\n"));
    return result;
}

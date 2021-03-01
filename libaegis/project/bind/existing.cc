//
//      aegis - project change supervisor
//      Copyright (C) 2004-2008 Peter Miller
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

#include <common/ac/ctype.h>
#include <common/ac/stdlib.h>

#include <libaegis/change/branch.h>
#include <common/error.h> // for assert
#include <common/fstrcmp.h>
#include <libaegis/gonzo.h>
#include <libaegis/project.h>
#include <common/str_list.h>
#include <libaegis/sub.h>
#include <common/trace.h>
#include <libaegis/zero.h>


static int
name_has_numeric_suffix(string_ty *name, string_ty **left, long *right)
{
    char            *ep;

    ep = name->str_text + name->str_length;
    while (ep > name->str_text && isdigit((unsigned char)ep[-1]))
        --ep;
    if (!*ep)
    // still pointing to end of string
        return 0;
    if (ep < name->str_text + 2 || !ispunct((unsigned char)ep[-1]))
        return 0;
    *left = str_n_from_c(name->str_text, ep - 1 - name->str_text);
    *right = magic_zero_encode(atol(ep));
    return 1;
}


void
project_ty::bind_existing()
{
    string_ty       *s;
    string_ty       *parent_name;
    int             alias_retry_count;

    //
    // make sure project exists
    //
    trace(("project_ty::bind_existing(this = %08lX)\n{\n", (long)this));
    assert(!home_path);
    alias_retry_count = 0;
    alias_retry:
    s = gonzo_project_home_path_from_name(name);

    //
    // Look to see if there is an alias.
    //
    if (!s)
    {
        s = gonzo_alias_to_actual(name);
        if (s)
        {
            str_free(name);
            name = str_copy(s);
            s = gonzo_project_home_path_from_name(name);
        }
    }

    //
    // If the named project was not found, and it has a numeric suffix,
    // then assume that it is a project.branch combination,
    // and try to find the deeper project.
    //
    if (!s && name_has_numeric_suffix(name, &parent_name, &parent_bn))
    {
        trace(("mark\n"));
        project_ty *ppp = project_alloc(parent_name);
        ppp->bind_existing();
        int err = project_is_readable(ppp);
        if (err != 0)
        {
            off_limits = true;
            project_free(ppp);
            ppp = 0;
        }
        else
        {
            parent = ppp;
            ppp = 0;
            pcp = change_alloc(parent, parent_bn);
            change_bind_existing(pcp);
            if (!change_was_a_branch(pcp))
                change_fatal(pcp, 0, i18n("not a branch"));

            //
            // rebuild the project name
            //  ...eventually, use the remembered punctuation
            //
            str_free(name);
            name =
                str_format
                (
                    "%s.%ld",
                    project_name_get(parent)->str_text,
                    magic_zero_decode(parent_bn)
                );

            changes_path =
                str_format
                (
                    "%s.branch",
                    parent->change_path_get(parent_bn)->str_text
                );

            //
            // This project's user will be the same as the parent's user.
            //
            up = project_user(parent);
            uid = up->get_uid();
            gid = up->get_gid();
            return;
        }
    }

    //
    // If the name was not found, try to find an alias match for it.
    // Loop if an alias is found.
    //
    if (!s)
    {
        string_ty       *other;

        other = gonzo_alias_to_actual(name);
        if (other)
        {
            if (++alias_retry_count > 5)
            {
                project_fatal(this, 0, i18n("alias loop detected"));
            }
            str_free(name);
            name = str_copy(other);
            goto alias_retry;
        }
    }

    //
    // If the name was not found, try to find a fuzzy match for it.
    // In general, this results in more informative error messages.
    //
    if (!s)
    {
        string_list_ty  wl;
        string_ty       *best;
        double          best_weight;
        size_t          j;

        gonzo_project_list(&wl);
        best = 0;
        best_weight = 0.6;
        for (j = 0; j < wl.nstrings; ++j)
        {
            double          w;

            s = wl.string[j];
            w = fstrcmp(name->str_text, s->str_text);
            if (w > best_weight)
            {
                best = s;
                best_weight = w;
            }
        }
        if (best)
        {
            sub_context_ty  *scp;

            scp = sub_context_new();
            sub_var_set_string(scp, "Name", name);
            sub_var_set_string(scp, "Guess", best);
            fatal_intl(scp, i18n("no $name project, closest is $guess"));
        }
        else
        {
            sub_context_ty  *scp;

            scp = sub_context_new();
            sub_var_set_string(scp, "Name", name);
            fatal_intl(scp, i18n("no $name project"));
            // NOTREACHED
            sub_context_delete(scp);
        }
    }

    //
    // To cope with automounters, directories are stored as given,
    // or are derived from the home directory in the passwd file.
    // Within aegis, pathnames have their symbolic links resolved,
    // and any comparison of paths is done on this "system idea"
    // of the pathname.
    //
    home_path = str_copy(s);

    //
    // Create the project user from details of the project path.
    //
    get_the_owner();
    up = user_ty::create(uid, gid);

    //
    // set the umask from the project state data.
    //
    up->umask_set(umask_get());
    trace(("}\n"));
}


bool
project_ty::bind_existing_errok()
{
    string_ty       *s;
    string_ty       *parent_name;
    int             alias_retry_count;

    //
    // make sure project exists
    //
    trace(("project_ty::bind_existing_errok(this = %08lX)\n{\n", (long)this));
    assert(!home_path);
    alias_retry_count = 0;
    alias_retry:
    s = gonzo_project_home_path_from_name(name);

    //
    // If the named project was not found, and it has a numeric suffix,
    // then assume that it is a project.branch combination,
    // and try to find the deeper project.
    //
    if (!s && name_has_numeric_suffix(name, &parent_name, &parent_bn))
    {
        parent = project_alloc(parent_name);
        if (!parent->bind_existing_errok())
        {
            project_free(parent);
            trace(("return false;\n"));
            trace(("}\n"));
            return false;
        }
        pcp = change_alloc(parent, parent_bn);
        if (!change_bind_existing_errok(pcp))
        {
            change_free(pcp);
            project_free(parent);
            trace(("return false;\n"));
            trace(("}\n"));
            return false;
        }
        if (!change_was_a_branch(pcp))
        {
            change_free(pcp);
            project_free(parent);
            trace(("return false;\n"));
            trace(("}\n"));
            return false;
        }

        //
        // rebuild the project name
        //      ...eventually, use the remembered punctuation
        //
        str_free(name);
        name =
            str_format
            (
                "%s.%ld",
                project_name_get(parent)->str_text,
                magic_zero_decode(parent_bn)
            );

        changes_path =
            str_format
            (
                "%s.branch",
                parent->change_path_get(parent_bn)->str_text
            );

        //
        // This project's user will be the same as the parent's user.
        //
        up = project_user(parent);
        uid = up->get_uid();
        gid = up->get_gid();
        trace(("return true;\n"));
        trace(("}\n"));
        return true;
    }

    //
    // If the name was not found, try to find an alias match for it.
    // Loop if an alias is found.
    //
    if (!s)
    {
        string_ty       *other;

        other = gonzo_alias_to_actual(name);
        if (other)
        {
            if (++alias_retry_count > 5)
            {
                trace(("return false;\n"));
                trace(("}\n"));
                return false;
            }
            str_free(name);
            name = str_copy(other);
            goto alias_retry;
        }
    }

    //
    // If the name was not found, try to find a fuzzy match for it.
    // In general, this results in more informative error messages.
    //
    if (!s)
    {
        trace(("return false;\n"));
        trace(("}\n"));
        return false;
    }

    //
    // To cope with automounters, directories are stored as given,
    // or are derived from the home directory in the passwd file.
    // Within aegis, pathnames have their symbolic links resolved,
    // and any comparison of paths is done on this "system idea"
    // of the pathname.
    //
    home_path = str_copy(s);

    //
    // Create the project user from the details of the project directory.
    //
    get_the_owner();
    up = user_ty::create(uid, gid);

    //
    // set the umask from the project state data.
    //
    up->umask_set(umask_get());
    trace(("return true\n"));
    trace(("}\n"));
    return true;
}

//
//      aegis - project change supervisor
//      Copyright (C) 2007, 2008, 2011, 2012 Peter Miller
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
#include <common/ac/ctype.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>

#include <common/nstring/list.h>
#include <common/trace.h>
#include <libaegis/change.h>
#include <libaegis/gonzo.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>
#include <libaegis/zero.h>


static long
is_a_change_number(const char *s)
{
    char *ep = 0;
    long n = strtol(s, &ep, 10);
    if (ep == s || *ep)
        return 0;
    if (n < 0)
        return 0;
    return magic_zero_encode(n);
}


static long
project_dot_change(const nstring &s, const nstring &projnam)
{
    if
    (
        s.size() <= projnam.size() + 1
    ||
        memcmp(s.c_str(), projnam.c_str(), projnam.size())
    ||
        !ispunct((unsigned char)s.c_str()[projnam.size()])
    )
        return 0;
    const char *suffix = s.c_str() + projnam.size() + 1;
    while (isupper((unsigned char)*suffix))
        ++suffix;
    return is_a_change_number(suffix);
}


static bool
is_below(const nstring &hi, const nstring &lo)
{
    if (hi == lo)
        return true;
    return
        (
            lo.size() > hi.size()
        &&
            lo.c_str()[hi.size()] == '/'
        &&
            !memcmp(hi.c_str(), lo.c_str(), hi.size())
        );
}


long
user_ty::default_change(project *pp)
{
    trace(("user_ty::default_change(this = %p, pp = %p)\n{\n",
        this, pp));
    long change_number = 0;

    //
    // check the AEGIS_CHANGE environment variable
    //
    const char *cp = getenv("AEGIS_CHANGE");
    if (cp)
    {
        change_number = is_a_change_number(cp);
        if (!change_number)
        {
            sub_context_ty sc;
            sc.var_set_charstar("Name", "AEGIS_CHANGE");
            sc.fatal_intl(i18n("$$$name must be positive"));
            // NOTREACHED
        }
    }

    //
    // check the $HOME/.aegisrc file
    //
    if (!change_number)
    {
        uconf_ty *ucp = uconf_get();
        if (ucp->mask & uconf_default_change_number_mask)
        {
            change_number = magic_zero_encode(ucp->default_change_number);
        }
    }

    //
    // examine the pathname of the current directory
    // to see if we can extract the change number
    //
    // This only works if the development directory was created
    // by aegis, and not specified by the -DIRectory option.
    // It doesn't work at all for IntDir or BL.
    //
    if (!change_number)
    {
        //
        // get the current directory
        //
        os_become_orig();
        nstring cwd(os_curdir());
        os_become_undo();
        assert(cwd);

        //
        // break it into file names
        //
        nstring_list part;
        part.split(cwd, "/");

        //
        // search for <proj>.<num>
        //
        nstring projnam(project_name_get(pp));
        for (size_t j = 0; j < part.size(); ++j)
        {
            change_number = project_dot_change(part[j], projnam);
            if (change_number)
                break;
        }
    }

    //
    // slower method than above, but works with any directory
    //
    if (!change_number)
    {
        //
        // get the current directory
        //
        os_become_orig();
        nstring cwd(os_curdir());
        os_become_undo();
        assert(cwd);

        //
        // Examine each open change set
        //
        for (size_t j = 0;; ++j)
        {
            long cn = 0;
            if (!project_change_nth(pp, j, &cn))
                break;
            change::pointer cp2 = change_alloc(pp, cn);
            change_bind_existing(cp2);
            cstate_ty *cstate_data = cp2->cstate_get();
            switch (cstate_data->state)
            {
            case cstate_state_being_integrated:
                {
                    nstring d(change_integration_directory_get(cp2, 0));
                    if (is_below(d, cwd))
                    {
                        change_number = cn;
                        break;
                    }
                }
                // fall through...

            case cstate_state_awaiting_review:
            case cstate_state_awaiting_integration:
            case cstate_state_being_reviewed:
            case cstate_state_being_developed:
                {
                    nstring d(change_development_directory_get(cp2, 0));
                    if (is_below(d, cwd))
                    {
                        change_number = cn;
                    }
                }
                break;

            case cstate_state_awaiting_development:
            case cstate_state_completed:
                break;
            }
            change_free(cp2);
            if (change_number)
                break;
        }
    }

    //
    // If the user is only working on one change within the given
    // project, then that is the change.
    //
    if (!change_number)
    {
        ustate_ty *usp = ustate_get(pp);
        assert(usp->own);
        for (size_t j = 0; j < usp->own->length; ++j)
        {
            ustate_own_ty *own_data = usp->own->list[j];
            nstring project_name(project_name_get(pp));
            if (nstring(own_data->project_name) == project_name)
            {
                if (own_data->changes->length == 1)
                    change_number = own_data->changes->list[0];
                break;
            }
        }
    }

    //
    // It is an error if no change number has been given.
    //
    if (!change_number)
        project_fatal(pp, 0, i18n("no change number"));
    trace(("return %ld;\n", change_number));
    trace(("}\n"));
    return change_number;
}


nstring
user_ty::default_project()
{
    trace(("user_ty::default_project()\n{\n"));

    //
    // from the AEGIS_PROJECT environment variable.
    //
    const char *cp = getenv("AEGIS_PROJECT");
    if (cp && *cp)
    {
        nstring result(cp);
        trace(("return %s;\n", result.quote_c().c_str()));
        trace(("}\n"));
        return result;
    }

    //
    // From the $HOME/.aegisrc file.
    //
    uconf_ty *ucp = uconf_get();
    if (ucp->default_project_name)
    {
        nstring result(ucp->default_project_name);
        trace(("return %s;\n", result.quote_c().c_str()));
        trace(("}\n"));
        return result;
    }

    //
    // check the search path, see if we use just one
    //
    // else check the current dirctory to see if we are within one
    //
    // This only works if the development directory was created
    // by aegis, and not specified by the -DIRectory option.
    // It doesn't work at all for IntDir or BL.
    //
    nstring_list names;
    gonzo_project_list_user(login_name, names);
    if (names.size() == 1)
    {
        nstring result(names[0]);
        trace(("return %s;\n", result.quote_c().c_str()));
        trace(("}\n"));
        return result;
    }

    //
    // get pathname of the current directory
    //
    os_become_orig();
    nstring cwd(os_curdir());
    os_become_undo();
    assert(cwd);

    //
    // break into pieces
    //
    nstring_list part;
    part.split(cwd, "/");

    //
    // search the path
    // looking for <proj>.<num>
    //
    for (size_t j = 0; j < part.size(); ++j)
    {
        for (size_t k = 0; k < names.size(); ++k)
        {
            if (project_dot_change(part[j], names[k]))
            {
                nstring result(names[k]);
                trace(("return %s;\n", result.quote_c().c_str()));
                trace(("}\n"));
                return result;
            }
        }
    }

    //
    // check current directory more intensively.
    // this method is slower than previous, but works for
    // baseline and integration directories.
    //
    for (size_t j = 0; j < names.size(); ++j)
    {
        project *pp = project_alloc(names[j].get_ref());
        pp->bind_existing();

        //
        // first check if it is in baseline
        //
        nstring d(pp->baseline_path_get());
        if (is_below(d, cwd))
        {
            nstring result(names[j]);
            project_free(pp);
            trace(("return %s;\n", result.quote_c().c_str()));
            trace(("}\n"));
            return result;
        }

        //
        // second check for every change
        //
        for (size_t k = 0;; ++k)
        {
            long change_number = 0;
            if (!project_change_nth(pp, k, &change_number))
                break;
            change::pointer cp3 = change_alloc(pp, change_number);
            change_bind_existing(cp3);
            cstate_ty *cstate_data = cp3->cstate_get();
            switch (cstate_data->state)
            {
            case cstate_state_being_integrated:
                {
                    nstring id(change_integration_directory_get(cp3, 0));
                    if (is_below(id, cwd))
                    {
                        nstring result(names[j]);
                        trace(("return %s;\n", result.quote_c().c_str()));
                        trace(("}\n"));
                        return result;
                    }
                }
                // fall through...

            case cstate_state_awaiting_review:
            case cstate_state_awaiting_integration:
            case cstate_state_being_reviewed:
            case cstate_state_being_developed:
                {
                    nstring dd(change_development_directory_get(cp3, 0));
                    if (is_below(dd, cwd))
                    {
                        nstring result(names[j]);
                        trace(("return %s;\n", result.quote_c().c_str()));
                        trace(("}\n"));
                        return result;
                    }
                }
                break;

            case cstate_state_awaiting_development:
            case cstate_state_completed:
                break;
            }
            change_free(cp3);
        }
        project_free(pp);
    }

    //
    // It is an error if no project name has been given.
    //
    fatal_intl(0, i18n("no project name"));
    // NOTREACHED
    trace(("}\n"));
    return "";
}


// vim: set ts=8 sw=4 et :

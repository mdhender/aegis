//
// aegis - project change supervisor
// Copyright (C) 1999, 2001-2008, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published
// by the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>

#include <libaegis/change.h>
#include <libaegis/cstate.fmtgen.h>
#include <libaegis/pconf.fmtgen.h>
#include <libaegis/sub.h>


void
change_check_architectures(change::pointer cp)
{
    cstate_ty       *cstate_data;
    pconf_ty        *pconf_data;
    size_t          j, k;
    int             error_count;

    assert(cp->reference_count >= 1);
    cstate_data = cp->cstate_get();
    if (!cstate_data->architecture)
        return; // should not happen
    pconf_data = change_pconf_get(cp, 1);
    assert(pconf_data->architecture);
    if (!pconf_data->architecture)
        return; // should not happen
    error_count = 0;
    for (j = 0; j < cstate_data->architecture->length; ++j)
    {
        string_ty       *variant;

        variant = cstate_data->architecture->list[j];
        for (k = 0; k < pconf_data->architecture->length; ++k)
        {
            pconf_architecture_ty *ap;

            ap = pconf_data->architecture->list[k];
            if (str_equal(variant, ap->name))
            {
                if (ap->mode == pconf_architecture_mode_forbidden)
                {
                    sub_context_ty      *scp;

                    scp = sub_context_new();
                    sub_var_set_string(scp, "Name", variant);
                    change_fatal
                    (
                        cp,
                        scp,
                        i18n("architecture \"$name\" forbidden")
                    );
                    // NOTREACHED
                    sub_context_delete(scp);
                    ++error_count;
                }
                break;
            }
        }
        if (k >= pconf_data->architecture->length)
        {
            sub_context_ty      *scp;

            scp = sub_context_new();
            sub_var_set_string(scp, "Name", variant);
            change_error(cp, scp, i18n("architecture \"$name\" unlisted"));
            sub_context_delete(scp);
            ++error_count;
        }
    }
    if (error_count)
    {
        sub_context_ty  *scp;

        scp = sub_context_new();
        sub_var_set_long(scp, "Number", error_count);
        sub_var_optional(scp, "Number");
        change_fatal(cp, scp, i18n("found unlisted architectures"));
        // NOTREACHED
        sub_context_delete(scp);
    }
}


// vim: set ts=8 sw=4 et :

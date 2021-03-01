//
// aegis - project change supervisor
// Copyright (C) 2004-2008, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/ctype.h>
#include <common/ac/string.h>

#include <common/str_list.h>
#include <libaegis/change.h>
#include <libaegis/change/signedoffby.h>
#include <libaegis/cstate.fmtgen.h>
#include <libaegis/option.h>
#include <libaegis/user.h>


static string_ty *
trim_right(string_ty *s)
{
    size_t len = s->str_length;
    while (len > 0 && s->str_text[len - 1] == '\n')
        --len;
    return str_n_from_c(s->str_text, len);
}


void
change_signed_off_by(change::pointer cp, user_ty::pointer up)
{
    //
    // If the email address is already present, don't do anything else.
    //
    nstring email_address = up->get_email_address();
    cstate_ty *cstate_data = cp->cstate_get();
    string_ty *desc = trim_right(cstate_data->description);
    string_list_ty wl;
    wl.split(desc, "\n", true);
    bool last_was_sob = false;
    for (size_t j = 0; j < wl.nstrings; ++j)
    {
        string_ty *s = wl.string[j];
        last_was_sob = (0 == strncasecmp(s->str_text, "signed-off-by:", 14));
        if (!last_was_sob)
            continue;
        const char *sp = s->str_text + 14;
        while (*sp && isspace((unsigned char)*sp))
            ++sp;
        if (0 == strcasecmp(sp, email_address.c_str()))
            return;
    }

    //
    // Tack the email address onto the end of the description.
    //
    str_free(cstate_data->description);
    cstate_data->description =
        str_format
        (
            "%s\n%sSigned-off-by: %s",
            desc->str_text,
            (last_was_sob ? "" : "\n"),
            email_address.c_str()
        );
    str_free(desc);
}


bool
change_signed_off_by_get(change::pointer cp)
{
    pconf_ty *pconf_data = change_pconf_get(cp, 0);
    return (option_signed_off_by_get(pconf_data->signed_off_by));
}


// vim: set ts=8 sw=4 et :

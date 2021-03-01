//
// aegis - project change supervisor
// Copyright (C) 2007, 2008, 2012 Peter Miller
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

#include <libaegis/arglex2.h>
#include <libaegis/help.h>
#include <libaegis/user.h>


static int uconf_symlink_pref_option = -1;


void
user_ty::symlink_pref_argument(void (*usage)(void))
{
    if (uconf_symlink_pref_option >= 0)
        duplicate_option(usage);
    switch (arglex_token)
    {
    default:
        assert(0);
        return;

    case arglex_token_symbolic_links:
        uconf_symlink_pref_option = 1;
        break;

    case arglex_token_symbolic_links_not:
        uconf_symlink_pref_option = 0;
        break;
    }
}


bool
user_ty::symlink_pref(bool proj_files_changed)
{
    if (uconf_symlink_pref_option < 0)
    {
        if (proj_files_changed)
        {
            uconf_symlink_pref_option = 1;
        }
        else
        {
            uconf_ty *ucp = uconf_get();
            uconf_symbolic_link_preference_ty result =
                ucp->symbolic_link_preference;
            uconf_symlink_pref_option =
                (result == uconf_symbolic_link_preference_verify);
        }
    }
    return (uconf_symlink_pref_option != 0);
}


// vim: set ts=8 sw=4 et :

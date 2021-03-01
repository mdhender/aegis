//
//	aegis - project change supervisor
//	Copyright (C) 2007, 2008 Peter Miller
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

#include <common/error.h> // for assert
#include <libaegis/arglex2.h>
#include <libaegis/help.h>
#include <libaegis/user.h>


static int user_relative_filename_preference_option = -1;


void
user_ty::relative_filename_preference_argument(void (*usage)(void))
{
    if (user_relative_filename_preference_option >= 0)
	duplicate_option(usage);
    switch (arglex_token)
    {
    default:
	assert(0);
	return;

    case arglex_token_base_relative:
	user_relative_filename_preference_option =
	    uconf_relative_filename_preference_base;
	break;

    case arglex_token_current_relative:
	user_relative_filename_preference_option =
	    uconf_relative_filename_preference_current;
	break;
    }
}


uconf_relative_filename_preference_ty
user_ty::relative_filename_preference(
    uconf_relative_filename_preference_ty dflt)
{
    if (user_relative_filename_preference_option < 0)
    {
	uconf_ty *ucp = uconf_get();
	if (ucp->mask & uconf_relative_filename_preference_mask)
	    dflt = ucp->relative_filename_preference;
	user_relative_filename_preference_option = dflt;
    }
    return
	(uconf_relative_filename_preference_ty)
        user_relative_filename_preference_option;
}

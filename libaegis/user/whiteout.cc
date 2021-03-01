//
//	aegis - project change supervisor
//	Copyright (C) 2007 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//

#include <common/error.h> // for assert
#include <libaegis/arglex2.h>
#include <libaegis/help.h>
#include <libaegis/user.h>


static int uconf_whiteout_option = -1;


void
user_ty::whiteout_argument(void (*usage)(void))
{
    if (uconf_whiteout_option >= 0)
	duplicate_option(usage);
    switch (arglex_token)
    {
    default:
	assert(0);
	return;

    case arglex_token_whiteout:
	uconf_whiteout_option = 1;
	break;

    case arglex_token_whiteout_not:
	uconf_whiteout_option = 0;
	break;
    }
}


bool
user_ty::whiteout(int dflt)
{
    if (uconf_whiteout_option < 0)
    {
	if (dflt >= 0)
	{
	    uconf_whiteout_option = dflt;
	}
	else
	{
	    uconf_ty *ucp = uconf_get();
	    uconf_whiteout_preference_ty result = ucp->whiteout_preference;
	    uconf_whiteout_option =
		(result == uconf_whiteout_preference_always);
	}
        assert(uconf_whiteout_option >= 0);
    }
    return (uconf_whiteout_option != 0);
}

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
// MANIFEST: implementation of the user_progress class
//

#include <libaegis/arglex2.h>
#include <libaegis/attribute.h>
#include <libaegis/help.h>
#include <libaegis/user.h>


static int flag = -1;


void
user_ty::progress_option_set(void (*usage)(void))
{
    if (flag == 1)
        duplicate_option(usage);
    if (flag >= 0)
    {
        mutually_exclusive_options
        (
            arglex_token_progress,
            arglex_token_progress_not,
            usage
        );
    }
    flag = 1;
}


void
user_ty::progress_option_clear(void (*usage)(void))
{
    if (flag == 0)
        duplicate_option(usage);
    if (flag >= 0)
    {
        mutually_exclusive_options
        (
            arglex_token_progress,
            arglex_token_progress_not,
            usage
        );
    }
    flag = 0;
}


bool
user_ty::progress_get()
{
    if (flag < 0)
    {
        flag =
            attributes_list_find_boolean
            (
                uconf_get()->attribute,
                "progress-preference",
                false
            );
    }
    return (flag != 0);
}

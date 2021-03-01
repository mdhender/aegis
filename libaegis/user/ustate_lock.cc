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
#include <common/trace.h>
#include <libaegis/arglex2.h>
#include <libaegis/help.h>
#include <libaegis/lock.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>


void
user_ty::waiting_for_lock(void *p)
{
    user_ty *up = (user_ty *)p;
    up->waiting_for_lock();
}


void
user_ty::waiting_for_lock()
{
    if (lock_wait())
    {
        sub_context_ty sc;
        nstring s(subst_intl(&sc, i18n("waiting for lock")));

        sub_context_ty sc2;
        sc2.var_set_string("MeSsaGe", s);
        sc2.var_set_string("Name", login_name);
        sc2.error_intl(i18n("user $name: $message"));
    }
    else
    {
        sub_context_ty sc;
        nstring s(subst_intl(&sc, i18n("lock not available")));

        sub_context_ty sc2;
        sc2.var_set_string("MeSsaGe", s);
        sc2.var_set_string("Name", login_name);
        sc2.fatal_intl(i18n("user $name: $message"));
    }
}


void
user_ty::ustate_lock_prepare()
{
    trace(("user_ty::ustate_lock_prepare(this = %08lX)\n{\n", (long)this));
    lock_prepare_ustate(user_id, waiting_for_lock, this);
    trace(("}\n"));
}


static int uconf_lock_wait_option = -1;


void
user_ty::lock_wait_argument(void (*usage)(void))
{
    if (uconf_lock_wait_option >= 0)
	duplicate_option(usage);
    switch (arglex_token)
    {
    default:
	assert(0);
	return;

    case arglex_token_wait:
	uconf_lock_wait_option = 1;
	break;

    case arglex_token_wait_not:
	uconf_lock_wait_option = 0;
	break;
    }
}


bool
user_ty::lock_wait()
{
    if (uconf_lock_wait_option < 0)
    {
	uconf_ty *ucp = uconf_get();
	uconf_lock_wait_preference_ty result = ucp->lock_wait_preference;
	if (result == uconf_lock_wait_preference_background)
	{
	    result =
		(
		    os_background()
		?
		    uconf_lock_wait_preference_always
		:
		    uconf_lock_wait_preference_never
		);
	}
	uconf_lock_wait_option = (result == uconf_lock_wait_preference_always);
    }
    return (uconf_lock_wait_option != 0);
}

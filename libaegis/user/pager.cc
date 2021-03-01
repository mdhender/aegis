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

#include <common/ac/stdlib.h>

#include <common/error.h> // for assert
#include <common/trace.h>
#include <libaegis/os.h>
#include <libaegis/user.h>


nstring
user_ty::pager_command()
{
    trace(("user_ty::pager_command()\n{\n"));
    assert(!os_become_active());
    uconf_ty *ucp = uconf_get();
    if (!ucp->pager_command)
    {
	const char *pager = getenv("PAGER");
	if (!pager || !*pager)
	    pager = "more";
	ucp->pager_command = str_from_c(pager);
    }
    trace(("return \"%s\";\n", ucp->pager_command->str_text));
    trace(("}\n"));
    return nstring(ucp->pager_command);
}


bool
user_ty::pager_preference()
{
    uconf_ty *ucp = uconf_get();
    return (ucp->pager_preference == uconf_pager_preference_foreground);
}

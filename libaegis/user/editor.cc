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
#include <libaegis/sub.h>
#include <libaegis/user.h>


nstring
user_ty::editor_command()
{
    trace(("user_ty::editor_command()\n{\n"));
    assert(!os_become_active());
    uconf_ty *ucp = uconf_get();
    if (!ucp->editor_command)
    {
	const char *editor = getenv("EDITOR");
	if (!editor || !*editor)
	{
	    sub_context_ty sc;
	    sc.var_set_charstar("Name", "EDITOR");
	    sc.fatal_intl(i18n("environment variable $name not set"));
	    // NOTREACHED
	}
	ucp->editor_command = str_from_c(editor);
    }
    trace(("return \"%s\";\n", ucp->editor_command->str_text));
    trace(("}\n"));
    return nstring(ucp->editor_command);
}

//
//	aegis - project change supervisor
//	Copyright (C) 1991-2006, 2008 Peter Miller
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

#include <common/ac/errno.h>
#include <common/ac/unistd.h>

#include <libaegis/glue.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>
#include <common/trace.h>


void
os_link(const nstring &from, const nstring &to)
{
    trace(("os_link(from = \"%s\", to = \"%s\")\n{\n",
	from.c_str(), to.c_str()));
    os_become_must_be_active();
    if (glue_link(from.c_str(), to.c_str()))
    {
	int errno_old = errno;
	sub_context_ty sc;
	sc.errno_setx(errno_old);
	sc.var_set_string("File_Name1", from);
	sc.var_set_string("File_Name2", to);
	sc.fatal_intl(i18n("link(\"$filename1\", \"$filename2\"): $errno"));
	// NOTREACHED
    }
    trace(("}\n"));
}


void
os_link(string_ty *from, string_ty *to)
{
    os_link(nstring(from), nstring(to));
}

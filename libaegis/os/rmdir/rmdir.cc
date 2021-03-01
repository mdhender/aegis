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
//	along with this program; if not, see
//	<http://www.gnu.org/licenses/>.
//

#include <common/ac/errno.h>
#include <common/ac/unistd.h>

#include <libaegis/glue.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>
#include <common/trace.h>


void
os_rmdir(const nstring &path)
{
    trace(("os_rmdir(path = \"%s\")\n{\n", path.c_str()));
    os_become_must_be_active();
    if (glue_rmdir(path.c_str()) && errno != ENOENT)
    {
	int errno_old = errno;
	sub_context_ty *scp = sub_context_new();
	sub_errno_setx(scp, errno_old);
	sub_var_set_string(scp, "File_Name", path);
	fatal_intl(scp, i18n("rmdir $filename: $errno"));
	// NOTREACHED
    }
    trace(("}\n"));
}


void
os_rmdir(string_ty *path)
{
    os_rmdir(nstring(path));
}

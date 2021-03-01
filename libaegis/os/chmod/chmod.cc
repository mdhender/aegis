//
//	aegis - project change supervisor
//	Copyright (C) 1991-2006 Peter Miller
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
// MANIFEST: wrappers around operating system functions
//

#include <common/ac/errno.h>
#include <common/ac/stddef.h>
#include <common/ac/sys/types.h>
#include <sys/stat.h>

#include <libaegis/glue.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>
#include <common/trace.h>


void
os_chmod(string_ty *path, int mode)
{
    os_chmod(nstring(str_copy(path)), mode);
}


void
os_chmod(const nstring &path, int mode)
{
    trace(("os_chmod(path = \"%s\", mode = 0%o)\n{\n", path.c_str(), mode));
    mode &= 07777;
    os_become_must_be_active();
    if (glue_chmod(path.c_str(), mode))
    {
	int errno_old = errno;
	sub_context_ty *scp = sub_context_new();
	sub_errno_setx(scp, errno_old);
	sub_var_set_string(scp, "File_Name", path);
	sub_var_set_format(scp, "Argument", "%5.5o", mode);
	fatal_intl(scp, i18n("chmod(\"$filename\", $arg): $errno"));
	// NOTREACHED
    }
    trace(("}\n"));
}

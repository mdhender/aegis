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
#include <common/ac/stddef.h>
#include <common/ac/sys/types.h>
#include <common/ac/sys/stat.h>

#include <libaegis/glue.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>
#include <common/trace.h>


int
os_chmod_query(string_ty *path)
{
    int             mode;
    struct stat     st;
    int             oret;

    trace(("os_chmod_query(path = \"%s\")\n{\n", path->str_text));
    os_become_must_be_active();
#ifdef S_IFLNK
    oret = glue_lstat(path->str_text, &st);
#else
    oret = glue_stat(path->str_text, &st);
#endif
    if (oret)
    {
	sub_context_ty  *scp;
	int             errno_old;

	errno_old = errno;
	scp = sub_context_new();
	sub_errno_setx(scp, errno_old);
	sub_var_set_string(scp, "File_Name", path);
	fatal_intl(scp, i18n("stat $filename: $errno"));
	// NOTREACHED
    }
    mode = st.st_mode & 07777;
    trace(("return %05o;\n", mode));
    trace(("}\n"));
    return mode;
}

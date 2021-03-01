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

#include <common/ac/stddef.h>
#include <common/ac/sys/types.h>
#include <sys/stat.h>

#include <libaegis/glue.h>
#include <libaegis/os.h>
#include <common/trace.h>


int
os_symlink_query(string_ty *path)
{
    int             result;
    struct stat     st;

    trace(("os_symlink_query(\"%s\")\n{\n", path->str_text));
    os_become_must_be_active();
#ifdef S_IFLNK
    result =
	(!glue_lstat(path->str_text, &st) && (st.st_mode & S_IFMT) == S_IFLNK);
#else
    result = 0;
#endif
    trace(("return %d;\n", result));
    trace(("}\n"));
    return result;
}

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
#include <common/ac/stddef.h>
#include <common/ac/sys/types.h>
#include <common/ac/sys/stat.h>

#include <libaegis/glue.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>


bool
os_exists(const nstring &path, bool eaccess_is_ok)
{
    os_become_must_be_active();
    struct stat st;
#ifdef S_IFLNK
    int oret = glue_lstat(path.c_str(), &st);
#else
    int oret = glue_stat(path.c_str(), &st);
#endif
    if (oret)
    {
	int errno_old = errno;
	switch (errno_old)
	{
	case ENOENT:
	case ENOTDIR:
	    break;

	case EACCES:
	    if (eaccess_is_ok)
		break;
	    // fall through...

	default:
	    sub_context_ty sc;
	    sc.errno_setx(errno_old);
	    sc.var_set_string("File_Name", path);
	    sc.fatal_intl(i18n("stat $filename: $errno"));
	    // NOTREACHED
	}
	return false;
    }
    return true;
}


int
os_exists(string_ty *path, bool eaccess_is_ok)
{
    return os_exists(nstring(path), eaccess_is_ok);
}

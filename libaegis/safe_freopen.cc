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

#include <common/ac/errno.h>
#include <common/ac/stdio.h>

#include <libaegis/os.h>
#include <libaegis/safe_freopen.h>
#include <libaegis/sub.h>


void
safe_freopen(const char *path, const char *mode, FILE *fp)
{
    os_become_orig();
    if (!freopen(path, mode, fp))
    {
        int err = errno;
        sub_context_ty sc;
        sc.errno_setx(err);
        sc.var_set_charstar("File_Name", path);
        sc.fatal_intl(i18n("open $filename: $errno"));
        // NOTREACHED
    }
    os_become_undo();
}

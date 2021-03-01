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
#include <utime.h>

#include <libaegis/glue.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>
#include <common/trace.h>


void
os_mtime_set(string_ty *path, time_t when)
{
    struct utimbuf  utb;
    int             err;

    trace(("os_mtime_set(path = %08lX, when = %ld)\n{\n", (long)path,
	(long)when));
    os_become_must_be_active();
    trace_string(path->str_text);
    trace(("when = %s", ctime(&when)));
    utb.actime = when;
    utb.modtime = when;
    err = glue_utime(path->str_text, &utb);
#ifdef __CYGWIN__
    if (err && errno == EACCES)
	err = 0;
#endif
    if (err)
    {
	sub_context_ty  *scp;
	int             errno_old;

	errno_old = errno;
	scp = sub_context_new();
	sub_errno_setx(scp, errno_old);
	sub_var_set_string(scp, "File_Name", path);
	fatal_intl(scp, i18n("utime $filename: $errno"));
	// NOTREACHED
    }
    trace(("}\n"));
}

//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002-2004 Peter Miller;
//	All rights reserved.
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
// MANIFEST: functions to manipulate sizes
//

#include <ac/errno.h>
#include <ac/sys/types.h>
#include <sys/stat.h>

#include <glue.h>
#include <os.h>
#include <sub.h>
#include <trace.h>


long
os_file_size(string_ty *path)
{
    trace(("os_mtime_actual(path = %08lX)\n{\n", (long)path));
    os_become_must_be_active();
    trace_string(path->str_text);

    struct stat	st;
    int oret = glue_stat(path->str_text, &st);
    if (oret)
    {
	int errno_old = errno;
	sub_context_ty sc;
	sc.errno_setx(errno_old);
	sc.var_set_string("File_Name", path);
	sc.fatal_intl(i18n("stat $filename: $errno"));
	// NOTREACHED
    }

    trace(("return %ld;\n", (long)st.st_size));
    trace(("}\n"));
    return st.st_size;
}

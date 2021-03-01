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
#include <common/ac/stddef.h>
#include <common/ac/sys/types.h>
#include <common/ac/sys/stat.h>

#include <common/error.h>
#include <libaegis/glue.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>
#include <common/trace.h>


void
os_symlink(string_ty *src, string_ty *dst)
{
    os_symlink(nstring(src), nstring(dst));
}


void
os_symlink(const nstring &src, const nstring &dst)
{
    trace(("os_symlink(src = \"%s\", dst - \"%s\")\n{\n", src.c_str(),
       	dst.c_str()));
    os_become_must_be_active();
#ifdef S_IFLNK
    if (glue_symlink(src.c_str(), dst.c_str()))
    {
	int errno_old = errno;
	sub_context_ty sc;
	sc.errno_setx(errno_old);
	sc.var_set_string("File_Name1", src);
	sc.var_set_string("File_Name2", dst);
	sc.fatal_intl
	(
	    i18n("symlink(\"$filename1\", \"$filename2\"): $errno")
	);
    }
#else
    fatal_raw
    (
	"symlink(\"%s\", \"%s\"): symbolic links not available",
	src.c_str(),
	dst.c_str()
    );
#endif
    trace(("}\n"));
}

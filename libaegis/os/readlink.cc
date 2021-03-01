//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2003-2006, 2008 Peter Miller
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
#include <common/ac/unistd.h>
#include <common/ac/string.h>
#include <common/ac/sys/types.h>
#include <common/ac/sys/clu.h>
#include <common/ac/sys/stat.h>

#include <common/error.h>
#include <libaegis/glue.h>
#include <libaegis/os.h>
#include <common/stracc.h>
#include <libaegis/sub.h>
#include <common/trace.h>


string_ty *
os_readlink(string_ty *path)
{
	int		nbytes;
	string_ty	*result;
	char		buffer[2000];

	trace(("os_readlink(\"%s\")\n{\n", path->str_text));
	os_become_must_be_active();
#ifdef S_IFLNK
	nbytes = glue_readlink(path->str_text, buffer, sizeof(buffer));
	if (nbytes < 0)
	{
		sub_context_ty	*scp;
		int             errno_old;

		errno_old = errno;
		scp = sub_context_new();
		sub_errno_setx(scp, errno_old);
		sub_var_set_string(scp, "File_Name", path);
		fatal_intl(scp, i18n("readlink $filename: $errno"));
		sub_context_delete(scp);
	}
	if (nbytes == 0)
	{
		buffer[0] = '.';
		nbytes = 1;
	}
	result = str_n_from_c(buffer, nbytes);
#else
	fatal_raw
	(
		"readlink(\"%s\"): symbolic links not available",
		path->str_text,
	);
	result = str_copy(path);
#endif
	trace(("return \"%s\";\n", result->str_text));
	trace(("}\n"));
	return result;
}

/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate readlinks
 */

#include <ac/stddef.h>
#include <ac/unistd.h>
#include <ac/string.h>
#include <sys/types.h>
#include <ac/sys/clu.h>
#include <sys/stat.h>

#include <error.h>
#include <glue.h>
#include <os.h>
#include <stracc.h>
#include <sub.h>
#include <trace.h>


string_ty *
os_readlink(path)
	string_ty	*path;
{
	int		nbytes;
	string_ty	*result;
	char		buffer[2000];

	trace(("os_readlink(\"%s\")\n{\n"/*}*/, path->str_text));
	os_become_must_be_active();
#ifdef S_IFLNK
	nbytes = glue_readlink(path->str_text, buffer, sizeof(buffer));
	if (nbytes < 0)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
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
	trace((/*{*/"}\n"));
	return result;
}
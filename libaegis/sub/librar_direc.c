/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2003 Peter Miller;
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
 * MANIFEST: functions to manipulate librar_direcs
 */

#include <libdir.h>
#include <sub.h>
#include <sub/librar_direc.h>
#include <trace.h>
#include <wstr_list.h>


/*
 * NAME
 *	sub_library_directory - the library substitution
 *
 * SYNOPSIS
 *	string_ty *sub_library_directory(wstring_list_ty *arg);
 *
 * DESCRIPTION
 *	The sub_library_directory function implements the library
 *	substitution.  The library_directory substitution is replaced
 *	by the absolute path of aegis' library directory.
 *
 * ARGUMENTS
 *	arg	- list of arguments, including the function name as [0]
 *
 * RETURNS
 *	a pointer to a string in dynamic memory;
 *	or NULL on error, setting suberr appropriately.
 */

wstring_ty *
sub_library_directory(sub_context_ty *scp, wstring_list_ty *arg)
{
	wstring_ty	*result;

	trace(("sub_library_directory()\n{\n"/*}*/));
	if (arg->nitems != 1)
	{
		sub_context_error_set(scp, i18n("requires zero arguments"));
		result = 0;
	}
	else
		result = wstr_from_c(configured_libdir());
	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}

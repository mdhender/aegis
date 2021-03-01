//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2003-2005 Peter Miller;
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
// MANIFEST: functions to manipulate common_direcs
//

#include <common/libdir.h>
#include <libaegis/sub.h>
#include <libaegis/sub/common_direc.h>
#include <common/trace.h>
#include <common/wstr/list.h>


//
// NAME
//	sub_common_directory - the shared library substitution
//
// SYNOPSIS
//	string_ty *sub_common_directory(wstring_list_ty *arg);
//
// DESCRIPTION
//	The sub_common_directory function implements the common_directory
//	substitution.  The common_directory substitution is replaced by
//	the absolute path of aegis' common (network shared writable)
//	data directory.  See ${data_directory} for the architecture
//	neutral (maybe readonly) library directory.
//
// ARGUMENTS
//	arg	- list of arguments, including the function name as [0]
//
// RETURNS
//	a pointer to a string in dynamic memory;
//	or NULL on error, setting suberr appropriately.
//

wstring_ty *
sub_common_directory(sub_context_ty *scp, wstring_list_ty *arg)
{
	wstring_ty	*result;

	trace(("sub_library()\n{\n"));
	if (arg->size() != 1)
	{
		sub_context_error_set(scp, i18n("requires zero arguments"));
		result = 0;
	}
	else
		result = wstr_from_c(configured_comdir());
	trace(("return %8.8lX;\n", (long)result));
	trace(("}\n"));
	return result;
}

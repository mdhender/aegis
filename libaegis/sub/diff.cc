//
//	aegis - project change supervisor
//	Copyright (C) 2005 Peter Miller;
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
// MANIFEST: implementation of the sub_diff class
//

#include <libaegis/sub.h>
#include <libaegis/sub/diff.h>
#include <common/trace.h>
#include <common/wstr/list.h>


wstring_ty *
sub_diff(sub_context_ty *scp, wstring_list_ty *arg)
{
    trace(("sub_diff()\n{\n"));
    wstring_ty *result = 0;
    if (arg->size() != 1)
	sub_context_error_set(scp, i18n("requires zero arguments"));
    else
	result = wstr_from_c(CONF_DIFF);
    trace(("return %8.8lX;\n", (long)result));
    trace(("}\n"));
    return result;
}

//
//	aegis - project change supervisor
//	Copyright (C) 2006 Peter Miller;
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
// MANIFEST: implementation of the $copyright_owner substitution
//

#include <common/nstring.h>
#include <common/trace.h>
#include <common/wstr/list.h>
#include <libaegis/change.h>
#include <libaegis/change/attributes.h>
#include <libaegis/sub/copyri_owner.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>


wstring_ty *
sub_copyright_owner(sub_context_ty *scp, wstring_list_ty *arg)
{
    trace(("sub_copyright_owner()\n{\n"));
    change_ty *cp = sub_context_change_get(scp);
    if (!cp)
    {
	sub_context_error_set(scp, i18n("not valid in current context"));
	trace(("return NULL;\n"));
	trace(("}\n"));
	return 0;
    }
    if (arg->size() != 1)
    {
	sub_context_error_set(scp, i18n("requires zero arguments"));
	trace(("return NULL;\n"));
	trace(("}\n"));
	return 0;
    }

    nstring value = cp->pconf_copyright_owner_get();
    wstring_ty *result = str_to_wstr(value.get_ref());
    trace(("return %8.8lX;\n", (long)result));
    trace(("}\n"));
    return result;
}

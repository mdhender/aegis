//
//	aegis - project change supervisor
//	Copyright (C) 2004 Peter Miller;
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
// MANIFEST: implementation of the sub_change_attribute class
//

#include <change/attributes.h>
#include <sub.h>
#include <sub/change/attribute.h>
#include <trace.h>
#include <wstr/list.h>


wstring_ty *
sub_change_attribute(sub_context_ty *scp, wstring_list_ty *arg)
{
    trace(("sub_change_attribute()\n{\n"));
    change_ty *cp = sub_context_change_get(scp);
    if (!cp)
    {
	sub_context_error_set(scp, i18n("not valid in current context"));
	trace(("return NULL;\n"));
	trace(("}\n"));
	return 0;
    }
    if (arg->size() != 2)
    {
	sub_context_error_set(scp, i18n("requires one argument"));
	trace(("return NULL;\n"));
	trace(("}\n"));
	return 0;
    }

    string_ty *name = wstr_to_str(arg->get(1));
    string_ty *value = change_attributes_find(cp, name);
    str_free(name);
    wstring_ty *result = value ? str_to_wstr(value) : wstr_from_c("");
    trace(("return %8.8lX;\n", (long)result));
    trace(("}\n"));
    return result;
}

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
// MANIFEST: functions to manipulate branchs
//

#include <change.h>
#include <error.h> // for assert
#include <project.h>
#include <trace.h>


project_ty *
project_bind_branch(project_ty *ppp, change_ty *cp)
{
    string_ty	    *project_name;
    project_ty	    *pp;

    trace(("project_bind_branch(ppp = %08lX, cp = %8.8lX)\n{\n", (long)ppp,
	(long)cp));
    trace(("ppp->name = \"%s\"\n", ppp->name->str_text));
    trace(("cp->number = %ld\n", cp->number));
    if (cp->number == TRUNK_CHANGE_NUMBER)
    {
	trace(("no need to be clever\n"));
	assert(!ppp->parent);
	pp = project_copy(ppp);
    }
    else
    {
	// punctuation?
	project_name =
	    str_format
	    (
		"%s.%ld",
		ppp->name->str_text,
		magic_zero_decode(cp->number)
	    );
	trace(("name = \"%s\"\n", project_name->str_text));
	pp = project_alloc(project_name);
	pp->parent = project_copy(ppp);
	pp->parent_bn = cp->number;
	pp->pcp = cp;
	pp->changes_path =
	    str_format
	    (
		"%s.branch",
		project_change_path_get(ppp, cp->number)->str_text
	    );
	trace(("change path = \"%s\"\n", pp->changes_path->str_text));
    }
    trace(("return %8.8lX;\n", (long)pp));
    trace(("}\n"));
    return pp;
}

//
//	aegis - project change supervisor
//	Copyright (C) 2004-2006 Peter Miller;
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

#include <libaegis/change.h>
#include <common/error.h> // for assert
#include <libaegis/project.h>
#include <common/trace.h>


project_ty *
project_ty::bind_branch(change_ty *cp)
{
    assert(cp->pp == this);
    project_ty *pp = 0;
    trace(("project_ty::bind_branch(this = %08lX, cp = %08lX)\n{\n",
	(long)this, (long)cp));
    trace(("name = \"%s\"\n", name->str_text));
    trace(("cp->number = %ld\n", cp->number));
    if (cp->number == TRUNK_CHANGE_NUMBER)
    {
	trace(("no need to be clever\n"));
	assert(!parent);
	pp = project_copy(this);
    }
    else
    {
	// punctuation?
	string_ty *project_name =
	    str_format
	    (
		"%s.%ld",
		name->str_text,
		magic_zero_decode(cp->number)
	    );
	trace(("name = \"%s\"\n", project_name->str_text));
	pp = project_alloc(project_name);
	pp->parent = project_copy(this);
	pp->parent_bn = cp->number;
	pp->pcp = cp;
	pp->changes_path =
	    str_format("%s.branch", change_path_get(cp->number)->str_text);
	trace(("change path = \"%s\"\n", pp->changes_path->str_text));
    }
    trace(("return %08lX;\n", (long)pp));
    trace(("}\n"));
    return pp;
}

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
// MANIFEST: functions to manipulate find_by_uuids
//

#include <error.h> // for assert
#include <project/file.h>
#include <symtab.h>
#include <trace.h>


fstate_src_ty *
project_file_find_by_uuid(project_ty *pp, string_ty *uuid,
    view_path_ty as_view_path)
{
    //
    // Invert the list of sources on the UUID field.
    // This makes it an O(1) search instead of an O(n) search.
    //
    trace(("project_file_find_by_uuid(pp = %8.8lX, uuid = \"%s\")\n{\n",
	(long)pp, uuid->str_text));
    symtab_ty *stp = pp->file_by_uuid[as_view_path];
    if (!stp)
    {
	//
	// It would be nice to think that the file appeared exactly once,
	// and the first file entry with the correct UUID is the one we
	// want.  Unfortunately, moves are modeled as a remove and a create,
	// so we want the create or modify action in preference to the
	// remove action.
	//
	assert(file_action_create < file_action_remove);
	assert(file_action_modify < file_action_remove);

	stp = symtab_alloc(5);
	pp->file_by_uuid[as_view_path] = stp;
	for (size_t j = 0; ; ++j)
	{
	    trace(("j = %d\n", (int)j));
	    fstate_src_ty *src = project_file_nth(pp, j, as_view_path);
	    if (!src)
		break;
	    if (!src->uuid)
		continue;
	    fstate_src_ty *prev = (fstate_src_ty *)symtab_query(stp, src->uuid);
	    if (!prev || src->action < prev->action)
		symtab_assign(stp, src->uuid, src);
	}
    }

    //
    // Look for the UUID in the symbol table.
    //
    fstate_src_ty *result = (fstate_src_ty *)symtab_query(stp, uuid);
    trace(("%s %s \"%s\" %s %s\n", file_usage_ename(result->usage),
	file_action_ename(result->action), result->file_name->str_text,
	(result->edit_origin ? result->edit_origin->revision->str_text : ""),
	(result->edit ? result->edit->revision->str_text : "")));
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}

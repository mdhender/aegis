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
// MANIFEST: functions to manipulate find_by_uuids
//

#include <common/error.h> // for assert
#include <libaegis/project/file.h>
#include <common/symtab.h>
#include <common/trace.h>


fstate_src_ty *
project_ty::file_find_by_uuid(string_ty *uuid, view_path_ty as_view_path)
{
    //
    // Invert the list of sources on the UUID field.
    // This makes it an O(1) search instead of an O(n) search.
    //
    trace(("project_ty::file_find_by_uuid(this = %8.8lX, uuid = \"%s\")\n{\n",
	(long)this, uuid->str_text));

    //
    // Deep down in the call stack via project_file_nth, Aegis may
    // sometimes need to invalidate the file_by_uuid cache, so we have
    // to call it first, or we will segfault when it gets deleted out
    // from under us.  The simpest method of doing this is via the
    // project_ty::change_get method.
    //
    // An alternative would be to expose the libaegis/project.cc::
    // lock_sync() function for calling.  Maybe one day in the distant
    // future, when project_ty is a class and not a struct.
    //
    change_get();

    symtab_ty *stp = file_by_uuid[as_view_path];
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

	stp = new symtab_ty(5);
	file_by_uuid[as_view_path] = stp;
	for (size_t j = 0; ; ++j)
	{
	    trace(("j = %d\n", (int)j));
	    fstate_src_ty *src = file_nth(j, as_view_path);
	    if (!src)
		break;
	    if (!src->uuid)
		continue;
	    fstate_src_ty *prev = (fstate_src_ty *)stp->query(src->uuid);
	    if (!prev || src->action < prev->action)
		stp->assign(src->uuid, src);
	}
    }

    //
    // Look for the UUID in the symbol table.
    //
    fstate_src_ty *result = (fstate_src_ty *)stp->query(uuid);
    if (result)
    {
	trace(("%s %s \"%s\" %s %s\n", file_usage_ename(result->usage),
	    file_action_ename(result->action), result->file_name->str_text,
	    (result->edit_origin && result->edit_origin->revision
	       	? result->edit_origin->revision->str_text : ""),
	    (result->edit && result->edit->revision
		? result->edit->revision->str_text : "")));
    }
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}

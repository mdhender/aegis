//
//      aegis - project change supervisor
//      Copyright (C) 2004-2006, 2008, 2011, 2012 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>

#include <common/mem.h>
#include <common/symtab.h>
#include <common/trace.h>
#include <libaegis/project/file.h>


fstate_src_ty *
project::file_find_by_uuid(string_ty *uuid, view_path_ty as_view_path)
{
    //
    // Invert the list of sources on the UUID field.
    // This makes it an O(1) search instead of an O(n) search.
    //
    trace(("project::file_find_by_uuid(this = %p, uuid = \"%s\")\n{\n",
        this, uuid->str_text));

    //
    // Deep down in the call stack via project_file_nth, Aegis may
    // sometimes need to invalidate the file_by_uuid cache, so we have
    // to call it first, or we will segfault when it gets deleted out
    // from under us.  The simpest method of doing this is via the
    // project::change_get method.
    //
    // An alternative would be to expose the libaegis/project.cc::
    // lock_sync() function for calling.  Maybe one day in the distant
    // future, when project is a class and not a struct.
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
    trace(("return %p;\n", result));
    trace(("}\n"));
    return result;
}


// vim: set ts=8 sw=4 et :

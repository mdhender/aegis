//
//      aegis - project change supervisor
//      Copyright (C) 2005-2008, 2012 Peter Miller
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

#include <libaegis/change/file.h>
#include <common/trace.h>


string_ty *
change::file_path(fstate_src_ty *src)
{
    trace(("change::file_path(this = %p, src = \"%p\")\n{\n", this,
        src));
    string_ty *result =
        (
            src->uuid
        ?
            file_path_by_uuid(src->uuid)
        :
            file_path(src->file_name)
        );
    trace(("return \"%s\";\n", (result ? result->str_text : "")));
    trace(("}\n"));
    return result;
}


// vim: set ts=8 sw=4 et :

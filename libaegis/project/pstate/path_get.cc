//
//      aegis - project change supervisor
//      Copyright (C) 2006, 2008, 2011, 2012 Peter Miller
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

#include <common/error.h>
#include <common/trace.h>
#include <libaegis/project.h>


string_ty *
project::pstate_path_get()
{
    trace(("project::pstate_path_get(this = %p)\n{\n", this));
    if (parent)
        this_is_a_bug();
    if (!pstate_path)
    {
        pstate_path = str_format("%s/state", info_path_get()->str_text);
    }
    trace(("return \"%s\";\n", pstate_path->str_text));
    trace(("}\n"));
    return pstate_path;
}


// vim: set ts=8 sw=4 et :

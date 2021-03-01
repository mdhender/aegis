//
//      aegis - project change supervisor
//      Copyright (C) 2002-2008, 2012 Peter Miller
//      Copyright (C) 2008 Walter Franzini
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

#include <common/trace.h>
#include <libaegis/change.h>


string_ty *
change::creator_name()
{
    string_ty       *who;

    trace(("change_creator_name(this = %p)\n{\n", this));
    cstate_get();
    assert(cstate_data->history);
    who = cstate_data->history->list[0]->who;
    trace(("return \"%s\";\n", who ? who->str_text : ""));
    trace(("}\n"));
    return who;
}


// vim: set ts=8 sw=4 et :

//
//      aegis - project change supervisor
//      Copyright (C) 1999, 2003-2008, 2012 Peter Miller
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
#include <libaegis/change/architecture/find_variant.h>
#include <libaegis/uname.h>


cstate_architecture_times_ty *
change_find_architecture_variant(change::pointer cp)
{
    string_ty *variant = change_architecture_name(cp, 1);
    return change_find_architecture_variant(cp, variant);
}


cstate_architecture_times_ty *
change_find_architecture_variant(change::pointer cp, string_ty *an)
{
    //
    // find the name of the architecture variant
    //  one of the patterns, not the actual value in architecture
    //
    trace(("change_find_architecture_variant(cp = %p, an = \"%s\")\n{\n",
        cp, an->str_text));
    assert(cp->reference_count >= 1);

    //
    // find this variant in the times list
    //
    cstate_architecture_times_ty *tp = change_architecture_times_find(cp, an);

    //
    // adjust the node
    //
    str_free(tp->node);
    tp->node = str_copy(uname_node_get());
    trace(("return %p;\n", tp));
    trace(("}\n"));
    return tp;
}


// vim: set ts=8 sw=4 et :

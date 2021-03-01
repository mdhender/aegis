//
// aegis - project change supervisor
// Copyright (C) 1991-1995, 1998, 1999, 2001-2008, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/trace.h>

#include <libaegis/meta_context/sem.h>
#include <libaegis/meta_parse.h>


void *
parse(string_ty *filename, meta_type *type)
{
    trace(("parse(filename = \"%s\", type = %p)\n{\n", filename->str_text,
        type));
    void *addr = meta_context_sem().parse(nstring(filename), type);
    trace(("return %p;\n", addr));
    trace(("}\n"));
    return addr;
}


void *
parse_env(const char *name, meta_type *type)
{
    trace(("parse_env(name = \"%s\", type = %p)\n{\n", name, type));
    void *addr = meta_context_sem().parse_env(name, type);
    trace(("return %p;\n", addr));
    trace(("}\n"));
    return addr;
}


void *
parse_input(input &ifp, meta_type *type)
{
    trace(("parse_input(ifp = *%p, type = %p)\n{\n", &ifp,
        type));
    void *addr = meta_context_sem().parse_input(ifp, type);
    trace(("return %p;\n", addr));
    trace(("}\n"));
    return addr;
}


// vim: set ts=8 sw=4 et :

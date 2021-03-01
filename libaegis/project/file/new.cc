//
//      aegis - project change supervisor
//      Copyright (C) 1999, 2002-2008, 2011, 2012 Peter Miller
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
#include <libaegis/project/file.h>
#include <common/trace.h>


fstate_src_ty *
project::file_new(string_ty *file_name)
{
    trace(("project::file_new(this = %p, file_name = \"%s\")\n{\n",
        this, file_name->str_text));
    fstate_src_ty *src_data = change_get()->file_new(file_name);
    trace(("return %p;\n", src_data));
    trace(("}\n"));
    return src_data;
}


fstate_src_ty *
project::file_new(fstate_src_ty *meta)
{
    trace(("project::file_new(this = %p, meta->file_name = \"%s\")\n{\n",
        this, meta->file_name->str_text));
    fstate_src_ty *src_data = change_get()->file_new(meta);
    trace(("return %p;\n", src_data));
    trace(("}\n"));
    return src_data;
}


// vim: set ts=8 sw=4 et :

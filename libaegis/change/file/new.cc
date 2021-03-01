//
// aegis - project change supervisor
// Copyright (C) 1999, 2002-2008, 2011, 2012 Peter Miller
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

#include <common/ac/assert.h>

#include <common/symtab.h>
#include <common/trace.h>
#include <libaegis/change/file.h>


fstate_src_ty *
change::file_new(string_ty *file_name)
{
    trace(("change::file_new(this = %p)\n{\n", this));
    fstate_ty *fsd = fstate_get();
    assert(fsd->src);
    meta_type *type_p = 0;
    fstate_src_ty **src_data_p =
        (fstate_src_ty **)
        fstate_src_list_type.list_parse(fsd->src, &type_p);
    assert(type_p == &fstate_src_type);
    fstate_src_ty *src_data = (fstate_src_ty *)fstate_src_type.alloc();
    *src_data_p = src_data;
    src_data->file_name = str_copy(file_name);
    assert(fstate_stp);
    fstate_stp->assign(src_data->file_name, src_data);
    trace(("return %p;\n", src_data));
    trace(("}\n"));
    return src_data;
}


fstate_src_ty *
change::file_new(fstate_src_ty *meta)
{
    trace(("change::file_new(this = %p, meta = %p)\n{\n", this, meta));
    fstate_src_ty *src_data = file_new(meta->file_name);
    change_file_copy_basic_attributes(src_data, meta);
    trace(("return %p;\n", src_data));
    trace(("}\n"));
    return src_data;
}


// vim: set ts=8 sw=4 et :

//
// aegis - project change supervisor
// Copyright (C) 2008 Peter Miller
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
// You should have received a copy of the GNU General Public License along
// with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <fmtgen/type/real/include.h>


type_real_include::~type_real_include()
{
}


type_real_include::type_real_include(generator *a_gen) :
    type_real(a_gen)
{
}


type::pointer
type_real_include::create(generator *a_gen)
{
    return pointer(new type_real_include(a_gen));
}


void
type_real_include::gen_declarator(const nstring &variable_name, bool is_a_list,
    int, const nstring &) const
{
    const char *deref = (is_a_list ? "*" : "");
    printf("%s %s%s;\n", "double", deref, variable_name.c_str());
}

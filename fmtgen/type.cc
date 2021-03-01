//
//	aegis - project change supervisor
//	Copyright (C) 1991-1994, 1998, 2002-2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <common/error.h>
#include <common/mem.h>
#include <common/trace.h>
#include <fmtgen/type.h>


type::~type()
{
}


type::type(const nstring &a_name) :
    name(a_name),
    is_a_typedef(false),
    included_flag(false)
{
    assert(!name.empty());
    trace_string(name.c_str());
}


void
type::gen_include_declarator(const nstring &, bool)
    const
{
}


void
type::gen_include()
    const
{
}


void
type::gen_code_declarator(const nstring &, bool, int)
    const
{
}


void
type::gen_code_call_xml(const nstring &, const nstring &, int)
    const
{
}


void
type::gen_code()
    const
{
}


void
type::gen_free_declarator(const nstring &, bool)
    const
{
}


void
type::member_add(const nstring &, type *, int)
{
    //
    // This should only be called for structure members, and
    // type_structure_ty overloads it.  Any other call is wrong.
    //
    assert(0);
}


void
type::in_include_file()
{
    included_flag = true;
}


void
type::toplevel()
{
}


void
type::gen_code_copy(const nstring &)
    const
{
}

//
//	aegis - project change supervisor
//	Copyright (C) 1991-1994, 1998, 2002-2004 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to manipulate data types
//

#pragma implementation "type_ty"

#include <error.h>
#include <mem.h>
#include <trace.h>
#include <type.h>


type_ty::~type_ty()
{
}


type_ty::type_ty(const nstring &a_name) :
    name(a_name),
    is_a_typedef(false),
    included_flag(false)
{
    assert(!name.empty());
    trace_string(name.c_str());
}


void
type_ty::gen_include_declarator(const nstring &a_name, bool is_a_list)
    const
{
}


void
type_ty::gen_include()
    const
{
}


void
type_ty::gen_code_declarator(const nstring &a_name, bool is_a_list,
    int attributes) const
{
}


void
type_ty::gen_code_call_xml(const nstring &form_name, const nstring &member_name,
    int show) const
{
}


void
type_ty::gen_code()
    const
{
}


void
type_ty::gen_free_declarator(const nstring &a_name, bool is_a_list)
    const
{
}


void
type_ty::member_add(const nstring &member_name, type_ty *member_type,
    int attributes)
{
    //
    // This should only be called for structure members, and
    // type_structure_ty overloads it.  Any other call is wrong.
    //
    assert(0);
}


void
type_ty::in_include_file()
{
    included_flag = true;
}


void
type_ty::toplevel()
{
}

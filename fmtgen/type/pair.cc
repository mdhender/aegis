//
// aegis - project change supervisor
// Copyright (C) 2008, 2012 Peter Miller
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

#include <common/ac/assert.h>

#include <fmtgen/type/pair.h>
#include <fmtgen/type/vector.h>


type_pair::~type_pair()
{
}


type_pair::type_pair(generator *ag, const pointer &a_t1, const pointer &a_t2) :
    type(ag, a_t1->def_name(), false),
    t1(a_t1),
    t2(a_t2)
{
}


type_pair::pointer
type_pair::create(generator *a_gen, const pointer &a_t1, const pointer &a_t2)
{
    return pointer(new type_pair(a_gen, a_t1, a_t2));
}


void
type_pair::gen_body()
    const
{
    t1->gen_body();
    t2->gen_body();
}

void
type_pair::gen_declarator(const nstring &a_name, bool is_a_list, int attributes,
    const nstring &cmnt) const
{
    t1->gen_declarator(a_name, is_a_list, attributes, cmnt);
    t2->gen_declarator(a_name, is_a_list, attributes, cmnt);
}


void
type_pair::gen_call_xml(const nstring &form_name,
    const nstring &member_name, int show) const
{
    t1->gen_call_xml(form_name, member_name, show);
    t2->gen_call_xml(form_name, member_name, show);
}


void
type_pair::gen_copy(const nstring &member_name)
    const
{
    t1->gen_copy(member_name);
    t2->gen_copy(member_name);
}


void
type_pair::gen_trace(const nstring &aname, const nstring &value)
    const
{
    t1->gen_trace(aname, value);
    t2->gen_trace(aname, value);
}


void
type_pair::gen_free_declarator(const nstring &aname, bool is_a_list)
    const
{
    t1->gen_free_declarator(aname, is_a_list);
    t2->gen_free_declarator(aname, is_a_list);
}


void
type_pair::gen_report_initializations()
{
    t1->gen_report_initializations();
    t2->gen_report_initializations();
}


void
type_pair::member_add(const nstring &member_name,
    const type::pointer &member_type, int attributes, const nstring &cmnt)
{
    if (member_type)
    {
        type_pair *pp = dynamic_cast<type_pair *>(member_type.get());
        assert(pp);
        t1->member_add(member_name, pp->get_t1(), attributes, cmnt);
        t2->member_add(member_name, pp->get_t2(), attributes, cmnt);
    }
    else
    {
        // enum members don't have types
        assert(!member_type);
        t1->member_add(member_name, member_type, attributes, cmnt);
        t2->member_add(member_name, member_type, attributes, cmnt);
    }
}


void
type_pair::toplevel()
{
    t1->toplevel();
    t2->toplevel();
}


void
type_pair::in_include_file()
{
    type::in_include_file();
    t1->in_include_file();
    t2->in_include_file();
}


nstring
type_pair::c_name_inner()
    const
{
    return t1->c_name();
}


bool
type_pair::has_a_mask()
    const
{
    assert(t1->has_a_mask() == t2->has_a_mask());
    return t1->has_a_mask();
}


void
type_pair::get_reachable(type_vector &results)
    const
{
    type_vector r1;
    t1->get_reachable(r1);
    type_vector r2;
    t2->get_reachable(r2);
    assert(r1.size() == r2.size());
    for (size_t j = 0; j < r1.size(); ++j)
        results.push_back(create(gen, r1[j], r2[j]));
}


// vim: set ts=8 sw=4 et :

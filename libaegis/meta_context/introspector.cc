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

#include <libaegis/meta_context/introspector.h>
#include <libaegis/sem.h>


meta_context_introspector::~meta_context_introspector()
{
}


meta_context_introspector::meta_context_introspector()
{
}


void
meta_context_introspector::parse_file(const nstring &filename,
    const introspector::pointer &ip)
{
    stack.push_back(ip);
    sem_parse_file(*this, filename);
    assert(stack.empty());
    stack.clear();
}


void
meta_context_introspector::integer(long n)
{
    assert(!stack.empty());
    stack.back()->integer(n);
}


void
meta_context_introspector::real(double n)
{
    assert(!stack.empty());
    stack.back()->real(n);
}


void
meta_context_introspector::string(const nstring &s)
{
    assert(!stack.empty());
    stack.back()->string(s);
}


void
meta_context_introspector::enumeration(const nstring &name)
{
    assert(!stack.empty());
    stack.back()->enumeration(name);
}


void
meta_context_introspector::list()
{
    assert(!stack.empty());
    stack.push_back(stack.back()->list());
}


void
meta_context_introspector::list_end()
{
    assert(!stack.empty());
    stack.pop_back();
}


void
meta_context_introspector::field(const nstring &name)
{
    assert(!stack.empty());
    stack.push_back(stack.back()->field(name));
}


void
meta_context_introspector::field_end()
{
    assert(!stack.empty());
    stack.pop_back();
}


void
meta_context_introspector::end()
{
    assert(!stack.empty());
    stack.pop_back();
}


// vim: set ts=8 sw=4 et :

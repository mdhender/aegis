//
// aegis - project change supervisor
// Copyright (C) 2004-2006, 2008, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published
// by the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
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

#include <common/mem.h>
#include <libaegis/file_revision.h>
#include <libaegis/os.h>


file_revision::~file_revision()
{
    assert(ref);
    ref->one_fewer();
    ref = 0;
}


void
file_revision::inner::one_fewer()
{
    --reference_count;
    if (reference_count <= 0)
        delete this;
}


file_revision::inner::~inner()
{
    if (need_to_unlink)
    {
        need_to_unlink = false;
        os_become_orig();
        os_unlink(filename.get_ref());
        os_become_undo();
    }
}


file_revision::file_revision(const nstring &arg1, bool arg2) :
    ref(new inner(arg1, arg2))
{
    assert(ref);
}


file_revision::inner::inner(const nstring &arg1, bool arg2) :
    reference_count(1),
    filename(arg1),
    need_to_unlink(arg2)
{
}


file_revision::file_revision(const file_revision &arg) :
    ref(arg.ref)
{
    ref->one_more();
}


file_revision &
file_revision::operator=(const file_revision &arg)
{
    if (this != &arg)
    {
        ref->one_fewer();
        ref = arg.ref;
        ref->one_more();
    }
    return *this;
}


// vim: set ts=8 sw=4 et :

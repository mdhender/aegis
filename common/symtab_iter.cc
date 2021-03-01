//
// aegis - project change supervisor
// Copyright (C) 2001, 2003-2008, 2012 Peter Miller
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

#include <common/symtab_iter.h>


symtab_iterator::symtab_iterator(const symtab_ty *arg) :
    stp(arg),
    rp(0),
    pos(0)
{
    assert(stp);
}


symtab_iterator::~symtab_iterator()
{
    stp = 0;
    pos = 0;
    rp = 0;
}


void
symtab_iterator::reset()
{
    rp = 0;
    pos = 0;
}


bool
symtab_iterator::next(string_ty **key_p, void **data_p)
{
    assert(key_p);
    assert(data_p);
    nstring key;
    void *data = 0;
    assert(stp);
    if (!next(key, data))
        return false;
    *key_p = key.get_ref();
    *data_p = data;
    return true;
}


bool
symtab_iterator::next(nstring &key, void *&data)
{
    assert(stp);
    while (rp == 0)
    {
        if (pos >= stp->hash_modulus)
            return false;
        rp = stp->hash_table[pos];
        pos++;
    }
    key = rp->key;
    assert(rp->data);
    data = rp->data;
    rp = rp->overflow;
    return true;
}


// vim: set ts=8 sw=4 et :

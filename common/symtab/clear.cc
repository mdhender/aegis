//
// aegis - project change supervisor
// Copyright (C) 2004-2008, 2012 Peter Miller
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

#include <common/symtab.h>


void
symtab_ty::clear()
{
    for (str_hash_ty j = 0; j < hash_modulus; ++j)
    {
        row_t **rpp = &hash_table[j];
        while (*rpp)
        {
            row_t *rp = *rpp;
            assert(rp->data);
            *rpp = rp->overflow;
            if (reap)
                reap(rp->data);
            delete rp;
        }
    }
    hash_load = 0;
}


// vim: set ts=8 sw=4 et :

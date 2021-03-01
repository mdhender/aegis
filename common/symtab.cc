//
//      aegis - project change supervisor
//      Copyright (C) 1990-1995, 2002-2006, 2008, 2012 Peter Miller
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

#include <common/mem.h>
#include <common/symtab.h>
#include <common/trace.h>


symtab_ty::symtab_ty(int suggested_size) :
    reap(0),
    hash_table(0),
    hash_modulus(0),
    hash_mask(0),
    hash_load(0)
{
    trace(("symtab(%d)\n{\n", suggested_size));
    trace(("this = %p\n", this));
    hash_modulus = 1 << 5; // MUST be a power of 2
    while ((long)hash_modulus < (long)suggested_size)
        hash_modulus <<= 1;
    hash_mask = hash_modulus - 1;
    hash_table = new row_t * [hash_modulus];
    for (str_hash_ty j = 0; j < hash_modulus; ++j)
        hash_table[j] = 0;
    trace(("}\n"));
}


symtab_ty::~symtab_ty()
{
    trace(("~symtab()\n{\n"));
    trace(("this = %p\n", this));
    clear();

    reap = 0;
    delete [] hash_table;
    hash_table = 0;
    hash_modulus = 0;
    hash_mask = 0;
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :

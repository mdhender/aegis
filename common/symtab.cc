//
//	aegis - project change supervisor
//	Copyright (C) 1990-1995, 2002-2005 Peter Miller;
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
// MANIFEST: functions to manipulate symbol tables
//

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
    trace(("this = %08lX\n", (long)this));
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
    trace(("this = %08lX\n", (long)this));
    clear();

    reap = 0;
    delete [] hash_table;
    hash_table = 0;
    hash_modulus = 0;
    hash_mask = 0;
    trace(("}\n"));
}

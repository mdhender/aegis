//
//	aegis - project change supervisor
//	Copyright (C) 2004-2008 Peter Miller
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

#include <common/mem.h>
#include <common/symtab.h>


void
symtab_ty::assign_push(string_ty *key, void *data)
{
    assign_push(nstring(key), data);
}


void
symtab_ty::assign_push(const nstring &key, void *data)
{
    str_hash_ty idx = key.get_hash() & hash_mask;
    row_t *p = new row_t;
    p->key = key;
    p->overflow = hash_table[idx];
    p->data = data;
    hash_table[idx] = p;

    hash_load++;
    if (hash_load * 10 >= hash_modulus * 8)
	split();
}

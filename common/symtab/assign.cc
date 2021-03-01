//
//	aegis - project change supervisor
//	Copyright (C) 2004, 2005 Peter Miller;
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
// MANIFEST: implementation of the symtab_assign class
//

#include <common/symtab.h>


void
symtab_ty::assign(string_ty *key, void *data)
{
    str_hash_ty index = key->str_hash & hash_mask;
    row_t *p = 0;
    for (p = hash_table[index]; p; p = p->overflow)
    {
	if (str_equal(key, p->key))
	{
    	    if (reap)
       		reap(p->data);
    	    p->data = data;
    	    return;
	}
    }

    p = new row_t;
    p->key = str_copy(key);
    p->overflow = hash_table[index];
    p->data = data;
    hash_table[index] = p;

    hash_load++;
    if (hash_load * 10 >= hash_modulus * 8)
	split();
}

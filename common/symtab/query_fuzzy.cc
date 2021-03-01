//
//	aegis - project change supervisor
//	Copyright (C) 2004 Peter Miller;
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
// MANIFEST: implementation of the symtab_query_fuzzy class
//

#include <fstrcmp.h>
#include <symtab.h>


string_ty *
symtab_ty::query_fuzzy(string_ty *key)
    const
{
    string_ty *best_name = 0;
    double best_weight = 0.6;
    for (str_hash_ty index = 0; index < hash_modulus; ++index)
    {
	for (row_t *p = hash_table[index]; p; p = p->overflow)
	{
	    double weight = fstrcmp(key->str_text, p->key->str_text);
	    if (weight > best_weight)
		best_name = p->key;
	}
    }
    return best_name;
}

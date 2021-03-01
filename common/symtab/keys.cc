//
//	aegis - project change supervisor
//	Copyright (C) 2002, 2004 Peter Miller;
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
// MANIFEST: functions to manipulate keyss
//

#include <symtab/keys.h>


void
symtab_keys(symtab_ty *stp, string_list_ty *keys)
{
    size_t	    j;
    symtab_row_ty   *p;

    string_list_constructor(keys);
    for (j = 0; j < stp->hash_modulus; ++j)
       	for (p = stp->hash_table[j]; p; p = p->overflow)
	    string_list_append(keys, p->key);
}

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

#include <common/error.h> // for assert
#include <common/symtab.h>


void *
symtab_ty::query(string_ty *key)
    const
{
    return query(nstring(key));
}


void *
symtab_ty::query(const nstring &key)
    const
{
    str_hash_ty idx = key.get_hash() & hash_mask;
    for (row_t *p = hash_table[idx]; p; p = p->overflow)
    {
	if (key == p->key)
	    return p->data;
    }
    return 0;
}

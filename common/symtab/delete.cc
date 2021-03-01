//
//	aegis - project change supervisor
//	Copyright (C) 2002-2008 Peter Miller
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

#include <common/symtab.h>


void
symtab_ty::remove(string_ty *key)
{
    remove(nstring(key));
}


void
symtab_ty::remove(const nstring &key)
{
    str_hash_ty idx = key.get_hash() & hash_mask;
    row_t **pp = &hash_table[idx];
    for (;;)
    {
	row_t *p = *pp;
	if (!p)
	    break;
	if (key == p->key)
	{
	    if (reap)
	       	reap(p->data);
	    *pp = p->overflow;
	    delete p;
	    hash_load--;
	    break;
	}
	pp = &p->overflow;
    }
}

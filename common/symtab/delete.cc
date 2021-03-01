//
//	aegis - project change supervisor
//	Copyright (C) 2002-2005 Peter Miller;
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
// MANIFEST: functions to manipulate deletes
//

#include <common/symtab.h>


void
symtab_ty::remove(string_ty *key)
{
    str_hash_ty index = key->str_hash & hash_mask;
    row_t **pp = &hash_table[index];
    for (;;)
    {
	row_t *p = *pp;
	if (!p)
	    break;
	if (str_equal(key, p->key))
	{
	    if (reap)
	       	reap(p->data);
	    str_free(p->key);
	    *pp = p->overflow;
	    delete p;
	    hash_load--;
	    break;
	}
	pp = &p->overflow;
    }
}

/*
 *	aegis - project change supervisor
 *	Copyright (C) 2002, 2003 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate deletes
 */

#include <mem.h>
#include <symtab.h>


/*
 * NAME
 *	symtab_delete - delete a variable
 *
 * SYNOPSIS
 *	void symtab_delete(string_ty *name, symtab_class_ty class);
 *
 * DESCRIPTION
 *	The symtab_delete function is used to delete variables.
 *
 * CAVEAT
 *	The name is freed, the data is reaped.
 *	(By default, reap does nothing.)
 */

void
symtab_delete(symtab_ty *stp, string_ty *key)
{
    str_hash_ty     index;
    symtab_row_ty   **pp;

    index = key->str_hash & stp->hash_mask;

    pp = &stp->hash_table[index];
    for (;;)
    {
	symtab_row_ty	*p;

	p = *pp;
	if (!p)
	    break;
	if (str_equal(key, p->key))
	{
	    if (stp->reap)
	       	stp->reap(p->data);
	    str_free(p->key);
	    *pp = p->overflow;
	    mem_free(p);
	    stp->hash_load--;
	    break;
	}
	pp = &p->overflow;
    }
}

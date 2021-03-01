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
// MANIFEST: implementation of the symtab_valid class
//

#include <symtab.h>


bool
symtab_ty::valid()
    const
{
    if (hash_table == 0)
	return false;
    if (hash_modulus == 0)
	return false;
    // the hash_modulus is required to be a power of two
    if ((hash_modulus & (hash_modulus - 1)) != 0)
	return false;
    if (hash_mask + 1 != hash_modulus)
	return false;
    if (hash_load >= hash_modulus)
	return false;
    str_hash_ty count = 0;
    for (str_hash_ty j = 0; j < hash_modulus; ++j)
    {
	for (row_t *rp = hash_table[j]; rp; rp = rp->overflow)
	{
	    if (rp->data == 0)
		return false;
	    if (!str_validate(rp->key))
		return false;
	    ++count;
	}
    }
    return (count == hash_load);
}

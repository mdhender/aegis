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

#include <common/symtab.h>


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
	    if (!rp->key.valid())
		return false;
	    ++count;
	}
    }
    return (count == hash_load);
}

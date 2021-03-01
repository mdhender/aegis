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
#include <common/mem.h>
#include <common/symtab.h>


void
symtab_ty::split()
{
    //
    // double the modulus
    //
    // This is subtle.  If we only increase the modulus by one, the
    // load always hovers around 80%, so we have to do a split for
    // every insert.  I.e. the malloc burden os O(n) for the lifetime of
    // the symtab.  BUT if we double the modulus, the length of time
    // until the next split also doubles, making the probablity of a
    // split halve, and sigma(2**-n)=1, so the malloc burden becomes O(1)
    // for the lifetime of the symtab.
    //
    str_hash_ty new_hash_modulus = hash_modulus * 2;
    str_hash_ty new_hash_mask = new_hash_modulus - 1;
    row_t **new_hash_table = new row_t * [new_hash_modulus];

    //
    // now redistribute the list elements
    //
    for (str_hash_ty idx = 0; idx < hash_modulus; ++idx)
    {
	new_hash_table[idx] = 0;
	new_hash_table[idx + hash_modulus] = 0;

	row_t *p = hash_table[idx];
	while (p)
	{
	    row_t *p2 = p;
	    p = p2->overflow;
	    p2->overflow = 0;

	    //
	    // It is important to preserve the order of the links because
	    // they can be push-down stacks, and to simply add them to the
	    // head of the list will reverse the order of the stack!
	    //
	    assert((p2->key.get_hash() & hash_mask) == idx);
	    str_hash_ty idx2 = p2->key.get_hash() & new_hash_mask;
	    row_t **ipp = &new_hash_table[idx2];
	    for (; *ipp; ipp = &(*ipp)->overflow)
		;
	    *ipp = p2;
	}
    }
    delete [] hash_table;
    hash_table = new_hash_table;
    hash_modulus = new_hash_modulus;
    hash_mask = new_hash_mask;
}

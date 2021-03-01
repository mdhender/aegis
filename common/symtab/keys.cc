//
//	aegis - project change supervisor
//	Copyright (C) 2002, 2004-2008 Peter Miller
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
#include <common/str_list.h>
#include <common/nstring/list.h>


void
symtab_ty::keys(string_list_ty *result)
    const
{
    result->clear();
    for (str_hash_ty j = 0; j < hash_modulus; ++j)
       	for (row_t *p = hash_table[j]; p; p = p->overflow)
	    result->push_back(p->key.get_ref());
}


void
symtab_ty::keys(nstring_list &result)
    const
{
    result.clear();
    for (str_hash_ty j = 0; j < hash_modulus; ++j)
       	for (row_t *p = hash_table[j]; p; p = p->overflow)
	    result.push_back(p->key);
}

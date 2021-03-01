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

#include <common/fstrcmp.h>
#include <common/symtab.h>


string_ty *
symtab_ty::query_fuzzy(string_ty *key)
    const
{
    nstring result = query_fuzzy(nstring(key));
    return (result.empty() ? 0 : result.get_ref());
}


nstring
symtab_ty::query_fuzzy(const nstring &key)
    const
{
    nstring best_name;
    double best_weight = 0.6;
    for (str_hash_ty idx = 0; idx < hash_modulus; ++idx)
    {
	for (row_t *p = hash_table[idx]; p; p = p->overflow)
	{
	    double weight = fstrcmp(key.c_str(), p->key.c_str());
	    if (weight > best_weight)
            {
		best_name = p->key;
                best_weight = weight;
            }
	}
    }
    return best_name;
}

/*
 *	aegis - project change supervisor
 *	Copyright (C) 1990-1995, 2002, 2003 Peter Miller;
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
 * MANIFEST: functions to manipulate symbol tables
 */

#include <error.h>
#include <fstrcmp.h>
#include <mem.h>
#include <symtab.h>


symtab_ty *
symtab_alloc(int size)
{
    symtab_ty	*stp;
    str_hash_ty	j;

    stp = (symtab_ty *)mem_alloc(sizeof(symtab_ty));
    stp->chain = 0;
    stp->reap = 0;
    stp->hash_modulus = 1 << 5; /* MUST be a power of 2 */
    stp->hash_mask = stp->hash_modulus - 1;
    stp->hash_load = 0;
    stp->hash_table = (symtab_row_ty **)mem_alloc(
        stp->hash_modulus * sizeof(symtab_row_ty *));
    for (j = 0; j < stp->hash_modulus; ++j)
	stp->hash_table[j] = 0;
    return stp;
}


void
symtab_free(symtab_ty *stp)
{
    str_hash_ty     j;

    for (j = 0; j < stp->hash_modulus; ++j)
    {
	symtab_row_ty	**rpp;

	rpp = &stp->hash_table[j];
	while (*rpp)
	{
	    symtab_row_ty   *rp;

	    rp = *rpp;
	    *rpp = rp->overflow;
	    if (stp->reap)
		stp->reap(rp->data);
	    str_free(rp->key);
	    mem_free(rp);
	}
    }
}


/*
 * NAME
 *	split - reduce symbol table load
 *
 * SYNOPSIS
 *	void split(symtab_ty);
 *
 * DESCRIPTION
 *	The split function is used to split symbols in the bucket indicated by
 *	the split point.  The symbols are split between that bucket and the one
 *	after the current end of the table.
 *
 * CAVEAT
 *	It is only sensable to do this when the symbol table load exceeds some
 *	reasonable threshold.  A threshold of 80% is suggested.
 */

static void split(symtab_ty *);

static void
split(symtab_ty *stp)
{
    str_hash_ty     new_hash_modulus;
    str_hash_ty     new_hash_mask;
    symtab_row_ty   **new_hash_table;
    str_hash_ty     idx;

    /*
     * double the modulus
     *
     * This is subtle.  If we only increase the modulus by one, the
     * load always hovers around 80%, so we have to do a split for
     * every insert.  I.e. the malloc burden os O(n) for the lifetime of
     * the symtab.  BUT if we double the modulus, the length of time
     * until the next split also doubles, making the probablity of a
     * split halve, and sigma(2**-n)=1, so the malloc burden becomes O(1)
     * for the lifetime of the symtab.
     */
    new_hash_modulus = stp->hash_modulus * 2;
    new_hash_mask = new_hash_modulus - 1;
    new_hash_table =
        (symtab_row_ty **)mem_alloc(new_hash_modulus * sizeof(symtab_row_ty *));

    /*
     * now redistribute the list elements
     */
    for (idx = 0; idx < stp->hash_modulus; ++idx)
    {
	symtab_row_ty   *p;

	new_hash_table[idx] = 0;
	new_hash_table[idx + stp->hash_modulus] = 0;

	p = stp->hash_table[idx];
	while (p)
	{
	    str_hash_ty     index;
	    symtab_row_ty   **ipp;
	    symtab_row_ty   *p2;

	    p2 = p;
	    p = p2->overflow;
	    p2->overflow = 0;

	    /*
	     * It is important to preserve the order of the links because
	     * they can be push-down stacks, and to simply add them to the
	     * head of the list will reverse the order of the stack!
	     */
	    assert((p2->key->str_hash & stp->hash_mask) == idx);
	    index = p2->key->str_hash & new_hash_mask;
	    for (ipp = &new_hash_table[index]; *ipp; ipp = &(*ipp)->overflow)
		;
	    *ipp = p2;
	}
    }
    mem_free(stp->hash_table);
    stp->hash_table = new_hash_table;
    stp->hash_modulus = new_hash_modulus;
    stp->hash_mask = new_hash_mask;
}


/*
 * NAME
 *	symtab_query - search for a variable
 *
 * SYNOPSIS
 *	int symtab_query(symtab_ty *, string_ty *key);
 *
 * DESCRIPTION
 *	The symtab_query function is used to reference a variable.
 *
 * RETURNS
 *	If the variable has been defined, the function returns a non-zero value
 *	and the value is returned through the 'value' pointer.
 *	If the variable has not been defined, it returns zero,
 *	and 'value' is unaltered.
 */

void *
symtab_query(symtab_ty *stp, string_ty *key)
{
    while (stp)
    {
	str_hash_ty     index;
	symtab_row_ty   *p;

	index = key->str_hash & stp->hash_mask;
	for (p = stp->hash_table[index]; p; p = p->overflow)
	{
	    if (str_equal(key, p->key))
	       	return p->data;
	}
	stp = stp->chain;
    }
    return 0;
}


/*
 * NAME
 *	symtab_query_fuzzy - search for a variable name
 *
 * SYNOPSIS
 *	string_ty *symtab_query_fuzzy(symtab_ty *, string_ty *key);
 *
 * DESCRIPTION
 *	The symtab_query_fuzzy function is used to search for a variable name.
 *
 * RETURNS
 *	The closest match for the variable name given.
 *	If no match is particularly close, it returns 0.
 */

string_ty *
symtab_query_fuzzy(symtab_ty *stp, string_ty *key)
{
    string_ty	    *best_name;
    double	    best_weight;

    best_name = 0;
    best_weight = 0.6;
    while (stp)
    {
	str_hash_ty     index;

	for (index = 0; index < stp->hash_modulus; ++index)
	{
	    symtab_row_ty   *p;

	    for (p = stp->hash_table[index]; p; p = p->overflow)
	    {
		double          weight;

		weight = fstrcmp(key->str_text, p->key->str_text);
		if (weight > best_weight)
		    best_name = p->key;
	    }
	}
	stp = stp->chain;
    }
    return best_name;
}


/*
 * NAME
 *	symtab_assign - assign a variable
 *
 * SYNOPSIS
 *	void symtab_assign(symtab_ty *, string_ty *key, void *data);
 *
 * DESCRIPTION
 *	The symtab_assign function is used to assign
 *	a value to a given variable.
 *
 * CAVEAT
 *	The name is copied, the data is not.
 */

void
symtab_assign(symtab_ty *stp, string_ty *key, void *data)
{
    str_hash_ty     index;
    symtab_row_ty   *p;

    index = key->str_hash & stp->hash_mask;

    for (p = stp->hash_table[index]; p; p = p->overflow)
    {
	if (str_equal(key, p->key))
	{
    	    if (stp->reap)
       		stp->reap(p->data);
    	    p->data = data;
    	    return;
	}
    }

    p = (symtab_row_ty *)mem_alloc(sizeof(symtab_row_ty));
    p->key = str_copy(key);
    p->overflow = stp->hash_table[index];
    p->data = data;
    stp->hash_table[index] = p;

    stp->hash_load++;
    if (stp->hash_load * 10 >= stp->hash_modulus * 8)
	split(stp);
}


/*
 * NAME
 *	symtab_assign_push - assign a variable
 *
 * SYNOPSIS
 *	void symtab_assign_push(symtab_ty *, string_ty *key, void *data);
 *
 * DESCRIPTION
 *	The symtab_assign function is used to assign
 *	a value to a given variable.
 *	Any previous value will be obscured until this one
 *	is deleted with symtab_delete.
 *
 * CAVEAT
 *	The name is copied, the data is not.
 */

void
symtab_assign_push(symtab_ty *stp, string_ty *key, void *data)
{
    str_hash_ty     index;
    symtab_row_ty   *p;

    index = key->str_hash & stp->hash_mask;

    p = (symtab_row_ty *)mem_alloc(sizeof(symtab_row_ty));
    p->key = str_copy(key);
    p->overflow = stp->hash_table[index];
    p->data = data;
    stp->hash_table[index] = p;

    stp->hash_load++;
    if (stp->hash_load * 10 >= stp->hash_modulus * 8)
	split(stp);
}

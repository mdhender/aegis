/*
 *	aegis - project change supervisor
 *	Copyright (C) 1990-1995, 2002 Peter Miller;
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

    stp = mem_alloc(sizeof(symtab_ty));
    stp->chain = 0;
    stp->reap = 0;
    stp->hash_modulus = 1 << 2; /* MUST be a power of 2 */
    while (stp->hash_modulus < size)
	stp->hash_modulus <<= 1;
    stp->hash_cutover = stp->hash_modulus;
    stp->hash_split = stp->hash_modulus - stp->hash_cutover;
    stp->hash_cutover_mask = stp->hash_cutover - 1;
    stp->hash_cutover_split_mask = (stp->hash_cutover * 2) - 1;
    stp->hash_load = 0;
    stp->hash_table =
	mem_alloc(stp->hash_modulus * sizeof(symtab_row_ty *));
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
    symtab_row_ty   *p;
    symtab_row_ty   **ipp;
    symtab_row_ty   *p2;
    str_hash_ty     index;

    /*
     * get the list to be split across buckets
     */
    p = stp->hash_table[stp->hash_split];
    stp->hash_table[stp->hash_split] = 0;

    /*
     * increase the modulus by one
     */
    stp->hash_modulus++;
    stp->hash_table =
	mem_change_size
	(
    	    stp->hash_table,
    	    stp->hash_modulus * sizeof(symtab_row_ty *)
	);
    stp->hash_table[stp->hash_modulus - 1] = 0;
    stp->hash_split = stp->hash_modulus - stp->hash_cutover;
    if (stp->hash_split >= stp->hash_cutover)
    {
	stp->hash_cutover = stp->hash_modulus;
	stp->hash_split = 0;
	stp->hash_cutover_mask = stp->hash_cutover - 1;
	stp->hash_cutover_split_mask = (stp->hash_cutover * 2) - 1;
    }

    /*
     * now redistribute the list elements
     *
     * It is important to preserve the order of the links because
     * they can be push-down stacks, and to simply add them to the
     * head of the list will reverse the order of the stack!
     */
    while (p)
    {
	p2 = p;
	p = p2->overflow;
	p2->overflow = 0;

	index = p2->key->str_hash & stp->hash_cutover_mask;
	if (index < stp->hash_split)
    	    index = (p2->key->str_hash & stp->hash_cutover_split_mask);
	for
	(
    	    ipp = &stp->hash_table[index];
    	    *ipp;
    	    ipp = &(*ipp)->overflow
	)
    	    ;
	*ipp = p2;
    }
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
    str_hash_ty     index;
    symtab_row_ty   *p;

    while (stp)
    {
	index = key->str_hash & stp->hash_cutover_mask;
	if (index < stp->hash_split)
	    index = key->str_hash & stp->hash_cutover_split_mask;
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
    str_hash_ty     index;
    symtab_row_ty   *p;
    string_ty	    *best_name;
    double	    best_weight;
    double	    weight;

    best_name = 0;
    best_weight = 0.6;
    while (stp)
    {
	for (index = 0; index < stp->hash_modulus; ++index)
	{
	    for (p = stp->hash_table[index]; p; p = p->overflow)
	    {
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

    index = key->str_hash & stp->hash_cutover_mask;
    if (index < stp->hash_split)
	index = key->str_hash & stp->hash_cutover_split_mask;

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

    p = mem_alloc(sizeof(symtab_row_ty));
    p->key = str_copy(key);
    p->overflow = stp->hash_table[index];
    p->data = data;
    stp->hash_table[index] = p;

    stp->hash_load++;
    while (stp->hash_load * 10 >= stp->hash_modulus * 8)
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

    index = key->str_hash & stp->hash_cutover_mask;
    if (index < stp->hash_split)
	index = key->str_hash & stp->hash_cutover_split_mask;

    p = mem_alloc(sizeof(symtab_row_ty));
    p->key = str_copy(key);
    p->overflow = stp->hash_table[index];
    p->data = data;
    stp->hash_table[index] = p;

    stp->hash_load++;
    while (stp->hash_load * 10 >= stp->hash_modulus * 8)
	split(stp);
}

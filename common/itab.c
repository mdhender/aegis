/*
 *	aegis - project change supervisor
 *	Copyright (C) 1998 Peter Miller;
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
 * MANIFEST: functions to manipulate integer indexed tables
 */

#include <itab.h>
#include <mem.h>
#include <trace.h>


/*
 * NAME
 *	itab_alloc
 *
 * SYNOPSIS
 *	itab_ty *itabLalloc(int);
 *
 * DESCRIPTION
 *	The itab_alloc function is used to allocate a new integer table
 *	instance in dynamic memory.
 *
 * RETURNS
 *	itab_ty *; pointer to table
 *
 * CAVEAT
 *	Use itab_free when you are done with it.
 */

itab_ty *
itab_alloc(size)
	int		size;
{
	itab_ty		*itp;
	itab_key_ty	j;

	trace(("itab_alloc(size = %d)\n{\n"/*}*/, size));
	itp = mem_alloc(sizeof(itab_ty));
	itp->reap = 0;
	itp->hash_modulus = 1 << 2; /* MUST be a power of 2 */
	while (itp->hash_modulus < size)
		itp->hash_modulus <<= 1;
	itp->hash_cutover = itp->hash_modulus;
	itp->hash_split = itp->hash_modulus - itp->hash_cutover;
	itp->hash_cutover_mask = itp->hash_cutover - 1;
	itp->hash_cutover_split_mask = (itp->hash_cutover * 2) - 1;
	itp->load = 0;
	itp->hash_table = mem_alloc(itp->hash_modulus * sizeof(itab_row_ty *));
	for (j = 0; j < itp->hash_modulus; ++j)
		itp->hash_table[j] = 0;
	trace(("return %08lX;\n", (long)itp));
	trace((/*{*/"}\n"));
	return itp;
}


/*
 * NAME
 *	itab_free
 *
 * SYNOPSIS
 *	void itab_free(itab_ty *);
 *
 * DESCRIPTION
 *	The itab_free function is used to release the resources held by
 *	an integer table.
 */

void
itab_free(itp)
	itab_ty		*itp;
{
	itab_key_ty	j;

	trace(("itab_free(itp = %08lX)\n{\n"/*}*/, (long)itp));
	for (j = 0; j < itp->hash_modulus; ++j)
	{
		itab_row_ty	**rpp;

		rpp = &itp->hash_table[j];
		while (*rpp)
		{
			itab_row_ty	*rp;

			rp = *rpp; 
			*rpp = rp->overflow;
			if (itp->reap)
				itp->reap(rp->data);
			mem_free(rp);
		}
	}
	mem_free(itp->hash_table);
	mem_free(itp);
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	split - reduce symbol table load
 *
 * SYNOPSIS
 *	void split(itab_ty);
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

static void split _((itab_ty *));

static void
split(itp)
	itab_ty		*itp;
{
	itab_row_ty	*p;
	itab_row_ty	**ipp;
	itab_row_ty	*p2;
	itab_key_ty	index;

	/*
	 * get the list to be split across buckets 
	 */
	trace(("split(itp = %08lX)\n{\n"/*}*/, (long)itp));
	p = itp->hash_table[itp->hash_split];
	itp->hash_table[itp->hash_split] = 0;

	/*
	 * increase the modulus by one
	 */
	itp->hash_modulus++;
	itp->hash_table =
		mem_change_size
		(
			itp->hash_table,
			itp->hash_modulus * sizeof(itab_row_ty *)
		);
	itp->hash_table[itp->hash_modulus - 1] = 0;
	itp->hash_split = itp->hash_modulus - itp->hash_cutover;
	if (itp->hash_split >= itp->hash_cutover)
	{
		itp->hash_cutover = itp->hash_modulus;
		itp->hash_split = 0;
		itp->hash_cutover_mask = itp->hash_cutover - 1;
		itp->hash_cutover_split_mask = (itp->hash_cutover * 2) - 1;
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

		index = p2->key & itp->hash_cutover_mask;
		if (index < itp->hash_split)
			index = p2->key & itp->hash_cutover_split_mask;
		for
		(
			ipp = &itp->hash_table[index];
			*ipp;
			ipp = &(*ipp)->overflow
		)
			;
		*ipp = p2;
	}
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	itab_query - search for a variable
 *
 * SYNOPSIS
 *	int itab_query(itab_ty *, string_ty *key);
 *
 * DESCRIPTION
 *	The itab_query function is used to reference a variable.
 *
 * RETURNS
 *	If the variable has been defined, the function returns a non-zero value
 *	and the value is returned through the 'value' pointer.
 *	If the variable has not been defined, it returns zero,
 *	and 'value' is unaltered.
 */

void *
itab_query(itp, key)
	itab_ty		*itp;
	itab_key_ty	key;
{
	itab_key_ty	index;
	itab_row_ty	*p;
	void		*result;

	trace(("itab_query(itp = %08lX, key = %ld)\n{\n"/*}*/,
		(long)itp, (long)key));
	result = 0;
	index = key & itp->hash_cutover_mask;
	if (index < itp->hash_split)
		index = key & itp->hash_cutover_split_mask;
	for (p = itp->hash_table[index]; p; p = p->overflow)
	{
		if (key == p->key)
		{
			result = p->data;
			break;
		}
	}
	trace(("return %08lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	itab_assign - assign a variable
 *
 * SYNOPSIS
 *	void itab_assign(itab_ty *, string_ty *key, void *data);
 *
 * DESCRIPTION
 *	The itab_assign function is used to assign
 *	a value to a given variable.
 *
 * CAVEAT
 *	The name is copied, the data is not.
 */

void
itab_assign(itp, key, data)
	itab_ty		*itp;
	itab_key_ty	key;
	void		*data;
{
	itab_key_ty	index;
	itab_row_ty	*p;

	trace(("itab_assign(itp = %08lX, key = %ld, data = %08lX)\n\
{\n"/*}*/, (long)itp, (long)key, (long)data));
	index = key & itp->hash_cutover_mask;
	if (index < itp->hash_split)
		index = key & itp->hash_cutover_split_mask;

	for (p = itp->hash_table[index]; p; p = p->overflow)
	{
		if (key == p->key)
		{
			trace(("modify existing entry\n"));
			if (itp->reap)
				itp->reap(p->data);
			p->data = data;
			goto done;
		}
	}

	trace(("new entry\n"));
	p = mem_alloc(sizeof(itab_row_ty));
	p->key = key;
	p->overflow = itp->hash_table[index];
	p->data = data;
	itp->hash_table[index] = p;

	itp->load++;
	while (itp->load * 10 >= itp->hash_modulus * 8)
		split(itp);
	done:
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	itab_delete - delete a variable
 *
 * SYNOPSIS
 *	void itab_delete(string_ty *name, itab_class_ty class);
 *
 * DESCRIPTION
 *	The itab_delete function is used to delete variables.
 *
 * CAVEAT
 *	The name is freed, the data is reaped.
 *	(By default, reap does nothing.)
 */

void
itab_delete(itp, key)
	itab_ty		*itp;
	itab_key_ty	key;
{
	itab_key_ty	index;
	itab_row_ty	**pp;

	trace(("itab_delete(itp = %08lX, key = %ld)\n{\n"/*}*/,
		(long)itp, (long)key));
	index = key & itp->hash_cutover_mask;
	if (index < itp->hash_split)
		index = key & itp->hash_cutover_split_mask;

	pp = &itp->hash_table[index];
	for (;;)
	{
		itab_row_ty	*p;

		p = *pp;
		if (!p)
			break;
		if (key == p->key)
		{
			if (itp->reap)
				itp->reap(p->data);
			*pp = p->overflow;
			mem_free(p);
			itp->load--;
			break;
		}
		pp = &p->overflow;
	}
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	itab_walk
 *
 * SYNOPSIS
 *	void itab_walk(itab_ty *, void (*)(itab_ty *, itab_key_ty, void *,
 *		void *), void *);
 *
 * DESCRIPTION
 *	The itab_walk function is used to visit each element of an
 *	integer table, in no particular order.
 */

void
itab_walk(itp, func, arg)
	itab_ty		*itp;
	void		(*func)_((itab_ty *, itab_key_ty, void *, void *));
	void		*arg;
{
	long		j;
	itab_row_ty	*rp;

	trace(("itab_walk(itp = %08lX)\n{\n"/*}*/, (long)itp));
	for (j = 0; j < itp->hash_modulus; ++j)
		for (rp = itp->hash_table[j]; rp; rp = rp->overflow)
			func(itp, rp->key, rp->data, arg);
	trace((/*{*/"}\n"));
}

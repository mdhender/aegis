/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001 Peter Miller;
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
 * MANIFEST: functions to manipulate symtab_iters
 */

#include <error.h>
#include <mem.h>
#include <symtab_iter.h>


void
symtab_iterator_constructor(stip, stp)
	symtab_iterator *stip;
	symtab_ty	*stp;
{
	assert(stip);
	stip->stp = stp;
	stip->pos = 0;
	stip->rp = 0;
}


symtab_iterator *
symtab_iterator_new(stp)
	symtab_ty	*stp;
{
	symtab_iterator *stip;

	assert(stp);
	stip = mem_alloc(sizeof(symtab_iterator));
	symtab_iterator_constructor(stip, stp);
	return stip;
}


void
symtab_iterator_destructor(stip)
	symtab_iterator	*stip;
{
	assert(stip);
	assert(stip->stp);
	stip->stp = 0;
	stip->pos = 0;
	stip->rp = 0;
}


void
symtab_iterator_delete(stip)
	symtab_iterator *stip;
{
	assert(stip);
	assert(stip->stp);
	symtab_iterator_destructor(stip);
	mem_free(stip);
}


void
symtab_iterator_reset(stip)
	symtab_iterator *stip;
{
	assert(stip);
	assert(stip->stp);
	stip->pos = 0;
	stip->rp = 0;
}


int
symtab_iterator_next(stip, key, data)
	symtab_iterator *stip;
	string_ty	**key;
	void		**data;
{
	symtab_ty	*stp;
	symtab_row_ty	*rp;

	assert(stip);
	assert(key);
	assert(data);
	stp = stip->stp;
	assert(stp);
	while (stip->rp == 0)
	{
		if (stip->pos >= stp->hash_modulus)
			return 0;
		stip->rp = stp->hash_table[stip->pos];
		stip->pos++;
	}
	rp = stip->rp;
	*key = rp->key;
	assert(*key);
	*data = rp->data;
	assert(*data);
	stip->rp = rp->overflow;
	return 1;
}

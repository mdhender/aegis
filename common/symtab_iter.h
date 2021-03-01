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
 * MANIFEST: interface definition for common/symtab_iter.c
 */

#ifndef COMMON_SYMTAB_ITER_H
#define COMMON_SYMTAB_ITER_H

#include <symtab.h>

typedef struct symtab_iterator symtab_iterator;
struct symtab_iterator
{
	/*
	 * These fields are for internal use only, and are not to be
	 * accessed or manipulated by clents of this interface.
	 */
	symtab_ty	*stp;
	symtab_row_ty	*rp;
	size_t		pos;
};

void symtab_iterator_constructor _((symtab_iterator *, symtab_ty *));
symtab_iterator *symtab_iterator_new _((symtab_ty *));
void symtab_iterator_destructor _((symtab_iterator *));
void symtab_iterator_delete _((symtab_iterator *));
void symtab_iterator_reset _((symtab_iterator *));
int symtab_iterator_next _((symtab_iterator *, string_ty **key, void **data));

#endif /* COMMON_SYMTAB_ITER_H */

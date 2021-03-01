/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2002 Peter Miller;
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
 * MANIFEST: interface definition for libaegis/col/private.c
 */

#ifndef LIBAEGIS_COL_PRIVATE_H
#define LIBAEGIS_COL_PRIVATE_H

#include <col.h>

/*
 * This structure is defined here solely to facilitate optimization of
 * the interface.  It should not be used by clients of the interface.
 */
typedef struct col_vtbl_ty col_vtbl_ty;
struct col_vtbl_ty
{
	int		size;
	void		(*destructor)(col_ty *);
	struct output_ty *(*create)(col_ty *, int, int, const char *);
	void		(*title)(col_ty *, const char *, const char *);
	void		(*eoln)(col_ty *);
	void		(*eject)(col_ty *);
	void		(*need)(col_ty *, int);

	/*
	 * Putting this field last catches many instances of forgetting
	 * a method pointer.
	 */
	const char	*typename;
};

col_ty *col_new(col_vtbl_ty *);

#endif /* LIBAEGIS_COL_PRIVATE_H */

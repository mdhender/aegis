/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993 Peter Miller.
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
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * MANIFEST: interface definition for aegis/type.c
 */

#ifndef TYPE_H
#define TYPE_H

#include <stddef.h>

#include <main.h>
#include <str.h>

enum type_class_ty
{
	type_class_integer,
	type_class_string,
	type_class_enum,
	type_class_list,
	type_class_struct
};
typedef enum type_class_ty type_class_ty;

typedef struct type_ty type_ty;
struct type_ty
{
	type_class_ty	class;
	char		*name;

	void *(*alloc)_((void));
	void (*free)_((void *this));
	int (*enum_parse)_((string_ty *name, void *addr));
	void (*list_parse)_((void *this, type_ty **type_pp, void **addr_p));
	int (*struct_parse)_((void *this, string_ty *name, type_ty **type_pp,
		void **addr_p, unsigned long *maskp));
};

char *type_class_name _((type_ty *));

#endif /* TYPE_H */

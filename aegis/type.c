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
 * MANIFEST: functions to manipulate types in aegis' data files
 */

#include <type.h>


/*
 * NAME
 *	type_class_name
 *
 * SYNOPSIS
 *	char *type_class_name(type_class_ty n);
 *
 * DESCRIPTION
 *	The type_class_name function is used to
 *	find the symbolic name of a type class number.
 *
 * ARGUMENTS
 *	n	- type class number to finf symbolic name for
 *
 * RETURNS
 *	char *;	pointer to nul-terminated string.
 *
 * CAVEAT
 *	Do not modify the string pointed to.
 */

char *
type_class_name(type)
	type_ty	*type;
{
	switch (type->class)
	{
	case type_class_integer:
		return "integer";

	case type_class_string:
		return "string";

	case type_class_enum:
		return "enumerator";

	case type_class_list:
		return "list";

	case type_class_struct:
		return "structure";
	}
	return "unknown";
}

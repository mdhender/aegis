/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994 Peter Miller.
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
 * MANIFEST: interface definition for aegis/aer/value.c
 */

#ifndef AEGIS_AER_VALUE_H
#define AEGIS_AER_VALUE_H

#include <ac/stddef.h>
#include <main.h>

struct rpt_value_ty;


enum
{
	rpt_value_type_array,
	rpt_value_type_boolean,
	rpt_value_type_deferred,
	rpt_value_type_enumeration,
	rpt_value_type_error,
	rpt_value_type_function,
	rpt_value_type_integer,
	rpt_value_type_list,
	rpt_value_type_nul,
	rpt_value_type_reference,
	rpt_value_type_real,
	rpt_value_type_string,
	rpt_value_type_structure,
	rpt_value_type_time,
	rpt_value_type_void,
	rpt_value_type_MAX	/* must be last */
};


typedef struct rpt_value_method_ty rpt_value_method_ty;
struct rpt_value_method_ty
{
	size_t		size;
	char		*name;
	int		type;
	void (*construct)_((struct rpt_value_ty *));
	void (*destruct)_((struct rpt_value_ty *));
	struct rpt_value_ty *(*arithmetic)_((struct rpt_value_ty *));
	struct rpt_value_ty *(*stringize)_((struct rpt_value_ty *));
	struct rpt_value_ty *(*booleanize)_((struct rpt_value_ty *));
	struct rpt_value_ty *(*lookup)_((struct rpt_value_ty *,
		struct rpt_value_ty *, int));
	struct rpt_value_ty *(*keys)_((struct rpt_value_ty *));
	struct rpt_value_ty *(*count)_((struct rpt_value_ty *));
	char *(*type_of)_((struct rpt_value_ty *));
};

#define RPT_VALUE				\
	rpt_value_method_ty *method;		\
	long		reference_count;

typedef struct rpt_value_ty rpt_value_ty;
struct rpt_value_ty
{
	RPT_VALUE
};

rpt_value_ty *rpt_value_alloc _((rpt_value_method_ty *));
void rpt_value_free _((rpt_value_ty *));
rpt_value_ty *rpt_value_copy _((rpt_value_ty *));
rpt_value_ty *rpt_value_arithmetic _((rpt_value_ty *));
rpt_value_ty *rpt_value_stringize _((rpt_value_ty *));
rpt_value_ty *rpt_value_booleanize _((rpt_value_ty *));
rpt_value_ty *rpt_value_lookup _((rpt_value_ty *, rpt_value_ty *, int));
rpt_value_ty *rpt_value_keys _((rpt_value_ty *));
rpt_value_ty *rpt_value_count _((rpt_value_ty *));
char *rpt_value_typeof _((rpt_value_ty *));

#endif /* AEGIS_AER_VALUE_H */

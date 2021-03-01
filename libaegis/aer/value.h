/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994, 1996, 2002 Peter Miller;
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
 * MANIFEST: interface definition for aegis/aer/value.c
 */

#ifndef AEGIS_AER_VALUE_H
#define AEGIS_AER_VALUE_H

#include <ac/stddef.h>
#include <main.h>

struct rpt_value_ty;


enum rpt_value_type_ty
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
typedef enum rpt_value_type_ty rpt_value_type_ty;


typedef struct rpt_value_method_ty rpt_value_method_ty;
struct rpt_value_method_ty
{
	size_t		size;
	char		*name;
	rpt_value_type_ty type;
	void (*construct)(struct rpt_value_ty *);
	void (*destruct)(struct rpt_value_ty *);
	struct rpt_value_ty *(*arithmetic)(struct rpt_value_ty *);
	struct rpt_value_ty *(*stringize)(struct rpt_value_ty *);
	struct rpt_value_ty *(*booleanize)(struct rpt_value_ty *);
	struct rpt_value_ty *(*lookup)(struct rpt_value_ty *,
		struct rpt_value_ty *, int);
	struct rpt_value_ty *(*keys)(struct rpt_value_ty *);
	struct rpt_value_ty *(*count)(struct rpt_value_ty *);
	char *(*type_of)(struct rpt_value_ty *);
	struct rpt_value_ty *(*undefer)(struct rpt_value_ty *);
};

#define RPT_VALUE				\
	rpt_value_method_ty *method;		\
	long		reference_count;

typedef struct rpt_value_ty rpt_value_ty;
struct rpt_value_ty
{
	RPT_VALUE
};

rpt_value_ty *rpt_value_alloc(rpt_value_method_ty *);
void rpt_value_free(rpt_value_ty *);
rpt_value_ty *rpt_value_copy(rpt_value_ty *);
rpt_value_ty *rpt_value_arithmetic(rpt_value_ty *);
rpt_value_ty *rpt_value_stringize(rpt_value_ty *);
rpt_value_ty *rpt_value_booleanize(rpt_value_ty *);
rpt_value_ty *rpt_value_lookup(rpt_value_ty *, rpt_value_ty *, int);
rpt_value_ty *rpt_value_keys(rpt_value_ty *);
rpt_value_ty *rpt_value_count(rpt_value_ty *);
char *rpt_value_typeof(rpt_value_ty *);
rpt_value_ty *rpt_value_undefer(rpt_value_ty *);

#endif /* AEGIS_AER_VALUE_H */

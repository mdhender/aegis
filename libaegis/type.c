/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-1994, 1996, 2002 Peter Miller;
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
 * MANIFEST: functions to manipulate types in aegis' data files
 */

#include <aer/expr/name.h>
#include <aer/value/enum.h>
#include <aer/value/integer.h>
#include <aer/value/real.h>
#include <aer/value/string.h>
#include <aer/value/struct.h>
#include <aer/value/time.h>
#include <error.h>
#include <fstrcmp.h>
#include <trace.h>
#include <type.h>
#include <zero.h>


static rpt_value_ty *
integer_convert(void *this)
{
    return rpt_value_integer(magic_zero_decode(*(long *)this));
}


static int
integer_is_set(void *this)
{
    return (*(long *)this != 0);
}


type_ty	integer_type =
{
    "integer",
    0, /* alloc */
    0, /* free */
    0, /* enum_parse */
    0, /* list_parse */
    0, /* struct_parse */
    0, /* fuzzy */
    integer_convert,
    integer_is_set,
};


static rpt_value_ty *
time_convert(void *this)
{
    return rpt_value_time(*(time_t *)this);
}


static int
time_is_set(void *this)
{
    return (*(time_t *)this != 0);
}


type_ty	time_type =
{
    "time",
    0, /* alloc */
    0, /* free */
    0, /* enum_parse */
    0, /* list_parse */
    0, /* struct_parse */
    0, /* fuzzy */
    time_convert,
    time_is_set,
};


static rpt_value_ty *
real_convert(void *this)
{
    return rpt_value_real(*(double *)this);
}


static int
real_is_set(void *this)
{
    return (*(double *)this != 0);
}


type_ty	real_type =
{
    "real",
    0, /* alloc */
    0, /* free */
    0, /* enum_parse */
    0, /* list_parse */
    0, /* struct_parse */
    0, /* fuzzy */
    real_convert,
    real_is_set,
};


static rpt_value_ty *
string_convert(void *this)
{
    return rpt_value_string(*(string_ty **)this);
}


static int
string_is_set(void *this)
{
    return (*(string_ty **)this != 0);
}


type_ty string_type =
{
    "string",
    0, /* alloc */
    0, /* free */
    0, /* enum_parse */
    0, /* list_parse */
    0, /* struct_parse */
    0, /* fuzzy */
    string_convert,
    string_is_set,
};


void *
generic_struct_parse(void *this, string_ty *name, type_ty **type_pp,
    unsigned long *mask_p, type_table_ty *table, size_t table_length)
{
    type_table_ty   *tp;
    void	    *addr;
    type_table_ty   *table_end;

    trace(("generic_struct_parse(this = %08lX, name = %08lX, "
	"type_pp = %08lX)\n{\n"/*}*/, (long)this, (long)name, (long)type_pp));
    table_end = table + table_length;
    addr = 0;
    for (tp = table; tp < table_end; ++tp)
    {
	if (!tp->fast_name)
	    tp->fast_name = str_from_c(tp->name);
	if (str_equal(name, tp->fast_name))
	{
	    *type_pp = tp->type;
	    trace_pointer(*type_pp);
	    addr = (char *)this + tp->offset;
	    trace_pointer(addr);
	    *mask_p = tp->mask;
	    break;
	}
    }
    trace((/*{*/"return %08lX;\n}\n", (long)addr));
    return addr;
}


string_ty *
generic_struct_fuzzy(string_ty *name, type_table_ty *table, size_t table_length)
{
    type_table_ty   *tp;
    type_table_ty   *table_end;
    string_ty	    *best_name;
    double	    best_weight;

    trace(("generic_struct_fuzzy(name = %08lX)\n{\n"/*}*/, (long)name));
    table_end = table + table_length;
    best_name = 0;
    best_weight = 0.6;
    for (tp = table; tp < table_end; ++tp)
    {
	double		weight;

	assert(tp->fast_name);
	weight = fstrcmp(name->str_text, tp->name);
	if (weight > best_weight)
	{
	    best_name = tp->fast_name;
	    best_weight = weight;
	}
    }
    trace(("return %08lX;\n", (long)best_name));
    trace((/*{*/"}\n"));
    return best_name;
}


rpt_value_ty *
generic_struct_convert(void *that, type_table_ty *table, size_t table_length)
{
    generic_struct_ty *this;
    type_table_ty   *tp;
    type_table_ty   *table_end;
    rpt_value_ty    *result;

    this = *(generic_struct_ty **)that;
    if (!this)
	return 0;
    trace(("generic_struct_convert(this = %08lX)\n{\n"/*}*/, (long)this));
    table_end = table + table_length;
    result = rpt_value_struct((struct symtab_ty *)0);
    for (tp = table; tp < table_end; ++tp)
    {
	void		*addr;

	if (!tp->fast_name)
	    tp->fast_name = str_from_c(tp->name);
	addr = (char *)this + tp->offset;
	if (tp->mask ? (this->mask & tp->mask) : tp->type->is_set(addr))
	{
	    rpt_value_ty    *vp;

	    vp = tp->type->convert(addr);
	    if (vp)
	    {
	       	rpt_value_struct__set(result, tp->fast_name, vp);
	       	rpt_value_free(vp);
	    }
	}
    }
    trace((/*{*/"return %08lX;\n}\n", (long)result));
    return result;
}


int
generic_struct_is_set(void *this)
{
    return (*(generic_struct_ty **)this != 0);
}


string_ty *
generic_enum_fuzzy(string_ty *name, string_ty **table, size_t table_length)
{
    int		    j;
    string_ty	    *best_name;
    double	    best_weight;

    best_name = 0;
    best_weight = 0.6;
    for (j = 0; j < table_length; ++j)
    {
	double		weight;

	assert(table[j]);
	weight = fstrcmp(name->str_text, table[j]->str_text);
	if (weight > best_weight)
	{
	    best_name = table[j];
	    best_weight = weight;
	}
    }
    return best_name;
}


rpt_value_ty *
generic_enum_convert(void *this, string_ty **table, size_t table_length)
{
    long	    n;

    n = *(int *)this;
    if (n < 0 || n >= table_length)
	return rpt_value_integer(n);
    assert(table[n]);
    return rpt_value_enumeration(n, table[n]);
}


int
generic_enum_is_set(void *this)
{
    return (*(int *)this != 0);
}


void
generic_enum__init(char **table, size_t table_length)
{
    long	    j;
    string_ty	    *name;
    rpt_value_ty    *value;

    for (j = 0; j < table_length; ++j)
    {
	name = str_from_c(table[j]);
	value = rpt_value_enumeration(j, name);
	rpt_expr_name__init(name, value);
	str_free(name);
    }
}


static int enum_option;


void
type_enum_option_set(void)
{
    enum_option = 1;
}


int
type_enum_option_query(void)
{
    return enum_option;
}


void
type_enum_option_clear(void)
{
    enum_option = 0;
}

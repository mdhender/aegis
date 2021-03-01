//
//      aegis - project change supervisor
//      Copyright (C) 1991-1994, 1996, 2002-2008 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <libaegis/aer/expr/name.h>
#include <libaegis/aer/value/boolean.h>
#include <libaegis/aer/value/enum.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/aer/value/real.h>
#include <libaegis/aer/value/string.h>
#include <libaegis/aer/value/struct.h>
#include <libaegis/aer/value/time.h>
#include <common/error.h>
#include <common/fstrcmp.h>
#include <common/trace.h>
#include <libaegis/meta_type.h>
#include <libaegis/zero.h>


static const char *boolean_s[] =
{
    "false",
    "true",
};
static string_ty *boolean_f[SIZEOF(boolean_s)];


const char *
boolean_ename(bool this_thing)
{
    return (this_thing ? boolean_s[1] : boolean_s[0]);
}


static bool
boolean_parse(string_ty *name, void *ptr)
{
    slow_to_fast(boolean_s, boolean_f, SIZEOF(boolean_s));
    for (size_t j = 0; j < SIZEOF(boolean_f); ++j)
    {
	if (str_equal(name, boolean_f[j]))
	{
	    *(bool *)ptr = (bool)j;
	    return true;
	}
    }
    return false;
}


static string_ty *
boolean_fuzzy(string_ty *name)
{
    return generic_enum_fuzzy(name, boolean_f, SIZEOF(boolean_f));
}


static rpt_value::pointer
boolean_convert(void *this_thing)
{
    return rpt_value_boolean::create(*(bool *)this_thing);
}


static bool
boolean_is_set(void *this_thing)
{
    return (*(bool *)this_thing);
}


meta_type boolean_type =
{
    "boolean",
    0, // alloc
    0, // free
    boolean_parse,
    0, // list_parse
    0, // struct_parse
    boolean_fuzzy,
    boolean_convert,
    boolean_is_set,
};


static rpt_value::pointer
integer_convert(void *this_thing)
{
    return rpt_value_integer::create(magic_zero_decode(*(long *)this_thing));
}


static bool
integer_is_set(void *this_thing)
{
    return (*(long *)this_thing != 0);
}


meta_type integer_type =
{
    "integer",
    0, // alloc
    0, // free
    0, // enum_parse
    0, // list_parse
    0, // struct_parse
    0, // fuzzy
    integer_convert,
    integer_is_set,
};


static rpt_value::pointer
time_convert(void *this_thing)
{
    return rpt_value_time::create(*(time_t *)this_thing);
}


static bool
time_is_set(void *this_thing)
{
    return (*(time_t *)this_thing != 0);
}


meta_type time_type =
{
    "time",
    0, // alloc
    0, // free
    0, // enum_parse
    0, // list_parse
    0, // struct_parse
    0, // fuzzy
    time_convert,
    time_is_set,
};


static rpt_value::pointer
real_convert(void *this_thing)
{
    return rpt_value_real::create(*(double *)this_thing);
}


static bool
real_is_set(void *this_thing)
{
    return (*(double *)this_thing != 0);
}


meta_type real_type =
{
    "real",
    0, // alloc
    0, // free
    0, // enum_parse
    0, // list_parse
    0, // struct_parse
    0, // fuzzy
    real_convert,
    real_is_set,
};


static rpt_value::pointer
string_convert(void *this_thing)
{
    return rpt_value_string::create(nstring(*(string_ty **)this_thing));
}


static bool
string_is_set(void *this_thing)
{
    return (*(string_ty **)this_thing != 0);
}


meta_type string_type =
{
    "string",
    0, // alloc
    0, // free
    0, // enum_parse
    0, // list_parse
    0, // struct_parse
    0, // fuzzy
    string_convert,
    string_is_set,
};


void *
generic_struct_parse(void *this_thing, string_ty *name, meta_type **type_pp,
    unsigned long *mask_p, int *redefinition_ok_p, type_table_ty *table,
    size_t table_length)
{
    type_table_ty   *tp;
    void            *addr;
    type_table_ty   *table_end;

    trace(("generic_struct_parse(this_thing = %08lX, name = %08lX, "
        "type_pp = %08lX)\n{\n",
           (long)this_thing, (long)name, (long)type_pp));
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
            addr = (char *)this_thing + tp->offset;
            trace_pointer(addr);
            *mask_p = tp->mask;
	    *redefinition_ok_p = tp->redefinition_ok;
            break;
        }
    }
    trace(("return %08lX;\n}\n", (long)addr));
    return addr;
}


string_ty *
generic_struct_fuzzy(string_ty *name, type_table_ty *table, size_t table_length)
{
    type_table_ty   *tp;
    type_table_ty   *table_end;
    string_ty       *best_name;
    double          best_weight;

    trace(("generic_struct_fuzzy(name = %08lX)\n{\n", (long)name));
    table_end = table + table_length;
    best_name = 0;
    best_weight = 0.6;
    for (tp = table; tp < table_end; ++tp)
    {
        double          weight;

        assert(tp->fast_name);
        weight = fstrcmp(name->str_text, tp->name);
        if (weight > best_weight)
        {
            best_name = tp->fast_name;
            best_weight = weight;
        }
    }
    trace(("return %08lX;\n", (long)best_name));
    trace(("}\n"));
    return best_name;
}


rpt_value::pointer
generic_struct_convert(void *that, type_table_ty *table, size_t table_length)
{
    generic_struct *this_thing;
    type_table_ty   *tp;
    type_table_ty   *table_end;

    this_thing = *(generic_struct **)that;
    if (!this_thing)
        return rpt_value::pointer();
    trace(("generic_struct_convert(this_thing = %08lX)\n{\n",
           (long)this_thing));
    table_end = table + table_length;
    rpt_value_struct *rvs = new rpt_value_struct();
    rpt_value::pointer result(rvs);
    for (tp = table; tp < table_end; ++tp)
    {
        if (!tp->fast_name)
            tp->fast_name = str_from_c(tp->name);
        void *addr = (char *)this_thing + tp->offset;
        if (tp->mask ? (this_thing->mask & tp->mask) : tp->type->is_set(addr))
        {
            rpt_value::pointer vp = tp->type->convert(addr);
            if (vp)
            {
                rvs->assign(tp->name, vp);
            }
        }
    }
    trace(("return %08lX;\n}\n", (long)result.get()));
    return result;
}


bool
generic_struct_is_set(void *this_thing)
{
    return (*(generic_struct **)this_thing != 0);
}


string_ty *
generic_enum_fuzzy(string_ty *name, string_ty **table, size_t table_length)
{
    size_t          j;
    string_ty       *best_name;
    double          best_weight;

    best_name = 0;
    best_weight = 0.6;
    for (j = 0; j < table_length; ++j)
    {
        double          weight;

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


rpt_value::pointer
generic_enum_convert(int n, string_ty **table, size_t table_length)
{
    if (n < 0 || n >= (int)table_length)
        return rpt_value_integer::create(n);
    assert(table[n]);
    return rpt_value_enumeration::create(n, nstring(table[n]));
}


void
generic_enum__init(const char *const *table, size_t table_length)
{
    for (size_t j = 0; j < table_length; ++j)
    {
        nstring name(table[j]);
        rpt_value::pointer value = rpt_value_enumeration::create(j, name);
        rpt_expr_name__init(name, value);
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

//
//	aegis - project change supervisor
//	Copyright (C) 1991-1994, 2001-2004 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: interface definition for aegis/meta_type.cc
//

#ifndef LIBAEGIS_META_TYPE_H
#define LIBAEGIS_META_TYPE_H

#include <ac/stddef.h>

#include <main.h>
#include <str.h>

struct type_table_ty
{
    const char      *name;
    size_t          offset;
    struct type_ty  *type;
    unsigned long   mask;
    int             redefinition_ok;
    string_ty       *fast_name;
};

struct type_ty
{
    const char      *name;

    void *(*alloc)(void);
    void (*free)(void *this_thing);
    bool (*enum_parse)(string_ty *name, void *);
    void *(*list_parse)(void *this_thing, type_ty **type_pp);
    void *(*struct_parse)(void *this_thing, string_ty *name, type_ty **type_pp,
	unsigned long *maskp, int *redefinition_ok_p);
    string_ty *(*fuzzy)(string_ty *name);
    struct rpt_value_ty *(*convert)(void *this_thing);
    bool (*is_set)(void *);
};

struct generic_struct_ty
{
    long            reference_count;
    unsigned long   mask;
    string_ty       *errpos;
};

extern type_ty boolean_type;
#define boolean_copy(x) (x)
const char *boolean_ename(bool);
extern type_ty integer_type;
#define integer_copy(x) (x)
extern type_ty real_type;
#define real_copy(x) (x)
extern type_ty string_type;
#define string_copy(x) str_copy(x)
extern type_ty time_type;
#define time_copy(x) (x)

void *generic_struct_parse(void *, string_ty *, type_ty **, unsigned long *,
    int *, type_table_ty *, size_t);
string_ty *generic_struct_fuzzy(string_ty *, type_table_ty *, size_t);
bool generic_struct_is_set(void *);
string_ty *generic_enum_fuzzy(string_ty *, string_ty **, size_t);
struct rpt_value_ty *generic_struct_convert(void *, type_table_ty *,
    size_t);
struct rpt_value_ty *generic_enum_convert(int, string_ty **, size_t);
void generic_enum__init(const char *const *, size_t);

void type_enum_option_set(void);
void type_enum_option_clear(void);
int type_enum_option_query(void);

#endif // LIBAEGIS_META_TYPE_H

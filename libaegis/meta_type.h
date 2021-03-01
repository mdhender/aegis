//
//	aegis - project change supervisor
//	Copyright (C) 1991-1994, 2001-2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#ifndef LIBAEGIS_META_TYPE_H
#define LIBAEGIS_META_TYPE_H

#include <common/ac/stddef.h>

#include <common/main.h>
#include <common/str.h>
#include <libaegis/aer/value.h>

class meta_type; // forward

struct type_table_ty
{
    const char      *name;
    size_t          offset;
    meta_type       *type;
    unsigned long   mask;
    int             redefinition_ok;
    string_ty       *fast_name;
};

/**
  * The meta_type class is used to represent a generic interface to
  * reflection data, used to read and write Aegis meta-data files.
  */
class meta_type
{
public:
    const char      *name;

    void *(*alloc)(void);
    void (*free)(void *this_thing);
    bool (*enum_parse)(string_ty *name, void *);
    void *(*list_parse)(void *this_thing, meta_type **type_pp);
    void *(*struct_parse)(void *this_thing, string_ty *name,
        meta_type **type_pp, unsigned long *maskp, int *redefinition_ok_p);
    string_ty *(*fuzzy)(string_ty *name);
    rpt_value::pointer (*convert)(void *this_thing);
    bool (*is_set)(void *);
};

/**
  * The generic_struct class is used to represent a set of attributes
  * common to all meta-data structures.
  */
class generic_struct
{
public:
    long            reference_count;
    unsigned long   mask;
    string_ty       *errpos;
};

extern meta_type boolean_type;
#define boolean_copy(x) (x)
const char *boolean_ename(bool);
extern meta_type integer_type;
#define integer_copy(x) (x)
extern meta_type real_type;
#define real_copy(x) (x)
extern meta_type string_type;
#define string_copy(x) str_copy(x)
extern meta_type time_type;
#define time_copy(x) (x)

void *generic_struct_parse(void *, string_ty *, meta_type **, unsigned long *,
    int *, type_table_ty *, size_t);
string_ty *generic_struct_fuzzy(string_ty *, type_table_ty *, size_t);
bool generic_struct_is_set(void *);
string_ty *generic_enum_fuzzy(string_ty *, string_ty **, size_t);
rpt_value::pointer generic_struct_convert(void *, type_table_ty *,
    size_t);
rpt_value::pointer generic_enum_convert(int, string_ty **, size_t);
void generic_enum__init(const char *const *, size_t);

void type_enum_option_set(void);
void type_enum_option_clear(void);
int type_enum_option_query(void);

#endif // LIBAEGIS_META_TYPE_H

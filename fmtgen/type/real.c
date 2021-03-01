/*
 *	aegis - project change supervisor
 *	Copyright (C) 1998, 1999, 2002, 2003 Peter Miller;
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
 * MANIFEST: functions to manipulate real type
 */

#include <indent.h>
#include <mem.h>
#include <type/real.h>


static void
gen_include_declarator(type_ty *type, string_ty *name, int is_a_list)
{
    const char      *deref;

    deref = (is_a_list ? "*" : "");
    indent_printf("%s\1%s%s;\n", "double", deref, name->str_text);
}


static void
gen_code_declarator(type_ty *type, string_ty *name, int is_a_list, int show)
{
    indent_printf("real_write(fp, ");
    if (is_a_list)
    {
	indent_printf("(const char *)0");
	show = 1;
    }
    else
    {
	indent_printf("\"%s\"", name->str_text);
	if (show < 0)
    	    show = 0;
    }
    indent_printf(", this_thing->%s, %d);\n", name->str_text, show);
}


static void
gen_code_call_xml(type_ty *type, string_ty *form_name, string_ty *member_name,
    int show)
{
    if (show < 0)
	show = 0;
    indent_printf
    (
	"real_write_xml(fp, \"%s\", this_thing->%s, %d);\n",
	form_name->str_text,
	member_name->str_text,
	show
    );
}


static void
gen_free_declarator(type_ty *type, string_ty *name, int is_a_list)
{
    if (is_a_list)
       	indent_printf(";\n");
}


type_method_ty type_real =
{
    sizeof(type_ty),
    "real",
    1, /* has a mask */
    0, /* constructor */
    0, /* destructor */
    0, /* gen_include */
    gen_include_declarator,
    0, /* gen_code */
    gen_code_declarator,
    gen_code_call_xml,
    gen_free_declarator,
    0, /* member_add */
    0, /* in_include_file */
};

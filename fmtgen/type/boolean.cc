//
//	aegis - project change supervisor
//	Copyright (C) 2004 Peter Miller;
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
// MANIFEST: implementation of the type_boolean class
//

#pragma implementation "type_boolean"

#include <indent.h>
#include <mem.h>
#include <type/boolean.h>


type_boolean::~type_boolean()
{
}


type_boolean::type_boolean() :
    type_ty("boolean")
{
}


void
type_boolean::gen_include_declarator(const nstring &variable_name,
    bool is_a_list) const
{
    const char *deref = (is_a_list ? "*" : "");
    indent_printf("%s\1%s%s;\n", "bool", deref, variable_name.c_str());
}


void
type_boolean::gen_code_declarator(const nstring &variable_name,
    bool is_a_list, int attributes) const
{
    int show = 1;
    if (attributes & (ATTRIBUTE_SHOW_IF_DEFAULT | ATTRIBUTE_HIDE_IF_DEFAULT))
	show = !!(attributes & ATTRIBUTE_SHOW_IF_DEFAULT);

    indent_printf("boolean_write(fp, ");
    if (is_a_list)
    {
	indent_printf("(const char *)0");
	show = 1;
    }
    else
    {
	indent_printf("\"%s\"", variable_name.c_str());
    }
    indent_printf(", this_thing->%s, %d);\n", variable_name.c_str(), show);
}


void
type_boolean::gen_code_call_xml(const nstring &form_name,
    const nstring &member_name, int attributes) const
{
    int show = !!(attributes & ATTRIBUTE_SHOW_IF_DEFAULT);
    indent_printf
    (
	"boolean_write_xml(fp, \"%s\", this_thing->%s, %d);\n",
	form_name.c_str(),
	member_name.c_str(),
	show
    );
}


void
type_boolean::gen_free_declarator(const nstring &variable_name,
    bool is_a_list) const
{
    if (is_a_list)
       	indent_printf(";\n");
}


nstring
type_boolean::c_name_inner()
    const
{
    return "bool";
}


bool
type_boolean::has_a_mask()
    const
{
    return true;
}
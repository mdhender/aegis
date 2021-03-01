//
//	aegis - project change supervisor
//	Copyright (C) 1991-1994, 1997-1999, 2001-2004 Peter Miller;
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
// MANIFEST: enumerated types functions generation
//

#include <indent.h>
#include <mem.h>
#include <parse.h>
#include <type/enumeration.h>


typedef struct type_enum_ty type_enum_ty;
struct type_enum_ty
{
    // inherited
    TYPE_TY

    // instance variables
    size_t	    nelements;
    string_ty	    **element;
};


static void
constructor(type_ty *type)
{
    type_enum_ty    *this_thing;

    this_thing = (type_enum_ty *)type;
    this_thing->nelements = 0;
    this_thing->element = 0;
}


static void
destructor(type_ty *type)
{
    type_enum_ty    *this_thing;
    size_t	    j;

    this_thing = (type_enum_ty *)type;
    for (j = 0; j < this_thing->nelements; ++j)
	str_free(this_thing->element[j]);
    if (this_thing->element)
	mem_free(this_thing->element);
}


static void
gen_include(type_ty *type)
{
    type_enum_ty    *this_thing;
    size_t	    j;

    this_thing = (type_enum_ty *)type;
    indent_putchar('\n');
    indent_printf("#ifndef %s_DEF\n", this_thing->name->str_text);
    indent_printf("#define %s_DEF\n", this_thing->name->str_text);
    indent_printf("enum %s_ty\n", this_thing->name->str_text);
    indent_printf("{\n");
    for (j = 0; j < this_thing->nelements; ++j)
    {
	indent_printf
	(
	    "%s_%s",
	    this_thing->name->str_text,
	    this_thing->element[j]->str_text
	);
	if (j < this_thing->nelements - 1)
	    indent_putchar(',');
	indent_putchar('\n');
    }
    indent_printf("};\n");
    indent_printf("#define %s_max %u\n",
                  this_thing->name->str_text,
                  this_thing->nelements);
    indent_printf("#ifdef CONF_enum_is_int\n");
    indent_printf
    (
	"typedef enum %s_ty %s_ty;\n",
	this_thing->name->str_text,
	this_thing->name->str_text
    );
    indent_printf("#else\n");
    indent_printf("typedef int %s_ty;\n", this_thing->name->str_text);
    indent_printf("#endif\n");
    indent_printf("#endif /* %s_DEF */\n", this_thing->name->str_text);

    indent_putchar('\n');
    indent_printf("extern type_ty %s_type;\n", this_thing->name->str_text);

    indent_putchar('\n');
    indent_printf
    (
	"void %s_write(struct output_ty *, const char *, %s_ty, int);\n",
	this_thing->name->str_text,
	this_thing->name->str_text
    );
    indent_printf
    (
	"void %s_write_xml(struct output_ty *, const char *, %s_ty, int);\n",
	this_thing->name->str_text,
	this_thing->name->str_text
    );
    indent_printf
    (
	"const char *%s_ename(%s_ty);\n",
	this_thing->name->str_text,
	this_thing->name->str_text
    );
}


static void
gen_include_declarator(type_ty *type, string_ty *variable_name, int is_a_list)
{
    indent_printf
    (
       	"%s_ty\1%s%s;\n",
       	type->name->str_text,
       	(is_a_list ? "*" : ""),
       	variable_name->str_text
    );
}


static void
gen_code(type_ty *type)
{
    type_enum_ty    *this_thing;
    size_t	    j;
    string_ty	    *s;

    this_thing = (type_enum_ty *)type;
    indent_putchar('\n');
    indent_printf("static const char *%s_s[] =\n", this_thing->name->str_text);
    indent_printf("{\n");
    for (j = 0; j < this_thing->nelements; ++j)
	indent_printf("\"%s\",\n", this_thing->element[j]->str_text);
    indent_printf("};\n");
    indent_printf
    (
	"static string_ty\1*%s_f[SIZEOF(%s_s)];\n",
	this_thing->name->str_text,
	this_thing->name->str_text
    );

    indent_putchar('\n');
    indent_printf("const char *\n");
    indent_printf
    (
	"%s_ename(%s_ty this_thing)\n",
	this_thing->name->str_text,
	this_thing->name->str_text
    );
    indent_printf("{\n");
    indent_printf("static char\1buffer[20];\n\n");
    indent_printf("if ((int)this_thing >= 0 && (int)this_thing < %d)\n",
                  this_thing->nelements);
    indent_more();
    indent_printf("return %s_s[this_thing];\n", this_thing->name->str_text);
    indent_less();
    indent_printf
    (
	"snprintf(buffer, sizeof(buffer), \"%%d\", (int)this_thing);\n"
    );
    indent_printf("return buffer;\n");
    indent_printf("}\n");

    indent_putchar('\n');
    indent_printf("void\n");
    indent_printf
    (
	"%s_write(output_ty *fp, const char *name, %s_ty this_thing, "
        "int show)\n",
	this_thing->name->str_text,
	this_thing->name->str_text
    );
    indent_printf("{\n");
    indent_printf("if (this_thing == 0)\n");
    indent_printf("{\n");
    indent_printf("if (!show || type_enum_option_query())\n");
    indent_more();
    indent_printf("return;\n");
    indent_less();
    indent_printf("}\n");
    indent_printf("if (name)\n");
    indent_printf("{\n");
    indent_printf("output_fputs(fp, name);\n");
    indent_printf("output_fputs(fp, \" = \");\n");
    indent_printf("}\n");
    indent_printf("output_fputs(fp, %s_s[this_thing]);\n",
                  this_thing->name->str_text);
    indent_printf("if (name)\n");
    indent_more();
    indent_printf("output_fputs(fp, \";\\n\");\n");
    indent_less();
    indent_printf("}\n");

    indent_putchar('\n');
    indent_printf("void\n");
    indent_printf
    (
	"%s_write_xml(output_ty *fp, const char *name, %s_ty this_thing, "
        "int show)\n",
	this_thing->name->str_text,
	this_thing->name->str_text
    );
    indent_printf("{\n");
    indent_printf("if (this_thing == 0)\n");
    indent_printf("{\n");
    indent_printf("if (!show || type_enum_option_query())\n");
    indent_more();
    indent_printf("return;\n");
    indent_less();
    indent_printf("}\n");
    indent_printf("assert(name);\n");
    indent_printf
    (
        "assert((size_t)this_thing < SIZEOF(%s_s));\n",
	this_thing->name->str_text
    );
    indent_printf("output_fputc(fp, '<');\n");
    indent_printf("output_fputs(fp, name);\n");
    indent_printf("output_fputc(fp, '>');\n");
    indent_printf(
        "output_fputs(fp, %s_s[this_thing]);\n", this_thing->name->str_text);
    indent_printf("output_fputs(fp, \"</\");\n");
    indent_printf("output_fputs(fp, name);\n");
    indent_printf("output_fputs(fp, \">\\n\");\n");
    indent_printf("}\n");

    indent_putchar('\n');
    indent_printf("static int\n");
    indent_printf("%s_parse(string_ty *name)\n", this_thing->name->str_text);
    indent_printf("{\n");
    indent_printf("size_t\1j;\n");
    indent_putchar('\n');
    indent_printf
    (
	"slow_to_fast(%s_s, %s_f, SIZEOF(%s_s));\n",
	this_thing->name->str_text,
	this_thing->name->str_text,
	this_thing->name->str_text
    );
    indent_printf
    (
	"for (j = 0; j < SIZEOF(%s_f); ++j)\n",
	this_thing->name->str_text
    );
    indent_printf("{\n");
    indent_printf("if (str_equal(name, %s_f[j]))\n",
                  this_thing->name->str_text);
    indent_more();
    indent_printf("return j;\n");
    indent_less();
    indent_printf("}\n");
    indent_printf("return -1;\n");
    indent_printf("}\n");

    indent_putchar('\n');
    indent_printf("static string_ty *\n");
    indent_printf("%s_fuzzy(string_ty *name)\n", this_thing->name->str_text);
    indent_printf("{\n");
    indent_printf
    (
	"return generic_enum_fuzzy(name, %s_f, SIZEOF(%s_f));\n",
	this_thing->name->str_text,
	this_thing->name->str_text
    );
    indent_printf("}\n");

    s = str_format
    (
	"generic_enum__init(%s_s, SIZEOF(%s_s));",
	this_thing->name->str_text,
	this_thing->name->str_text
    );
    generate_code__init(s);
    str_free(s);

    indent_putchar('\n');
    indent_printf("static struct rpt_value_ty *\n");
    indent_printf("%s_convert(void *this_thing)\n", this_thing->name->str_text);
    indent_printf("{\n");
    indent_printf("if (!%s_f[0])\n", this_thing->name->str_text);
    indent_more();
    indent_printf
    (
	"slow_to_fast(%s_s, %s_f, SIZEOF(%s_s));\n",
	this_thing->name->str_text,
	this_thing->name->str_text,
	this_thing->name->str_text
    );
    indent_less();
    indent_printf("return\n");
    indent_more();
    indent_printf("generic_enum_convert\n");
    indent_printf("(\n");
    indent_printf("this_thing,\n");
    indent_printf("%s_f,\n", this_thing->name->str_text);
    indent_printf("SIZEOF(%s_f)\n", this_thing->name->str_text);
    indent_printf(");\n");
    indent_less();
    indent_printf("}\n");

    indent_putchar('\n');
    indent_printf("type_ty %s_type =\n", this_thing->name->str_text);
    indent_printf("{\n");
    indent_printf("\"%s\",\n", this_thing->name->str_text);
    indent_printf("0, /* alloc */\n");
    indent_printf("0, /* free */\n");
    indent_printf("%s_parse,\n", this_thing->name->str_text);
    indent_printf("0, /* list_parse */\n");
    indent_printf("0, /* struct_parse */\n");
    indent_printf("%s_fuzzy,\n", this_thing->name->str_text);
    indent_printf("%s_convert,\n", this_thing->name->str_text);
    indent_printf("generic_enum_is_set,\n");
    indent_printf("};\n");
}


static void
gen_code_declarator(type_ty *type, string_ty *variable_name, int is_a_list,
    int attributes)
{
    int             show;

    if (attributes & (ATTRIBUTE_SHOW_IF_DEFAULT | ATTRIBUTE_HIDE_IF_DEFAULT))
	show = !!(attributes & ATTRIBUTE_SHOW_IF_DEFAULT);
    else
	show = 1;

    indent_printf("%s_write(fp, ", type->name->str_text);
    if (is_a_list)
    {
	indent_printf("(const char *)0");
	show = 1;
    }
    else
    {
	indent_printf("\"%s\"", variable_name->str_text);
    }
    indent_printf(", this_thing->%s, %d);\n", variable_name->str_text, show);
}


static void
gen_code_call_xml(type_ty *type, string_ty *form_name, string_ty *member_name,
    int attributes)
{
    int             show;

    show = !!(attributes & ATTRIBUTE_SHOW_IF_DEFAULT);
    indent_printf
    (
	"%s_write_xml(fp, \"%s\", this_thing->%s, %d);\n",
	type->name->str_text,
	form_name->str_text,
	member_name->str_text,
	show
    );
}


static void
gen_free_declarator(type_ty *type, string_ty *variable_name, int is_a_list)
{
    if (is_a_list)
       	indent_printf(";\n");
}


static void
member_add(type_ty *type, string_ty *member_name, type_ty *member_type,
    int attributes)
{
    type_enum_ty    *this_thing;
    size_t          nbytes;

    this_thing = (type_enum_ty *)type;
    nbytes = (this_thing->nelements + 1) * sizeof(this_thing->element[0]);
    this_thing->element =
        (string_ty **)mem_change_size(this_thing->element, nbytes);
    this_thing->element[this_thing->nelements++] = str_copy(member_name);
}


static string_ty *
c_name(type_ty *tp)
{
    type_enum_ty    *this_thing;

    this_thing = (type_enum_ty *)tp;
    return str_format("%s_ty", this_thing->name->str_text);
}


type_method_ty type_enumeration =
{
    sizeof(type_enum_ty),
    "enumeration",
    1, // has a mask
    constructor,
    destructor,
    gen_include,
    gen_include_declarator,
    gen_code,
    gen_code_declarator,
    gen_code_call_xml,
    gen_free_declarator,
    member_add,
    0, // in_include_file
    c_name,
};
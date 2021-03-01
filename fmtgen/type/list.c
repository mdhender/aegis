/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-1994, 1998, 1999, 2001-2003 Peter Miller;
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
 * MANIFEST: functions to manipulate list types
 */

#include <indent.h>
#include <mem.h>
#include <type/list.h>


typedef struct type_list_ty type_list_ty;
struct type_list_ty
{
    /* inherited */
    TYPE_TY

    /* instance variables */
    type_ty         *subtype;
};


static void
gen_include(type_ty *type)
{
    string_ty       *s;
    type_list_ty    *this_thing;

    this_thing = (type_list_ty *)type;
    indent_putchar('\n');
    indent_printf("#ifndef %s_DEF\n", this_thing->name->str_text);
    indent_printf("#define %s_DEF\n", this_thing->name->str_text);
    indent_printf
    (
	"typedef struct %s_ty %s_ty;\n",
	this_thing->name->str_text,
	this_thing->name->str_text
    );
    indent_printf("struct %s_ty\n", this_thing->name->str_text);
    indent_printf("{\n");
    indent_printf("%s\1length;\n", "size_t");
    indent_printf("%s\1maximum;\n", "size_t");
    s = str_from_c("list");
    type_gen_include_declarator(this_thing->subtype, s, 1);
    str_free(s);
    indent_printf("};\n");
    indent_printf("#endif /* %s_DEF */\n", this_thing->name->str_text);

    indent_putchar('\n');
    indent_printf("extern type_ty %s_type;\n", this_thing->name->str_text);

    indent_putchar('\n');
    indent_printf
    (
	"void %s_write(struct output_ty *, const char *, %s_ty *);\n",
	this_thing->name->str_text,
	this_thing->name->str_text
    );
    indent_printf
    (
	"void %s_write_xml(struct output_ty *, const char *, %s_ty *);\n",
	this_thing->name->str_text,
	this_thing->name->str_text
    );
}


static void
gen_include_declarator(type_ty *type, string_ty *variable_name, int is_a_list)
{
    const char      *deref;

    deref = (is_a_list ? "*" : "");
    indent_printf
    (
	"%s_ty\1%s*%s;\n",
	type->name->str_text,
	deref,
	variable_name->str_text
    );
}


static void
gen_code(type_ty *type)
{
    string_ty       *s;
    type_list_ty    *this_thing;
    static int      lincl_done;

    if (!lincl_done)
    {
	indent_putchar('\n');
	indent_printf("#include <aer/value/list.h>\n");
	lincl_done = 1;
    }

    this_thing = (type_list_ty *)type;
    indent_putchar('\n');
    indent_printf("void\n");
    indent_printf
    (
	"%s_write(output_ty *fp, const char *name, %s_ty *this_thing)\n",
	this_thing->name->str_text,
	this_thing->name->str_text
    );
    indent_printf("{\n");
    indent_printf("size_t\1j;\n");
    indent_putchar('\n');
    indent_printf("if (!this_thing)\n");
    indent_more();
    indent_printf("return;\n");
    indent_less();
    indent_printf
    (
	"trace((\"%s_write(name = \\\"%%s\\\", this_thing = %%08lX)\\n"
	    "{\\n\", name, (long)this_thing));\n",
	this_thing->name->str_text
    );
    indent_printf("if (name)\n");
    indent_printf("{\n");
    indent_printf("output_fputs(fp, name);\n");
    indent_printf("output_fputs(fp, \" =\\n\");\n");
    indent_printf("}\n");
    indent_printf("assert(this_thing->length <= this_thing->maximum);\n");
    indent_printf("assert(!this_thing->list == !this_thing->maximum);\n");
    indent_printf("output_fputs(fp, \"[\\n\");\n");
    indent_printf("for (j = 0; j < this_thing->length; ++j)\n");
    indent_printf("{\n");
    s = str_from_c("list[j]");
    type_gen_code_declarator(this_thing->subtype, s, 1, 1);
    str_free(s);
    indent_printf("output_fputs(fp, \",\\n\");\n");
    indent_printf("}\n");
    indent_printf("output_fputs(fp, \"]\");\n");
    indent_printf("if (name)\n");
    indent_more();
    indent_printf("output_fputs(fp, \";\\n\");\n");
    indent_less();
    indent_printf("trace((\"}\\n\"));\n");
    indent_printf("}\n");

    this_thing = (type_list_ty *)type;
    indent_putchar('\n');
    indent_printf("void\n");
    indent_printf
    (
	"%s_write_xml(output_ty *fp, const char *name, %s_ty *this_thing)\n",
	this_thing->name->str_text,
	this_thing->name->str_text
    );
    indent_printf("{\n");
    indent_printf("size_t\1j;\n");
    indent_putchar('\n');
    indent_printf("if (!this_thing)\n");
    indent_more();
    indent_printf("return;\n");
    indent_less();
    indent_printf
    (
	"trace((\"%s_write_xml(name = \\\"%%s\\\", this_thing = %%08lX)\\n"
	    "{\\n\", name, (long)this_thing));\n",
	this_thing->name->str_text
    );
    indent_printf("assert(name);\n");
    indent_printf("output_fputc(fp, '<');\n");
    indent_printf("output_fputs(fp, name);\n");
    indent_printf("output_fputs(fp, \">\\n\");\n");
    indent_printf("assert(this_thing->length <= this_thing->maximum);\n");
    indent_printf("assert(!this_thing->list == !this_thing->maximum);\n");
    indent_printf("for (j = 0; j < this_thing->length; ++j)\n");
    indent_printf("{\n");
    s = str_from_c("list[j]");
    type_gen_code_call_xml(
        this_thing->subtype, this_thing->subtype->name, s, 1);
    str_free(s);
    indent_printf("}\n");
    indent_printf("output_fputs(fp, \"</\");\n");
    indent_printf("output_fputs(fp, name);\n");
    indent_printf("output_fputs(fp, \">\\n\");\n");
    indent_printf("trace((\"}\\n\"));\n");
    indent_printf("}\n");

    indent_putchar('\n');
    indent_printf("static void *\n");
    indent_printf("%s_alloc(void)\n", this_thing->name->str_text);
    indent_printf("{\n");
    indent_printf("%s_ty\1*result;\n\n", this_thing->name->str_text);
    indent_printf("trace((\"%s_alloc()\\n{\\n\"));\n",
                  this_thing->name->str_text);
    indent_printf
    (
	"result = mem_alloc(sizeof(%s_ty));\n",
	this_thing->name->str_text
    );
    indent_printf("result->list = 0;\n");
    indent_printf("result->length = 0;\n");
    indent_printf("result->maximum = 0;\n");
    indent_printf("trace((\"return %%08lX;\\n\", (long)result));\n");
    indent_printf("trace((\"}\\n\"));\n");
    indent_printf("return result;\n");
    indent_printf("}\n");

    indent_putchar('\n');
    indent_printf("static void\n");
    indent_printf("%s_free(void *that)\n", this_thing->name->str_text);
    indent_printf("{\n");
    indent_printf("%s_ty\1*this_thing = that;\n", this_thing->name->str_text);
    indent_printf("size_t\1j;\n");
    indent_putchar('\n');
    indent_printf("if (!this_thing)\n");
    indent_more();
    indent_printf("return;\n");
    indent_less();
    indent_printf
    (
	"trace((\"%s_free(this_thing = %%08lX)\\n{\\n\", (long)this_thing));\n",
	this_thing->name->str_text
    );
    indent_printf("assert(this_thing->length <= this_thing->maximum);\n");
    indent_printf("assert(!this_thing->list == !this_thing->maximum);\n");
    indent_printf("for (j = 0; j < this_thing->length; ++j)\n");
    indent_more();
    s = str_from_c("list[j]");
    type_gen_free_declarator(this_thing->subtype, s, 1);
    str_free(s);
    indent_less();
    indent_printf("if (this_thing->list)\n");
    indent_more();
    indent_printf("mem_free(this_thing->list);\n");
    indent_less();
    indent_printf("mem_free(this_thing);\n");
    indent_printf("trace((\"}\\n\"));\n");
    indent_printf("}\n");

    indent_putchar('\n');
    indent_printf("static void *\n");
    indent_printf
    (
	"%s_parse(void *that, type_ty **type_pp)\n",
	this_thing->name->str_text
    );
    indent_printf("{\n");
    indent_printf("%s_ty\1*this_thing = that;\n", this_thing->name->str_text);
    indent_printf("void\1*addr;\n");
    indent_putchar('\n');
    indent_printf
    (
	"trace((\"%s_parse(this_thing = %%08lX, type_pp = %%08lX)\\n"
	    "{\\n\", (long)this_thing, (long)type_pp));\n",
	this_thing->name->str_text
    );
    indent_printf("assert(this_thing->length <= this_thing->maximum);\n");
    indent_printf("assert(!this_thing->list == !this_thing->maximum);\n");
    indent_printf("*type_pp = &%s_type;\n",
                  this_thing->subtype->name->str_text);
    indent_printf("trace_pointer(*type_pp);\n");
    indent_printf("if (this_thing->length >= this_thing->maximum)\n");
    indent_printf("{\n");
    indent_printf("size_t\1nbytes;\n\n");
    indent_printf("this_thing->maximum = this_thing->maximum * 2 + 16;\n");
    indent_printf(
        "nbytes = this_thing->maximum * sizeof(this_thing->list[0]);\n");
    indent_printf(
        "this_thing->list = mem_change_size(this_thing->list, nbytes);\n");
    indent_printf("}\n");
    indent_printf("addr = &this_thing->list[this_thing->length++];\n");
    indent_printf("trace((\"return %%08lX;\\n\", (long)addr));\n");
    indent_printf("trace((\"}\\n\"));\n");
    indent_printf("return addr;\n");
    indent_printf("}\n");

    indent_putchar('\n');
    indent_printf("static rpt_value_ty *\n");
    indent_printf("%s_convert(void *that)\n", this_thing->name->str_text);
    indent_printf("{\n");
    indent_printf("%s_ty\1*this_thing;\n", this_thing->name->str_text);
    indent_printf("rpt_value_ty\1*result;\n");
    indent_printf("size_t\1j;\n");
    indent_printf("rpt_value_ty\1*vp;\n");
    indent_putchar('\n');
    indent_printf("this_thing = *(%s_ty **)that;\n",
	this_thing->name->str_text);
    indent_printf("if (!this_thing)\n");
    indent_more();
    indent_printf("return 0;\n");
    indent_less();
    indent_printf
    (
	"trace((\"%s_convert(this_thing = %%08lX)\\n{\\n\", "
        "(long)this_thing));\n",
	this_thing->name->str_text
    );
    indent_printf("assert(this_thing->length <= this_thing->maximum);\n");
    indent_printf("assert(!this_thing->list == !this_thing->maximum);\n");
    indent_printf("result = rpt_value_list();\n");
    indent_printf("for (j = 0; j < this_thing->length; ++j)\n");
    indent_printf("{\n");
    indent_printf
    (
	"vp = %s_type.convert(&this_thing->list[j]);\n",
	this_thing->subtype->name->str_text
    );
    indent_printf("assert(vp);\n");
    indent_printf("rpt_value_list_append(result, vp);\n");
    indent_printf("rpt_value_free(vp);\n");
    indent_printf("}\n");
    indent_printf("trace((\"}\\n\"));\n");
    indent_printf("trace((\"return %%08lX;\\n\", (long)result));\n");
    indent_printf("return result;\n");
    indent_printf("}\n");

    indent_putchar('\n');
    indent_printf("type_ty %s_type =\n", this_thing->name->str_text);
    indent_printf("{\n");
    indent_printf("\"%s\",\n", this_thing->name->str_text);
    indent_printf("%s_alloc,\n", this_thing->name->str_text);
    indent_printf("%s_free,\n", this_thing->name->str_text);
    indent_printf("0, /* enum_parse */\n");
    indent_printf("%s_parse,\n", this_thing->name->str_text);
    indent_printf("0, /* struct_parse */\n");
    indent_printf("0, /* fuzzy */\n");
    indent_printf("%s_convert,\n", this_thing->name->str_text);
    indent_printf("generic_struct_is_set,\n");
    indent_printf("};\n");
}


static void
gen_code_declarator(type_ty *type, string_ty *variable_name, int is_a_list,
    int show)
{
    indent_printf("%s_write(fp, ", type->name->str_text);
    if (is_a_list)
       	indent_printf("\"\"");
    else
       	indent_printf("\"%s\"", variable_name->str_text);
    indent_printf(", this_thing->%s);\n", variable_name->str_text);
}


static void
gen_code_call_xml(type_ty *type, string_ty *form_name, string_ty *member_name,
    int show)
{
    indent_printf
    (
	"%s_write_xml(fp, \"%s\", this_thing->%s);\n",
	type->name->str_text,
	form_name->str_text,
	member_name->str_text
    );
}


static void
gen_free_declarator(type_ty *type, string_ty *variable_name, int is_a_list)
{
    if (type->included_flag)
    {
	indent_printf
	(
    	    "%s_type.free(this_thing->%s);\n",
    	    type->name->str_text,
    	    variable_name->str_text
	);
    }
    else
    {
	indent_printf
	(
    	    "%s_free(this_thing->%s);\n",
    	    type->name->str_text,
    	    variable_name->str_text
	);
    }
}


static void
member_add(type_ty *type, string_ty *member_name, type_ty *member_type,
    int show)
{
    type_list_ty    *this_thing;

    this_thing = (type_list_ty *)type;
    this_thing->subtype = member_type;
}


static void
in_include_file(type_ty *type)
{
    type_list_ty    *this_thing;

    this_thing = (type_list_ty *)type;
    type_in_include_file(this_thing->subtype);
}


type_method_ty type_list =
{
    sizeof(type_list_ty),
    "list",
    0, /* has a mask NOT */
    0, /* constructor */
    0, /* destructor */
    gen_include,
    gen_include_declarator,
    gen_code,
    gen_code_declarator,
    gen_code_call_xml,
    gen_free_declarator,
    member_add,
    in_include_file,
};

//
// aegis - project change supervisor
// Copyright (C) 2008, 2011 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/trace.h>

#include <fmtgen/type/top_level/code.h>
#include <fmtgen/type/vector.h>


type_top_level_code::~type_top_level_code()
{
}


type_top_level_code::type_top_level_code(generator *a_gen,
        const pointer &a_subtype) :
    type_top_level(a_gen, a_subtype)
{
}


type_top_level_code::pointer
type_top_level_code::create(generator *a_gen, const pointer &a_subtype)
{
    return pointer(new type_top_level_code(a_gen, a_subtype));
}


void
type_top_level_code::gen_body()
    const
{
    trace(("type_top_level_code::gen_body()\n"));
    include_once("common/trace.h");

    nstring cp1 = def_name();
    nstring top_level_name = def_name();
    printf("\n");
    printf("%s_ty *\n", cp1.c_str());
    printf
    (
        "%s_read_file(const nstring &filename)\n",
        top_level_name.c_str()
    );
    printf("{\n");
    printf
    (
        "return %s_read_file(filename.get_ref());\n",
        top_level_name.c_str()
    );
    printf("}\n");

    include_once("common/trace.h");
    include_once("libaegis/meta_parse.h");
    include_once("libaegis/os.h");
    printf("\n");
    printf("%s_ty *\n", cp1.c_str());
    printf("%s_read_file(string_ty *filename)\n", top_level_name.c_str());
    printf("{\n");
    printf("%s_ty *result;\n\n", cp1.c_str());
    printf
    (
        "trace((\"%s_read_file(filename = \\\"%%s\\\")\\n{\\n\", "
            "(filename ? filename->str_text : \"\")));\n",
        cp1.c_str()
    );
    printf("os_become_must_be_active();\n");
    printf
    (
        "result = (%s_ty *)parse(filename, &%s_type);\n",
        cp1.c_str(),
        top_level_name.c_str()
    );
    printf("trace((\"return %%p;\\n\", result));\n");
    printf("trace((\"}\\n\"));\n");
    printf("return result;\n");
    printf("}\n");

    printf("\n");
    printf("void\n");
    printf
    (
        "%s_write_file(const nstring &filename, %s_ty *value, "
            "bool comp)\n",
        top_level_name.c_str(),
        top_level_name.c_str()
    );
    printf("{\n");
    printf
    (
        "%s_write_file(filename.get_ref(), value, comp);\n",
        top_level_name.c_str()
    );
    printf("}\n");

    include_once("libaegis/io.h");
    include_once("libaegis/output/file.h");
    include_once("libaegis/output/filter/gzip.h");
    include_once("libaegis/output/filter/indent.h");
    printf("\n");
    printf("void\n");
    printf
    (
        "%s_write_file(string_ty *filename, %s_ty *value, "
            "int needs_compression)\n",
        top_level_name.c_str(),
        top_level_name.c_str()
    );
    printf("{\n");
    printf
    (
        "trace((\"%s_write_file(filename = \\\"%%s\\\", value = %%p)\\n"
            "{\\n\", (filename ? filename->str_text : \"\"), value));\n",
        cp1.c_str()
    );
    printf("if (filename)\n");
    indent_more();
    printf("os_become_must_be_active();\n");
    indent_less();
    printf("output::pointer fp;\n");
    printf("if (needs_compression)\n{\n");
    printf("fp = output_file::binary_open(filename);\n");
    printf("fp = output_filter_gzip::create(fp);\n");
    printf("}\nelse\n{\n");
    printf("fp = output_file::text_open(filename);\n");
    printf("}\n");
    printf("fp = output_filter_indent::create(fp);\n");
    printf("io_comment_emit(fp);\n");
    printf("%s_write(fp, value);\n", top_level_name.c_str());
    printf("type_enum_option_clear();\n");
    printf("trace((\"}\\n\"));\n");
    printf("}\n");

    printf("\n");
    printf("void\n");
    printf("%s__rpt_init(void)\n", top_level_name.c_str());
    printf("{\n");
    printf("trace((\"%s__rpt_init()\\n{\\n\"));\n", cp1.c_str());

    //
    // find the list of types we care about.
    // generate report initialization for them.
    //
    type_vector unique_types;
    unique_types.push_back(subtype);
    unique_types.reachable_closure();
    unique_types.gen_report_initializations();

    printf("trace((\"}\\n\"));\n");
    printf("}\n");

    trace(("}\n"));
}

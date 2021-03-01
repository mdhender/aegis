//
// aegis - project change supervisor
// Copyright (C) 2008 Peter Miller
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

#include <fmtgen/type/top_level/include.h>


type_top_level_include::~type_top_level_include()
{
}


type_top_level_include::type_top_level_include(generator *a_gen,
        const pointer &a_subtype) :
    type_top_level(a_gen, a_subtype)
{
}


type_top_level_include::pointer
type_top_level_include::create(generator *a_gen, const pointer &a_subtype)
{
    return pointer(new type_top_level_include(a_gen, a_subtype));
}


void
type_top_level_include::gen_body()
    const
{
    trace(("type_top_level_include::gen_body()\n{\n"));
    include_once("common/str.h");
    printf("\n");
    printf("/**\n");
    printf("  * The %s_write_file function is used to\n", def_name().c_str());
    printf("  * write %s meta data to the named file.\n", def_name().c_str());
    printf("  *\n");
    printf("  * @param filename\n");
    printf("  *     The name of the file to be written.\n");
    printf("  * @param value\n");
    printf("  *     The value of the meta-data to be written.\n");
    printf("  * @param comp\n");
    printf("  *     true (non-zero) if data should be compressed.\n");
    printf("  * @note\n");
    printf("  *      If any errors are encountered, this\n");
    printf("  *      function will not return.  All errors\n");
    printf("  *      will print a fatal error message, and\n");
    printf("  *      exit with an exit status of 1.\n");
    printf("  */\n");
    printf
    (
        "void %s_write_file(string_ty *filename, %s_ty *value, int comp);\n",
        def_name().c_str(),
        def_name().c_str()
    );

    include_once("common/nstring.h");
    printf("\n");
    printf("/**\n");
    printf("  * The %s_write_file function is used to\n", def_name().c_str());
    printf("  * write %s meta data to the named file.\n", def_name().c_str());
    printf("  *\n");
    printf("  * @param filnam\n");
    printf("  *     The name of the file to be written.\n");
    printf("  * @param value\n");
    printf("  *     The value of the meta-data to be written.\n");
    printf("  * @param comp\n");
    printf("  *     true if data should be compressed.\n");
    printf("  * @note\n");
    printf("  *      If any errors are encountered, this\n");
    printf("  *      function will not return.  All errors\n");
    printf("  *      will print a fatal error message, and\n");
    printf("  *      exit with an exit status of 1.\n");
    printf("  */\n");
    printf
    (
        "void %s_write_file(const nstring &filnam, %s_ty *value, bool comp);\n",
        def_name().c_str(),
        def_name().c_str()
    );

    printf("\n");
    printf("/**\n");
    printf("  * The %s_read_file function is used to\n", def_name().c_str());
    printf("  * read %s meta data from the named file.\n", def_name().c_str());
    printf("  *\n");
    printf("  * @param filename\n");
    printf("  *     The name of the file to be read.\n");
    printf("  * @returns\n");
    printf("  *     a pointer to a dynamically allocated\n");
    printf("  *     value read from the file.\n");
    printf("  * @note\n");
    printf("  *      If any errors are encountered, this\n");
    printf("  *      function will not return.  All errors\n");
    printf("  *      (including syntax errors) will print a\n");
    printf("  *      fatal error message, and exit with an\n");
    printf("  *      exit status of 1.\n");
    printf("  */\n");
    printf
    (
        "%s_ty *%s_read_file(string_ty *filename);\n",
        def_name().c_str(),
        def_name().c_str()
    );

    printf("\n");
    printf("/**\n");
    printf("  * The %s_read_file function is used to\n", def_name().c_str());
    printf("  * read %s meta data from the named file.\n", def_name().c_str());
    printf("  *\n");
    printf("  * @param filename\n");
    printf("  *     The name of the file to be read.\n");
    printf("  * @returns\n");
    printf("  *     a pointer to a dynamically allocated\n");
    printf("  *     value read from the file.\n");
    printf("  * @note\n");
    printf("  *      If any errors are encountered, this\n");
    printf("  *      function will not return.  All errors\n");
    printf("  *      (including syntax errors) will print a\n");
    printf("  *      fatal error message, and exit with an\n");
    printf("  *      exit status of 1.\n");
    printf("  */\n");
    printf
    (
        "%s_ty *%s_read_file(const nstring &filename);\n",
        def_name().c_str(),
        def_name().c_str()
    );

    printf("void %s__rpt_init(void);\n", def_name().c_str());
    trace(("}\n"));
}

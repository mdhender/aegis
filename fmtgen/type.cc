//
// aegis - project change supervisor
// Copyright (C) 1991-1994, 1998, 2002-2008, 2012 Peter Miller
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
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>

#include <common/error.h>
#include <common/mem.h>
#include <common/trace.h>

#include <fmtgen/generator.h>
#include <fmtgen/lex.h>
#include <fmtgen/type.h>


bool type::use_bit_fields = true;


type::~type()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    gen = 0;
}


type::type(generator *a_gen, const nstring &a_name, bool a_global) :
    gen(a_gen),
    name(a_name),
    is_a_typedef(false),
    included_flag(lex_in_include_file() && a_global)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    trace(("..name = %s\n", name.quote_c().c_str()));
    trace(("..a_global = %d\n", a_global));
    trace(("..included_flag = %d\n", included_flag));
    assert(gen);
    assert(!name.empty());
}


void
type::gen_declarator(const nstring &, bool, int, const nstring &)
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    trace(("name = %s\n", name.quote_c().c_str()));
    // Do nothing.
}


void
type::gen_body()
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    trace(("name = %s\n", name.quote_c().c_str()));
    // Do nothing.
}


void
type::gen_call_xml(const nstring &, const nstring &, int)
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    trace(("name = %s\n", name.quote_c().c_str()));
    // Do nothing.
}


void
type::gen_free_declarator(const nstring &, bool)
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    trace(("name = %s\n", name.quote_c().c_str()));
    // Do nothing.
}


void
type::member_add(const nstring &, const type::pointer &, int, const nstring &)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    //
    // This should only be called for structure members, and
    // type_structure_ty overloads it.  Any other call is wrong.
    //
    assert(0);
}


void
type::in_include_file()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    trace(("name = %s\n", name.quote_c().c_str()));
    included_flag = true;
}


void
type::toplevel()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    trace(("name = %s\n", name.quote_c().c_str()));
    included_flag = false;
}


void
type::gen_copy(const nstring &)
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    trace(("name = %s\n", name.quote_c().c_str()));
    // Do nothing.
}


void
type::gen_report_initializations()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    trace(("name = %s\n", name.quote_c().c_str()));
    // Do nothing.
}


void
type::gen_trace(const nstring &, const nstring &)
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    trace(("name = %s\n", name.quote_c().c_str()));
    // Do nothing.
}


void
type::gen_default_constructor(const nstring &inst_var_name)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    wrap_and_print
    (
        "// ",
        "method " + nstring(__PRETTY_FUNCTION__).quote_c() +
        " does nothing for type " + def_name().quote_c() +
        " variable " + inst_var_name.quote_c()
    );
}


void
type::gen_copy_constructor(const nstring &inst_var_name)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    printf("%s(rhs.%s)", inst_var_name.c_str(), inst_var_name.c_str());
}


void
type::gen_assignment_operator(const nstring &inst_var_name)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    printf("%s = rhs.%s;\n", inst_var_name.c_str(), inst_var_name.c_str());
}


void
type::printf(const char *fmt, ...)
    const
{
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
}


void
type::vprintf(const char *fmt, va_list ap)
    const
{
    assert(gen);
    gen->vprintf(fmt, ap);
}


void
type::wrap_and_print(const nstring &text)
    const
{
    assert(gen);
    gen->wrap_and_print("", text);
}


void
type::wrap_and_print(const nstring &prefix, const nstring &text)
    const
{
    assert(gen);
    gen->wrap_and_print(prefix, text);
}


void
type::indent_more()
    const
{
    assert(gen);
    gen->indent_more();
}


void
type::indent_less()
    const
{
    assert(gen);
    gen->indent_less();
}


void
type::include_once(const nstring &filename)
    const
{
    assert(gen);
    gen->include_once(filename);
}


void
type::get_reachable(type_vector &)
    const
{
    // no subtypes by default, so nothing to do by default.
    trace(("name = %s\n", name.quote_c().c_str()));
}


bool
type::is_in_include_file()
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    trace(("..name = %s\n", name.quote_c().c_str()));
    trace(("..return %s\n", (included_flag ? "true" : "false")));
    return included_flag;
}


void
type::comment_set(const nstring &text)
{
    comment = text;
}


void
type::gen_methods(const nstring &, const nstring &, int)
    const
{
    wrap_and_print
    (
        "// ",
        "method " + nstring(__PRETTY_FUNCTION__) +
        " does nothing for " + def_name()
    );
}


void
type::gen_write(const nstring &, const nstring &, bool)
    const
{
    wrap_and_print
    (
        "// ",
        "method " + nstring(__PRETTY_FUNCTION__) +
        " does nothing for " + def_name()
    );
}


nstring
type::default_instance_variable_comment(const nstring &inst_var_name,
    bool is_a_list) const
{

    nstring adjective = def_name().replace("_", " ");
    nstring noun = inst_var_name.replace("_", " ");
    // assume it will be passed to the wrap_and_print method, later.
    if (is_a_list)
        return
            (
                "The " + inst_var_name + " instance variable is\n"
                "used to remember the base address of a dynamically\n"
                "allocated array of " + adjective + " values.\n"
            );
    return
        (
            "The " + inst_var_name + " instance variable is used to\n"
            "remember the " + adjective + " " + noun + ".\n"
        );
}


void
type::set_bit_fields(bool yesno)
{
    use_bit_fields = yesno;
}


// vim: set ts=8 sw=4 et :

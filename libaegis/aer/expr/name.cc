//
// aegis - project change supervisor
// Copyright (C) 1994-1997, 1999, 2000, 2002, 2004-2008, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published
// by the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
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

#include <common/mem.h>
#include <common/symtab/template.h>
#include <common/trace.h>
#include <libaegis/aer/expr/constant.h>
#include <libaegis/aer/expr/name.h>
#include <libaegis/aer/func.h>
#include <libaegis/aer/lex.h>
#include <libaegis/aer/parse.h>
#include <libaegis/aer/value/boolean.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/group.h>
#include <libaegis/aer/value/gstate.h>
#include <libaegis/aer/value/null.h>
#include <libaegis/aer/value/passwd.h>
#include <libaegis/aer/value/ref.h>
#include <libaegis/aer/value/uconf.h>
#include <libaegis/cattr.fmtgen.h>
#include <libaegis/common.fmtgen.h>
#include <libaegis/cstate.fmtgen.h>
#include <libaegis/fstate.fmtgen.h>
#include <libaegis/gonzo.h>
#include <libaegis/pattr.fmtgen.h>
#include <libaegis/pconf.fmtgen.h>
#include <libaegis/pstate.fmtgen.h>
#include <libaegis/sub.h>
#include <libaegis/uconf.fmtgen.h>
#include <libaegis/ustate.fmtgen.h>


static symtab<rpt_value::pointer> symbol_table;


static void
init(void)
{
    if (!symbol_table.empty())
        return;
    trace(("%s\n", __PRETTY_FUNCTION__));

    //
    // initialize the names of the builtin functions
    //
    rpt_func::init(symbol_table);

    //
    // pull values from fmtgen
    //
    cattr__rpt_init();
    common__rpt_init();
    cstate__rpt_init();
    fstate__rpt_init();
    gstate__rpt_init();
    pattr__rpt_init();
    pconf__rpt_init();
    pstate__rpt_init();
    uconf__rpt_init();
    ustate__rpt_init();

    //
    // some constants
    //
    symbol_table.assign("true", rpt_value_boolean::create(true));
    symbol_table.assign("false", rpt_value_boolean::create(false));

    //
    // This one is so you can get at .aegisrc files.
    //
    symbol_table.assign("user", rpt_value_uconf::create());

    symbol_table.assign("passwd", rpt_value_passwd::create());
    symbol_table.assign("group", rpt_value_group::create());
    symbol_table.assign("project", rpt_value_gstate::create());

    //
    // the "arg" variable, containing the
    // strings specified on the command line.
    //
    report_parse__init_arg();
}


void
rpt_expr_name__init(const nstring &name, const rpt_value::pointer &value)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    symbol_table.assign(name, value);
}


rpt_expr::pointer
rpt_expr_name(const nstring &name)
{
    if (symbol_table.empty())
        init();

    trace(("%s\n", __PRETTY_FUNCTION__));
    rpt_value::pointer data = symbol_table.get(name);
    if (!data)
    {
        nstring name2 = symbol_table.query_fuzzy(name);
        if (name2.empty())
        {
            sub_context_ty sc;
            sc.var_set_string("Name", name);
            aer_lex_error(sc, i18n("the name \"$name\" is undefined"));
            data = rpt_value_null::create();
        }
        else
        {
            sub_context_ty sc;
            sc.var_set_string("Name", name);
            sc.var_set_string("Guess", name2);
            aer_lex_error(sc, i18n("no \"$name\", guessing \"$guess\""));
            data = symbol_table.get(name2);
            assert(data);
        }
    }

    return rpt_expr_constant::create(data);
}


void
rpt_expr_name__declare(const nstring &name)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    trace(("name = \"%s\"\n", name.c_str()));
    //
    // make sure the name is unique
    //
    if (symbol_table.empty())
        init();
    if (symbol_table.query(name))
    {
        sub_context_ty sc;
        sc.var_set_string("Name", name);
        aer_lex_error(sc, i18n("the name \"$name\" has already been used"));
        return;
    }

    //
    // create the value to be a reference to nul
    //  (it is a variable, it must be a reference to something)
    //
    trace(("name is new\n"));
    rpt_value::pointer v1 = rpt_value_null::create();
    rpt_value::pointer v2 = rpt_value_reference::create(v1);
    symbol_table.assign(name, v2);
    trace(("assigned nul\n"));
}


// vim: set ts=8 sw=4 et :

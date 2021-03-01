//
// aegis - project change supervisor
// Copyright (C) 1999, 2000, 2003-2008, 2011, 2012 Peter Miller
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
#include <common/ac/stdio.h>

#include <common/nstring/list.h>
#include <common/str_list.h>
#include <common/trace.h>
#include <libaegis/change/env_set.h>
#include <libaegis/change.h>
#include <libaegis/os.h>
#include <libaegis/project/file.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>


static int
run_test_command(change::pointer cp, user_ty::pointer up, string_ty *filename,
    string_ty *dir, int inp, string_ty *the_command, int bl,
    const nstring_list &variable_assignments)
{
    trace(("run_test_command(cp = %p, up = %p, filename = %s, "
        "dir = %s, inp = %d, the_command = %s, bl = %d)\n{\n", cp,
        up.get(), nstring(filename).quote_c().c_str(),
        nstring(dir).quote_c().c_str(), inp,
        nstring(the_command).quote_c().c_str(), bl));
    assert(cp->reference_count >= 1);
    sub_context_ty sc;
    sc.var_set_string("File_Name", filename);

    // Quote the variable assignments
    nstring_list var;
    for (size_t jj = 0; jj < variable_assignments.size(); ++jj)
        var.push_back(variable_assignments[jj].quote_shell());
    sc.var_set_string("VARiables", var.unsplit());
    sc.var_append_if_unused("VARiables");

    if (bl && !cp->bogus)
    {
        nstring_list spbl;
        cp->pp->search_path_get(spbl, false);
        nstring s = spbl.unsplit(":");
        sc.var_set_string("Search_Path_Executable", s);
        sc.var_override("Search_Path_Executable");
        sc.var_optional("Search_Path_Executable");
    }
    the_command = sc.substitute(cp, the_command);

    int flags = inp ? OS_EXEC_FLAG_INPUT : OS_EXEC_FLAG_NO_INPUT;
    change_env_set(cp, 1);
    user_ty::become scoped(up);
    int result = os_execute_retcode(the_command, flags, dir);
    str_free(the_command);
    trace(("return %d;\n", result));
    trace(("}\n"));
    return result;
}


int
change_run_test_command(change::pointer cp, user_ty::pointer up,
    string_ty *filename, string_ty *dir, int inp, int bl,
    const nstring_list &variable_assignments)
{
    trace(("change_run_test_command(cp = %p, up = %p, filename = %s, "
        "dir = %s, inp = %d, bl = %d)\n{\n", cp,
        up.get(), nstring(filename).quote_c().c_str(),
        nstring(dir).quote_c().c_str(), inp, bl));
    assert(cp->reference_count >= 1);
    pconf_ty *pconf_data = change_pconf_get(cp, 0);
    assert(pconf_data);
    string_ty *the_command = pconf_data->test_command;
    assert(the_command);
    int result =
        run_test_command
        (
            cp,
            up,
            filename,
            dir,
            inp,
            the_command,
            bl,
            variable_assignments
        );
    trace(("return %d;\n", result));
    trace(("}\n"));
    return result;
}


int
change_run_development_test_command(change::pointer cp, user_ty::pointer up,
    string_ty *filename, string_ty *dir, int inp, int bl,
    const nstring_list &variable_assignments)
{
    trace(("change_run_development_test_command(cp = %p, up = %p, "
        "filename = %s, dir = %s, inp = %d, bl = %d)\n{\n", cp,
        up.get(), nstring(filename).quote_c().c_str(),
        nstring(dir).quote_c().c_str(), inp, bl));
    pconf_ty        *pconf_data;
    string_ty       *the_command;

    assert(cp->reference_count >= 1);
    pconf_data = change_pconf_get(cp, 0);
    assert(pconf_data);
    the_command = pconf_data->development_test_command;
    assert(the_command);
    int result =
        run_test_command
        (
            cp,
            up,
            filename,
            dir,
            inp,
            the_command,
            bl,
            variable_assignments
        );
    trace(("return %d;\n", result));
    trace(("}\n"));
    return result;
}


// vim: set ts=8 sw=4 et :

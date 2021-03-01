//
//	aegis - project change supervisor
//	Copyright (C) 2004-2006, 2008 Peter Miller
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

#include <libaegis/file.h>
#include <libaegis/os.h>
#include <aecvsserver/server.h>
#include <common/trace.h>


int
server_execute(server_ty *sp, string_ty *cmd)
{
    //
    // Run the command.
    //
    trace(("server_execute()\n{\n"));
    os_become_orig();
    int flags = OS_EXEC_FLAG_SILENT;
    string_ty *dir = os_curdir();
    nstring s1(os_edit_filename(0));
    trace_nstring(s1);
    nstring s2 = nstring::format("( %s ) > %s 2>&1", cmd->str_text, s1.c_str());
    int result = os_execute_retcode(s2.get_ref(), flags, dir);
    s2 = read_whole_file(s1);
    os_unlink(s1);
    trace_nstring(s2);
    os_become_undo();

    //
    // If there was any output, send suitable diagnostic messages to
    // the client.
    //
    if (result)
    {
	server_error
	(
	    sp,
	    "%s\n%s\nexit status: %d",
	    cmd->str_text,
	    s2.c_str(),
	    result
	);
    }
    else if (!s2.empty())
    {
	server_e(sp, "%s\n%s", cmd->str_text, s2.c_str());
    }

    //
    // Report the exit status to the caller:
    // false (zero) on failure, true (non-zero) on success.
    //
    trace(("return %d;\n", (result == 0)));
    trace(("}\n"));
    return (result == 0);
}

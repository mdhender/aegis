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
// MANIFEST: functions to manipulate runs
//

#include <file.h>
#include <os.h>
#include <server.h>
#include <trace.h>


int
server_execute(server_ty *sp, string_ty *cmd)
{
    string_ty       *s1;
    string_ty       *s2;
    int             flags;
    string_ty       *dir;
    int             result;

    //
    // Run the command.
    //
    trace(("server_execute()\n{\n"));
    os_become_orig();
    flags = OS_EXEC_FLAG_SILENT;
    dir = os_curdir();
    s1 = os_edit_filename(0);
    trace_string(s1->str_text);
    s2 = str_format("( %s ) > %s 2>&1", cmd->str_text, s1->str_text);
    result = os_execute_retcode(s2, flags, dir);
    str_free(s2);
    s2 = read_whole_file(s1);
    os_unlink(s1);
    str_free(s1);
    trace_string(s2->str_text);
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
	    s2->str_text,
	    result
	);
    }
    else if (s2->str_length)
    {
	server_e(sp, "%s\n%s", cmd->str_text, s2->str_text);
    }
    str_free(s2);

    //
    // Report the exit status to the caller:
    // false (zero) on failure, true (non-zero) on success.
    //
    trace(("return %d;\n", (result == 0)));
    trace(("}\n"));
    return (result == 0);
}

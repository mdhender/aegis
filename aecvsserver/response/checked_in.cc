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
//
// Checked-in /client/side/dir/
// /server/side/file/<name>
// /<name>/version///
//
// This means a file has been successfully operated on (checked in,
// added, etc.).  The <name> in the Entries line (an argument to the
// command generating this repsonse) is the same as the last component
// of /server/side/file/<name>.
//

#include <common/ac/string.h>

#include <libaegis/os.h>
#include <libaegis/output.h>
#include <aecvsserver/response/checked_in.h>
#include <aecvsserver/server.h>


response_checked_in::~response_checked_in()
{
    str_free(client_side);
    client_side = 0;
    str_free(server_side);
    server_side = 0;
    str_free(version);
    version = 0;
}


response_checked_in::response_checked_in(string_ty *arg1,
    string_ty *arg2, int arg3, string_ty *arg4) :
    client_side(str_copy(arg1)),
    server_side(str_copy(arg2)),
    mode(arg3),
    version(str_copy(arg4))
{
}


void
response_checked_in::write(output::pointer op)
{
    //
    // The output looks something like this...
    //
    // S: Mode u=rw,g=r,o=r
    // S: Checked-in ./
    // S: /u/cvsroot/supermunger/mungeall.c
    // S: /mungeall.c/1.1///
    //
    string_ty *short_dir_name = os_dirname_relative(client_side);
    string_ty *short_file_name = os_entryname_relative(client_side);
    op->fprintf("Mode ");
    output_mode_string(op, mode);
    op->fprintf("Checked-in %s/\n", short_dir_name->str_text);
    op->fprintf(ROOT_PATH "/%s\n", server_side->str_text);
    op->fprintf
    (
	"/%s/%s///\n",
	short_file_name->str_text,
	version->str_text
    );
    str_free(short_dir_name);
    str_free(short_file_name);
}


response_code_ty
response_checked_in::code_get()
    const
{
    return response_code_Checked_in;
}

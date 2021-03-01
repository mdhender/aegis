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
// Removed <client-side-dir>/
// <server-side-dir>/<filename>
//
// The file has been removed from the repository.  The file needs its
// entry removed from CVS/Entries and it needs to be removed from the
// client's directory.  This is the case where cvs prints "file foobar.c
// is no longer pertinent".
//

#include <libaegis/os.h>
#include <libaegis/output.h>
#include <aecvsserver/response/removed.h>
#include <aecvsserver/server.h>


response_removed::~response_removed()
{
    str_free(client_side);
    client_side = 0;
    str_free(server_side);
    server_side = 0;
}


response_removed::response_removed(string_ty *arg1, string_ty *arg2) :
    client_side(str_copy(arg1)),
    server_side(str_copy(arg2))
{
}


void
response_removed::write(output::pointer op)
{
    //
    // The output looks something like this...
    //
    // S: M file mungeall.c is no longer pertinent
    // S: Removed munger/
    // S: /u/cvsroot/super/munger/mungeall.c
    //
    string_ty *short_dir_name = os_dirname_relative(client_side);
    string_ty *short_file_name = os_entryname_relative(client_side);
    op->fprintf
    (
	"M file %s is no longer pertinent\n"
	"Removed %s/\n"
	ROOT_PATH "/%s\n",
	short_file_name->str_text,
	short_dir_name->str_text,
	server_side->str_text
    );
    str_free(short_dir_name);
    str_free(short_file_name);
}


response_code_ty
response_removed::code_get()
    const
{
    return response_code_Removed;
}

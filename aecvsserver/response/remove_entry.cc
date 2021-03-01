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
// Remove-entry <pathname>
// <reposname>
//
// The file needs its entry removed from CVS/Entries, but the file
// itself is already gone (this happens in response to a "ci" request
// which involves committing the removal of a file).
//

#include <libaegis/os.h>
#include <libaegis/output.h>
#include <aecvsserver/response/remove_entry.h>
#include <aecvsserver/server.h>


response_remove_entry::~response_remove_entry()
{
    str_free(client_side);
    client_side = 0;
    str_free(server_side);
    server_side = 0;
}


response_remove_entry::response_remove_entry(string_ty *arg1, string_ty *arg2) :
    client_side(str_copy(arg1)),
    server_side(str_copy(arg2))
{
}


void
response_remove_entry::write(output::pointer op)
{
    //
    // The output looks something like this...
    //
    // S: Remove-entry ./
    // S: /u/cvsroot/supermunger/mungeall.c
    //
    string_ty *short_dir_name = os_dirname_relative(client_side);
    op->fprintf("Remove-entry %s/\n", short_dir_name->str_text);
    op->fprintf(ROOT_PATH "/%s\n", server_side->str_text);
    str_free(short_dir_name);
}


response_code_ty
response_remove_entry::code_get()
    const
{
    return response_code_Remove_entry;
}

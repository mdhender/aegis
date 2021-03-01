//
//	aegis - project change supervisor
//	Copyright (C) 2005-2008 Peter Miller
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
//	along with this program.  If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <common/ac/stdio.h>

#include <common/libdir.h>
#include <common/nstring.h>
#include <common/str_list.h>
#include <libaegis/file.h>
#include <libaegis/http.h>
#include <libaegis/os.h>


void
get_icon(string_ty *path, string_list_ty *)
{
    //
    // Build the path to the physical file.
    //
    nstring from =
	nstring::format("%s/%s", configured_datadir(), path->str_text);

    //
    // Print the output header.
    //
    // We use the "from" name (even though it could be an uninformative
    // name in /tmp) so that the file can be opened and scanned.
    //
    // No need for a Content-Length header, because fork_and_watch will
    // add it automagically.
    //
    http_content_type_header(from.get_ref());
    printf("\n");

    //
    // It is very important to flush the standard output at this point,
    // because copy_whole_file is going to write on file descriptor
    // zero, completely bypassing the stdio buffering.
    //
    fflush(stdout);

    //
    // Now copy the file contents to stdout.
    //
    nstring to;
    os_become_orig();
    copy_whole_file(from.get_ref(), to.get_ref(), 0);
    os_become_undo();
}

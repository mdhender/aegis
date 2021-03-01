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
// MANIFEST: functions to manipulate modifieds
//
//
// Modified <filename>
//
// Additional data: <mode>
// Additiona data: <file transmission>
//
// Send the server a copy of one locally modified file.  The <filename>
// given is a file within the most recent directory sent with Directory;
// it must not contain "/".  If the user is operating on only some files
// in a directory, only those files need to be included.  This can also
// be sent without Entry, if there is no entry for the file.
//
// Commands for which Modified is necessary are co, ci, update,
// and import.
//
// Response expected: no.
// Root required: yes.
//

#include <ac/string.h>

#include <error.h>
#include <file_info.h>
#include <input.h>
#include <module.h>
#include <os.h>
#include <request/modified.h>
#include <response/error.h>
#include <server.h>


static void
extract_project_and_filename(string_ty *from, string_ty **project_name_p,
    string_ty **file_name_p)
{
    const char      *ep;

    //
    // The server-side path is originally specified as ``ROOT_PATH "/"
    // project "/" filename'', but the command processing has already
    // stripped the ``ROOT_PATH "/"'' from the string, leaving ``project
    // "/" filename''.  See aecvsserver/request/directory.c for where
    // this is done.
    //
    if (from->str_length == 0)
    {
	*project_name_p = str_from_c("");
	*file_name_p = str_from_c(".");
	return;
    }
    assert(from->str_text[0] != '/');
    ep = strchr(from->str_text, '/');
    if (!ep)
    {
	*project_name_p = str_copy(from);
	*file_name_p = str_from_c(".");
	return;
    }
    *project_name_p = str_n_from_c(from->str_text, ep - from->str_text);
    while (*ep == '/')
	++ep;
    if (*ep)
	*file_name_p = str_n_from_c(ep, from->str_text + from->str_length - ep);
    else
	*file_name_p = str_from_c(".");
}


static void
run(server_ty *sp, string_ty *arg)
{
    int             mode;
    input_ty        *ip;
    string_ty       *server_side;
    directory_ty    *dp;
    string_ty       *project_name;
    string_ty       *file_name;
    module_ty       *mp;
    file_info_ty    *fip;

    assert(sp);
    assert(sp->np);
    if (server_root_required(sp, "Modified"))
	return;
    if (server_directory_required(sp, "Modified"))
	return;

    mode = server_file_mode_get(sp);
    if (mode < 0)
    {
	// error already sent
	return;
    }

    dp = sp->np->curdir;
    server_side = os_path_cat(dp->server_side, arg);
    fip = server_file_info_find(sp, server_side, 1);
    fip->mode = mode;
    fip->modified = 1;

    //
    // Extract the module name and filename from the path.
    //
    extract_project_and_filename(server_side, &project_name, &file_name);
    str_free(server_side);

    //
    // Get an input stream for reading the file contents from.  Any input
    // the method choses to ignore will be thrown away, so as to ensure
    // correct sync with the input stream.
    //
    ip = server_file_contents_get(sp);

    //
    // Find the module to work on.
    //
    mp = module_find(project_name);
    str_free(project_name);

    //
    // Have the module figure out what to do with the request.
    //
    module_modified(mp, sp, file_name, fip, ip);
    str_free(file_name);
    input_delete(ip);
    module_delete(mp);
}


const request_ty request_modified =
{
    "Modified",
    run,
    0, // no reset
};

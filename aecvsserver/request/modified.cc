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

#include <common/ac/string.h>

#include <common/error.h>
#include <aecvsserver/file_info.h>
#include <libaegis/input.h>
#include <aecvsserver/module.h>
#include <libaegis/os.h>
#include <aecvsserver/request/modified.h>
#include <aecvsserver/response/error.h>
#include <aecvsserver/server.h>


request_modified::~request_modified()
{
}


request_modified::request_modified()
{
}


static void
extract_project_and_filename(string_ty *from, string_ty **project_name_p,
    string_ty **file_name_p)
{
    const char      *ep;

    //
    // The server-side path is originally specified as "ROOT_PATH /
    // project / filename", but the command processing has already
    // stripped the "ROOT_PATH /" from the string, leaving "project
    // / filename".  See aecvsserver/request/directory.c for where
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


void
request_modified::run_inner(server_ty *sp, string_ty *arg)
    const
{
    int             mode;
    string_ty       *server_side;
    directory_ty    *dp;
    string_ty       *project_name;
    string_ty       *file_name;
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

    dp = sp->np->get_curdir();
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
    input ip = server_file_contents_get(sp);

    //
    // Find the module to work on.
    //
    module mp = module::find(project_name);
    str_free(project_name);

    //
    // Have the module figure out what to do with the request.
    //
    mp->modified(sp, file_name, fip, ip);
    str_free(file_name);
}


const char *
request_modified::name()
    const
{
    return "Modified";
}


bool
request_modified::reset()
    const
{
    return false;
}

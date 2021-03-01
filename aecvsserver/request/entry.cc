//
// aegis - project change supervisor
// Copyright (C) 2004-2006, 2008, 2012 Peter Miller
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
// //
// Entry <entry-line>
//
// Tell the server what version of a file is on the local machine.
// The name in <entry-line> is a name relative to the directory most
// recently specified with Directory.  If the user is operating on only
// some files in a directory, Entry requests for only those files need be
// included.  If an Entry request is sent without Modified, Is-modified,
// or Unchanged, it means the file is lost (does not exist in the
// working directory).  If both Entry and one of Modified, Is-modified,
// or Unchanged are sent for the same file, Entry must be sent first.
// For a given file, one can send Modified, Is-modified, or Unchanged,
// but not more than one of these three.
//
// Response expected: no.
// Root required: yes.
//

#include <common/ac/assert.h>

#include <libaegis/os.h>

#include <aecvsserver/file_info.h>
#include <aecvsserver/response/error.h>
#include <aecvsserver/request/entry.h>
#include <aecvsserver/server.h>


request_entry::~request_entry()
{
}


request_entry::request_entry()
{
}


void
request_entry::run_inner(server_ty *sp, string_ty *arg)
    const
{
    string_ty       *filename;

    assert(sp);
    assert(sp->np);
    if (server_root_required(sp, "Entry"))
        return;
    if (server_directory_required(sp, "Entry"))
        return;

    //
    // split out the filename portion of the Entry line.
    //
    filename = str_field(arg, '/', 1);
    if (filename->str_length)
    {
        directory_ty    *dp;
        string_ty       *server_side;
        file_info_ty    *fip;

        //
        // Build the name of the file on the server side.
        //
        dp = sp->np->get_curdir();
        server_side = os_path_cat(dp->server_side, filename);

        //
        // Add the entry to the file info.
        //
        fip = server_file_info_find(sp, server_side, 1);
        fip->version = str_field(arg, '/', 2);
        str_free(server_side);
    }
    str_free(filename);
}


const char *
request_entry::name()
    const
{
    return "Entry";
}


bool
request_entry::reset()
    const
{
    return false;
}


// vim: set ts=8 sw=4 et :

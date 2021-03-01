//
//	aegis - project change supervisor
//	Copyright (C) 2004, 2005 Peter Miller;
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
// MANIFEST: functions to manipulate update_exists
//
//
// Update-existing <pathname>
// <server-side-dir>/<filename>
// /<filename>/<version>///
// <file-mode>
// <length>
// <length-bytes-of-contents>
//
// This is just like Updated and takes the same additional data, but is
// used only if a Entry, Modified, or Unchanged request has been sent
// for the file in question.
//
// This response, or Merged, indicates that the server has determined
// that it is OK to overwrite the previous contents of the file specified
// by pathname.  Provided that the client has correctly sent Modified
// or Is-modified requests for a modified file, and the file was not
// modified while CVS was running, the server can ensure that a user's
// modifications are not lost.
//
//
// The output looks something like this...
//
// S: M U supermunger/mungeall.c
// S: Update-existing supermunger/
// S: /u/cvsroot/supermunger/mungeall.c
// S: /mungeall.c/1.1///
// S: u=rw,g=r,o=r
// S: 26
// S: int mein () { abort (); }
//

#include <ac/string.h>

#include <input.h>
#include <os.h>
#include <output.h>
#include <response/update_exist.h>
#include <response/private.h>


struct response_update_existing_ty
{
    response_ty     inherited;
    string_ty       *client_side;
    string_ty       *server_side;
    input_ty        *source;
    int             mode;
    string_ty       *version;
};


static void
destructor(response_ty *rp)
{
    response_update_existing_ty *rcp;

    rcp = (response_update_existing_ty *)rp;
    str_free(rcp->client_side);
    rcp->client_side = 0;
    str_free(rcp->server_side);
    rcp->server_side = 0;
    input_delete(rcp->source);
    rcp->source = 0;
    str_free(rcp->version);
    rcp->version = 0;
}


static void
write(response_ty *rp, output_ty *op)
{
    response_update_existing_ty *rcp;
    string_ty       *short_dir_name;
    string_ty       *short_file_name;
    long            length;

    //
    // The output looks something like this...
    //
    // S: M U supermunger/mungeall.c
    // S: Update-existing supermunger/
    // S: /u/cvsroot/supermunger/mungeall.c
    // S: /mungeall.c/1.1///
    // S: u=rw,g=r,o=r
    // S: 26
    // S: int mein () { abort (); }
    //
    rcp = (response_update_existing_ty *)rp;
    short_dir_name = os_dirname_relative(rcp->client_side);
    short_file_name = os_entryname_relative(rcp->client_side);
    op->fprintf("M U %s\n", short_file_name->str_text);
    op->fprintf("Update-existing %s/\n", short_dir_name->str_text);
    op->fprintf("/aegis/%s\n", rcp->server_side->str_text);
    op->fprintf
    (
	"/%s/%s///\n",
	short_file_name->str_text,
	rcp->version->str_text
    );
    output_mode_string(op, rcp->mode);
    os_become_orig();
    length = input_length(rcp->source);
    if (length > 0)
    {
	//
	// There is the ability to compress the file contents, but the
	// client has to ask for it.  Slightly difficult: we have to
	// send the *compressed* file size first, which isn't easy using
	// a naive usage of output_gzip().  Just say no.
	//
	op->fprintf("%ld\n", length);
	input_to_output(rcp->source, op);
    }
    else
    {
	op->fputs("0\n");
    }
    os_become_undo();
    str_free(short_dir_name);
    str_free(short_file_name);
}


static const response_method_ty vtbl =
{
    sizeof(response_update_existing_ty),
    destructor,
    write,
    response_code_Update_existing,
    1, // flushable
};


response_ty *
response_update_existing_new(string_ty *client_side, string_ty *server_side,
    input_ty *source, int mode, string_ty *version)
{
    response_ty     *rp;
    response_update_existing_ty *rcp;

    rp = response_new(&vtbl);
    rcp = (response_update_existing_ty *)rp;
    rcp->client_side = str_copy(client_side);
    rcp->server_side = str_copy(server_side);
    rcp->source = source;
    rcp->mode = mode;
    rcp->version = str_copy(version);
    return rp;
}

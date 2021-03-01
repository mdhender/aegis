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
// MANIFEST: functions to manipulate new_entrys
//
//
// New-entry /client/side/dir/
// /server/side/file/<name>
// /<name>/version///
//
// This means a file has been successfully operated on (checked in,
// added, etc.).  The <name> in the Entries line (an argument to the
// command generating this repsonse) is the same as the last component
// of /server/side/file/<name>.
//
//
// Reverse Engineering Notes:
//
//    The cvsclient.texi file fails to mention the server-side path
//    (in fact, omits it from the example), but does say that it is the
//    same as Checked-in response.
//

#include <ac/string.h>

#include <os.h>
#include <output.h>
#include <response/new_entry.h>
#include <response/private.h>
#include <server.h>


struct response_new_entry_ty
{
    response_ty     inherited;
    string_ty       *client_side;
    string_ty       *server_side;
    int             mode;
    string_ty       *version;
};


static void
destructor(response_ty *rp)
{
    response_new_entry_ty *rcp;

    rcp = (response_new_entry_ty *)rp;
    str_free(rcp->client_side);
    rcp->client_side = 0;
    str_free(rcp->server_side);
    rcp->server_side = 0;
    str_free(rcp->version);
    rcp->version = 0;
}


static void
write(response_ty *rp, output_ty *op)
{
    response_new_entry_ty *rcp;
    string_ty       *short_dir_name;
    string_ty       *short_file_name;

    //
    // The output looks something like this...
    //
    // S: Mode u=rw,g=r,o=r
    // S: New-entry ./
    // S: /u/cvsroot/supermunger/mungeall.c
    // S: /mungeall.c/1.1///
    //
    rcp = (response_new_entry_ty *)rp;
    short_dir_name = os_dirname_relative(rcp->client_side);
    short_file_name = os_entryname_relative(rcp->client_side);
    output_fprintf(op, "Mode ");
    output_mode_string(op, rcp->mode);
    output_fprintf(op, "New-entry %s/\n", short_dir_name->str_text);
    output_fprintf(op, ROOT_PATH "/%s\n", rcp->server_side->str_text);
    output_fprintf
    (
	op,
	"/%s/%s///\n",
	short_file_name->str_text,
	rcp->version->str_text
    );
    str_free(short_dir_name);
    str_free(short_file_name);
}


static const response_method_ty vtbl =
{
    sizeof(response_new_entry_ty),
    destructor,
    write,
    response_code_New_entry,
    0, // flushable
};


response_ty *
response_new_entry_new(string_ty *client_side, string_ty *server_side,
    int mode, string_ty *version)
{
    response_ty     *rp;
    response_new_entry_ty *rcp;

    rp = response_new(&vtbl);
    rcp = (response_new_entry_ty *)rp;
    rcp->client_side = str_copy(client_side);
    rcp->server_side = str_copy(server_side);
    rcp->mode = mode;
    rcp->version = str_copy(version);
    return rp;
}

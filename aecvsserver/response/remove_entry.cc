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
// MANIFEST: functions to manipulate remove_entrys
//
//
// Remove-entry <pathname>
// <reposname>
//
// The file needs its entry removed from CVS/Entries, but the file
// itself is already gone (this happens in response to a "ci" request
// which involves committing the removal of a file).
//

#include <os.h>
#include <output.h>
#include <response/remove_entry.h>
#include <response/private.h>
#include <server.h>


typedef struct response_remove_entry_ty response_remove_entry_ty;
struct response_remove_entry_ty
{
    response_ty     inherited;
    string_ty       *client_side;
    string_ty       *server_side;
};


static void
destructor(response_ty *rp)
{
    response_remove_entry_ty *rcp;

    rcp = (response_remove_entry_ty *)rp;
    str_free(rcp->client_side);
    rcp->client_side = 0;
    str_free(rcp->server_side);
    rcp->server_side = 0;
}


static void
write(response_ty *rp, output_ty *op)
{
    response_remove_entry_ty *rcp;
    string_ty       *short_dir_name;

    //
    // The output looks something like this...
    //
    // S: Remove-entry ./
    // S: /u/cvsroot/supermunger/mungeall.c
    //
    rcp = (response_remove_entry_ty *)rp;
    short_dir_name = os_dirname_relative(rcp->client_side);
    output_fprintf(op, "Remove-entry %s/\n", short_dir_name->str_text);
    output_fprintf(op, ROOT_PATH "/%s\n", rcp->server_side->str_text);
    str_free(short_dir_name);
}


static const response_method_ty vtbl =
{
    sizeof(response_remove_entry_ty),
    destructor,
    write,
    response_code_Remove_entry,
    0, // flushable
};


response_ty *
response_remove_entry_new(string_ty *client_side, string_ty *server_side)
{
    response_ty     *rp;
    response_remove_entry_ty *rcp;

    rp = response_new(&vtbl);
    rcp = (response_remove_entry_ty *)rp;
    rcp->client_side = str_copy(client_side);
    rcp->server_side = str_copy(server_side);
    return rp;
}

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
// MANIFEST: functions to manipulate removeds
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

#include <os.h>
#include <output.h>
#include <response/removed.h>
#include <response/private.h>
#include <server.h>


struct response_removed_ty
{
    response_ty     inherited;
    string_ty       *client_side;
    string_ty       *server_side;
};


static void
destructor(response_ty *rp)
{
    response_removed_ty *rcp;

    rcp = (response_removed_ty *)rp;
    str_free(rcp->client_side);
    rcp->client_side = 0;
    str_free(rcp->server_side);
    rcp->server_side = 0;
}


static void
write(response_ty *rp, output_ty *op)
{
    response_removed_ty *rcp;
    string_ty       *short_dir_name;
    string_ty       *short_file_name;

    //
    // The output looks something like this...
    //
    // S: M file mungeall.c is no longer pertinent
    // S: Removed munger/
    // S: /u/cvsroot/super/munger/mungeall.c
    //
    rcp = (response_removed_ty *)rp;
    short_dir_name = os_dirname_relative(rcp->client_side);
    short_file_name = os_entryname_relative(rcp->client_side);
    output_fprintf
    (
	op,
	"M file %s is no longer pertinent\n"
	"Removed %s/\n"
	ROOT_PATH "/%s\n",
	short_file_name->str_text,
	short_dir_name->str_text,
	rcp->server_side->str_text
    );
    str_free(short_dir_name);
    str_free(short_file_name);
}


static const response_method_ty vtbl =
{
    sizeof(response_removed_ty),
    destructor,
    write,
    response_code_Removed,
    0, // flushable
};


response_ty *
response_removed_new(string_ty *client_side, string_ty *server_side)
{
    response_ty     *rp;
    response_removed_ty *rcp;

    rp = response_new(&vtbl);
    rcp = (response_removed_ty *)rp;
    rcp->client_side = str_copy(client_side);
    rcp->server_side = str_copy(server_side);
    return rp;
}

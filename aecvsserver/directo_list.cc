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
// MANIFEST: functions to manipulate directo_lists
//

#include <directo_list.h>
#include <mem.h>


void
directory_list_constructor(directory_list_ty *dlp)
{
    dlp->length = 0;
    dlp->maximum = 0;
    dlp->item = 0;
}


void
directory_list_destructor(directory_list_ty *dlp)
{
    size_t          j;

    for (j = 0; j < dlp->length; ++j)
	directory_destructor(dlp->item+ j);
    if (dlp->item)
	mem_free(dlp->item);
    dlp->length = 0;
    dlp->maximum = 0;
    dlp->item = 0;
}


void
directory_list_rewind(directory_list_ty *dlp)
{
    size_t          j;

    for (j = 0; j < dlp->length; ++j)
	directory_destructor(dlp->item + j);
    dlp->length = 0;
}


void
directory_list_append(directory_list_ty *dlp, string_ty *client_side,
    string_ty *server_side)
{
    directory_ty    *dp;
    size_t          nbytes;

    if (dlp->length >= dlp->maximum)
    {
	dlp->maximum = dlp->maximum * 2 + 4;
	nbytes = dlp->maximum * sizeof(dlp->item[0]);
	dlp->item = (directory_ty *)mem_change_size(dlp->item, nbytes);
    }
    dp = dlp->item + dlp->length++;
    directory_constructor(dp, client_side, server_side);
}

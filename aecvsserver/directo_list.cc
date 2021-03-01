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

#include <aecvsserver/directo_list.h>


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
    delete [] dlp->item;
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

    if (dlp->length >= dlp->maximum)
    {
	dlp->maximum = dlp->maximum * 2 + 4;
	directory_ty *new_item = new directory_ty [dlp->maximum];
	for (size_t j = 0; j < dlp->length; ++j)
	    new_item[j] = dlp->item[j];
	delete [] dlp->item;
	dlp->item = new_item;
    }
    dp = dlp->item + dlp->length++;
    directory_constructor(dp, client_side, server_side);
}

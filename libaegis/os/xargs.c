/*
 *	aegis - project change supervisor
 *	Copyright (C) 2002 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate xargss
 */

#include <os.h>
#include <str_list.h>


void
os_xargs(string_ty *the_command, string_list_ty *the_list, string_ty *dir)
{
    enum { chunk = 50 };
    size_t          j;

    for (j = 0; j < the_list->nstrings; j += chunk)
    {
	size_t          nargs;
	string_list_ty  wl;
	size_t          k;
	string_ty       *s;

	nargs = chunk;
	if (j + nargs > the_list->nstrings)
	    nargs = the_list->nstrings - j;
	string_list_constructor(&wl);
	string_list_append(&wl, the_command);
	for (k = 0; k < nargs; ++k)
	{
	    s = str_quote_shell(the_list->string[j + k]);
	    string_list_append(&wl, s);
	    str_free(s);
	}
	s = wl2str(&wl, 0, wl.nstrings, (char *)0);
	string_list_destructor(&wl);
	os_become_orig();
	os_execute(s, OS_EXEC_FLAG_INPUT, dir);
	os_become_undo();
	str_free(s);
    }
}

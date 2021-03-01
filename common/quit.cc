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
// MANIFEST: implementation of the quit class
//

#include <ac/stddef.h>
#include <ac/stdio.h>
#include <ac/stdlib.h>

#include <error.h> // for assert
#include <progname.h>
#include <quit.h>

static quit_action *quit_list[10];
static size_t quit_list_len;
static int quitting;


void
quit_register(quit_action &action)
{
    if (quitting)
	return;
    assert(quit_list_len < SIZEOF(quit_list));
    for (size_t j = 0; j < quit_list_len; ++j)
	if (quit_list[j] == &action)
    	    return;
    quit_list[quit_list_len++] = &action;
}


void
quit_unregister(quit_action &action)
{
    if (quitting)
	return;
    assert(quit_list_len < SIZEOF(quit_list));
    for (size_t j = 0; j < quit_list_len; ++j)
    {
	if (quit_list[j] == &action)
	{
	    for (size_t k = j + 1; k < quit_list_len; ++k)
		quit_list[k - 1] = quit_list[k];
	    --quit_list_len;
    	    return;
	}
    }
}


void
quit(int n)
{
    if (quitting > 4)
    {
	fprintf
	(
    	    stderr,
    	    "%s: incorrectly handled error while quitting (bug)\n",
    	    progname_get()
	);
	exit(1);
    }
    ++quitting;
    while (quit_list_len > 0)
    {
	quit_action *qap = quit_list[--quit_list_len];
	(*qap)(n);
    }
    exit(n);
}

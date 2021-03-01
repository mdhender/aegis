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

#include <common/ac/stddef.h>
#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>

#include <common/error.h> // for assert
#include <common/mem.h>
#include <common/progname.h>
#include <common/quit.h>

static quit_action **quit_list;
static size_t quit_list_len;
static size_t quit_list_max;
static int quitting;


void
quit_register(quit_action &action)
{
    if (quitting)
	return;
    for (size_t j = 0; j < quit_list_len; ++j)
	if (quit_list[j] == &action)
    	    return;
    if (quit_list_len >= quit_list_max)
    {
	size_t new_max = quit_list_max * 2 + 16;
	quit_action **new_list = new quit_action * [new_max];
	for (size_t j = 0; j < quit_list_len; ++j)
	    new_list[j] = quit_list[j];
	delete [] quit_list;
	quit_list = new_list;
	quit_list_max = new_max;
    }
    quit_list[quit_list_len++] = &action;
}


void
quit_unregister(quit_action &action)
{
    if (quitting)
	return;
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
    if (quitting++ > 4)
    {
	fprintf
	(
    	    stderr,
    	    "%s: incorrectly handled error while quitting (bug)\n",
    	    progname_get()
	);
	exit(1);
    }
    while (quit_list_len > 0)
    {
	quit_action *qap = quit_list[--quit_list_len];
	(*qap)(n);
    }
    exit(n);
}

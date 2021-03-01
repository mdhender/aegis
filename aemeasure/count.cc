//
//	aegis - project change supervisor
//	Copyright (C) 2002-2004 Peter Miller;
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
// MANIFEST: functions to manipulate counts
//

#include <ac/stdio.h>

#include <count.h>
#include <error.h>
#include <lang.h>


static lang_count_t counting_function;


void
set_language(const char *name)
{
    counting_function = lang_by_name(name)->count;
}


void
count(const char *ifn, const char *ofn)
{
    FILE            *ifp;
    FILE            *ofp;
    static lang_data_t data_zero;
    lang_data_t     data;

    if (!counting_function)
	counting_function = lang_by_file(ifn)->count;

    if (ifn)
    {
	ifp = fopen(ifn, "r");
	if (!ifp)
	    nfatal("open %s", ifn);
    }
    else
    {
	ifn = "standard input";
	ifp = stdin;
    }

    data = data_zero;
    counting_function(ifp, &data);
    if (ferror(ifp))
	nfatal("read %s", ifn);
    fclose(ifp);

    //
    // Write the output.
    //
    if (ofn)
    {
	ofp = fopen(ofn, "w");
	if (!ofp)
	    nfatal("open %s", ofn);
    }
    else
    {
	ofn = "standard output";
	ofp = stdout;
    }
    lang_data_print(ofp, &data);
    if (fflush(ofp))
	nfatal("write %s", ofn);
    if (fclose(ofp))
	nfatal("close %s", ofn);
}

//
//	aegis - project change supervisor
//	Copyright (C) 2003, 2004 Peter Miller;
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
// MANIFEST: functions to manipulate magics
//

#include <ac/magic.h>
#include <ac/string.h>
#include <ac/stdio.h>

#include <main.h>

#ifndef HAVE_MAGIC_FILE


magic_t
magic_open(int flags)
{
    return (magic_t)1;
}


void
magic_close(magic_t cookie)
{
}


const char *
magic_file(magic_t cookie, const char *filename)
{
    typedef struct table_t table_t;
    struct table_t
    {
	const char *suffix;
	const char *type;
    };

    static table_t table[] =
    {
	{ "c",    "text/plain" },
	{ "c++",  "text/plain" },
	{ "cc",   "text/plain" },
	{ "cpp",  "text/plain" },
	{ "gif",  "image/gif"  },
	{ "h",    "text/plain" },
	{ "htm",  "text/html"  },
	{ "html", "text/html"  },
	{ "ico",  "image/x-icon" },
	{ "jpeg", "image/jpeg" },
	{ "jpg",  "image/jpeg" },
	{ "pdf",  "application/pdf" },
	{ "png",  "image/png"  },
	{ "txt",  "text/plain" },
    };

    const char      *base;
    const char      *suffix;
    table_t         *tp;
    FILE            *fp;
    int             j;

    base = strrchr(filename, '/');
    if (!base)
	base = filename;

    suffix = strrchr(base, '.');
    if (suffix)
    {
	++suffix;
	for (tp = table; tp < ENDOF(table); ++tp)
	{
	    if (0 == strcasecmp(suffix, tp->suffix))
		return tp->type;
	}
    }

    fp = fopen(filename, "r");
    if (!fp)
	return "application/x-unknown";
    for (j = 0; j < 10000; ++j)
    {
	int c = getc(fp);
	if (c == EOF)
	    break;
	if (c == 0)
	{
	    fclose(fp);
	    return "application/x-unknown";
	}
    }
    fclose(fp);
    return "text/plain";
}


const char *
magic_error(magic_t cookie)
{
    return "no error";
}

#endif // HAVE_MAGIC_FILE

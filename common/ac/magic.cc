//
// aegis - project change supervisor
// Copyright (C) 2003-2008, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/magic.h>
#include <common/ac/string.h>
#include <common/ac/stdio.h>


#ifndef HAVE_MAGIC_FILE


magic_t
magic_open(int)
{
    return (magic_t)1;
}


void
magic_close(magic_t)
{
}


const char *
magic_file(magic_t, const char *filename)
{
    struct table_t
    {
        const char *suffix;
        const char *type;
    };

    static table_t table[] =
    {
        { "c",    "text/plain; charset=us-ascii" },
        { "c++",  "text/plain; charset=us-ascii" },
        { "cc",   "text/plain; charset=us-ascii" },
        { "cpp",  "text/plain; charset=us-ascii" },
        { "gif",  "image/gif"  },
        { "h",    "text/plain; charset=us-ascii" },
        { "htm",  "text/html"  },
        { "html", "text/html"  },
        { "ico",  "image/x-icon" },
        { "jpeg", "image/jpeg" },
        { "jpg",  "image/jpeg" },
        { "pdf",  "application/pdf" },
        { "png",  "image/png"  },
        { "txt",  "text/plain; charset=us-ascii" },
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
    return "text/plain; charset=us-ascii";
}


const char *
magic_error(magic_t)
{
    return "no error";
}

int
magic_load(magic_t, const char *)
{
    return 0;
}

#endif // HAVE_MAGIC_FILE


// vim: set ts=8 sw=4 et :

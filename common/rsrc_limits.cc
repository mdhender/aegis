//
//	aegis - project change supervisor
//	Copyright (C) 2005 Peter Miller;
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
// MANIFEST: implementation of the rsrc_limits class
//

#include <ac/stdio.h>
#include <ac/unistd.h>
#include <sys/resource.h>

#include <rsrc_limits.h>
#include <config.h>


static void
adjust_resource(int resource)
{
    rlimit r;
    if (getrlimit(resource, &r) >= 0 && r.rlim_cur != r.rlim_max)
    {
	r.rlim_cur = r.rlim_max;
	setrlimit(resource, &r);
    }
}


void
resource_limits_init()
{
    adjust_resource(RLIMIT_AS);
    adjust_resource(RLIMIT_DATA);
}


static void
print_size(long size, const char *caption, int pagesize)
{
    if (pagesize <= 1 || size == 0)
    {
	fprintf(stderr, "%8ld   %s\n", size, caption);
	return;
    }

    while (pagesize < 1024)
    {
	size = (size + 1) >> 1;
	pagesize <<= 1;
    }
    pagesize >>= 10;

    size *= pagesize;
    if (size < 10000)
    {
	fprintf(stderr, "%4ldK  %s\n", size, caption);
	return;
    }
    size = (size + 512) >> 10;
    if (size < 10000)
    {
	fprintf(stderr, "%4ldM  %s\n", size, caption);
	return;
    }
    size = (size + 512) >> 10;
    fprintf(stderr, "%4ldG  %s\n", size, caption);
}


void
resource_usage_print()
{
#ifdef __linux__
    const char *fn = "/proc/self/statm";
    FILE *fp = fopen(fn, "r");
    if (!fp)
	return;
    long size, resident, share, trs, drs, lrs;
    fscanf(fp, "%ld%ld%ld%ld%ld%ld",
	&size, &resident, &share, &trs, &drs, &lrs);
    fclose(fp);

    long pagesize = getpagesize();

    print_size(size, "total program size", pagesize);
    print_size(resident, "resident set size", pagesize);
    print_size(share, "shared pages", pagesize);
    print_size(trs, "text (code)", pagesize);
    print_size(drs, "data/stack", pagesize);
    print_size(lrs, "library", pagesize);
#endif // __linux__
}

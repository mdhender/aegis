//
//	aegis - project change supervisor
//	Copyright (C) 2005-2008 Peter Miller
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

#include <common/ac/signal.h>
#include <common/ac/stdio.h>
#include <common/ac/unistd.h>
#include <common/ac/sys/resource.h>

#include <common/rsrc_limits.h>


//
// This is a disgusting hack to get around a disgusting hack in glibc's
// implementation of sys/resource.h
//
// Glibc introduces a typedef for the first argument of getrlimit, and in
// some versions, fails to insulate this against C++ which has stricter
// enum to int rules than C does.  This results in a compile time error.
//
#ifndef __USE_GNU
typedef int rlimit_resource_ty;
#else
typedef __rlimit_resource_t rlimit_resource_ty;
#endif


static void
adjust_resource(rlimit_resource_ty resource)
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
    //
    // From getrlimit(2) manual page:  "A child process created via
    // fork(2) inherits its parents resource limits.  Resource
    // limits are preserved across execve(2)."
    //
    adjust_resource(RLIMIT_AS);
    adjust_resource(RLIMIT_DATA);
    adjust_resource(RLIMIT_FSIZE);

    //
    // Some operating systems generate the SIGXFSZ signal when a
    // file exceeds the getrlimit(RLIMIT_FSIZE) size, in addition to
    // returning the EFBIG errno value.  By ignoring this signal, the
    // error gets returned and it is possible to report the offending
    // file's name, making for a more useful error message.
    //
    // Linux ignores this signal by default, but allows it to be
    // set.  Other posix implementations may not ignore this signal by
    // default.
    //
#ifdef SIGXFSZ
    signal(SIGXFSZ, SIG_IGN);
#endif
}


#ifdef __linux__


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
	fprintf(stderr, "%4ldk  %s\n", size, caption);
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

#endif // __linux__


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

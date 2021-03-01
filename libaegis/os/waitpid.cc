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
// MANIFEST: functions to manipulate waitpids
//

#include <ac/errno.h>
#include <ac/stddef.h>

#include <sys/types.h>
#include <sys/wait.h>

#include <error.h> // for assert
#include <mem.h>
#include <os.h>
#include <trace.h>


int
os_waitpid(int child, int *status_p)
{
    struct ret_ty
    {
	int             pid;
	int             status;
    };

    static long     nret;
    static long     nret_max;
    static ret_ty   *ret;
    int             pid;
    int             status;
    int             j;
    int             result;

    //
    // see if we already have it
    //
    trace(("os_waitpid(child = %d)\n{\n", child));
    assert(child > 0);
    result = 0;
    for (j = 0; j < nret; ++j)
    {
	if (ret[j].pid != child)
	    continue;
	*status_p = ret[j].status;
	ret[j] = ret[--nret];
	goto done;
    }

    //
    // new one, go hunting
    //
    for (;;)
    {
	//
	// block until a child terminates,
	// or there are no more children
	//
	pid = wait(&status);
	if (pid == -1)
	{
	    if (errno == EINTR)
		continue;
	    result = -1;
	    break;
	}

	//
	// stop if this is the child
	// we are looking for
	//
	if (pid == child)
	{
	    *status_p = status;
	    break;
	}

	//
	// remember and keep going
	//
	if (nret >= nret_max)
	{
	    long            nbytes;

	    nret_max += 11;
	    nbytes = nret_max * sizeof(ret_ty);
	    if (!ret)
		ret = (ret_ty *)mem_alloc(nbytes);
	    else
		ret = (ret_ty *)mem_change_size(ret, nbytes);
	}
	ret[nret].pid = pid;
	ret[nret].status = status;
	++nret;
    }

    //
    // here for all exits
    //
    done:
    trace(("return %d;\n", result));
    trace(("}\n"));
    return result;
}

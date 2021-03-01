//
//	aegis - project change supervisor
//	Copyright (C) 2002-2006, 2008 Peter Miller
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

#include <common/ac/errno.h>
#include <common/ac/stddef.h>
#include <common/ac/sys/types.h>
#include <sys/wait.h>

#include <common/error.h> // for assert
#include <common/trace.h>
#include <libaegis/os.h>


int
os_waitpid(int child, int *status_p)
{
    struct ret_ty
    {
	int             pid;
	int             status;
    };

    static size_t   nret;
    static size_t   nret_max;
    static ret_ty   *ret;
    int             pid;
    int             status;
    int             result;

    //
    // see if we already have it
    //
    trace(("os_waitpid(child = %d)\n{\n", child));
    assert(child > 0);
    result = 0;
    for (size_t j = 0; j < nret; ++j)
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
	    nret_max = nret_max * 2 + 8;
	    ret_ty *new_ret  = new ret_ty [nret_max];
	    for (size_t k = 0; k < nret; ++k)
		new_ret[k] = ret[k];
	    delete [] ret;
	    ret = new_ret;
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

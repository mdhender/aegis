//
//	aegis - project change supervisor
//	Copyright (C) 1994, 2003-2008 Peter Miller.
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

#include <common/error.h>
#include <common/mem.h>
#include <common/mem.h>
#include <libaegis/aer/stmt.h>


rpt_stmt::rpt_stmt() :
    child(0),
    nchild(0),
    nchild_max(0)
{
}


rpt_stmt::~rpt_stmt()
{
    delete [] child;
    child = 0;
    nchild = 0;
    nchild_max = 0;
}


void
rpt_stmt::append(const pointer &sp)
{
    if (nchild >= nchild_max)
    {
	size_t new_nchild_max = nchild_max * 2 + 4;
	pointer *new_child = new pointer [new_nchild_max];
	for (size_t j = 0; j < nchild; ++j)
	    new_child[j] = child[j];
	delete [] child;
	child = new_child;
	nchild_max = new_nchild_max;
    }
    child[nchild++] = sp;
}


void
rpt_stmt::prepend(const pointer &sp)
{
    if (nchild >= nchild_max)
    {
	size_t new_nchild_max = nchild_max * 2 + 4;
	pointer *new_child = new pointer [new_nchild_max];
	for (size_t j = 0; j < nchild; ++j)
	    new_child[j + 1] = child[j];
	delete [] child;
	child = new_child;
	nchild_max = new_nchild_max;
    }
    else
    {
	for (size_t j = nchild; j > 0; --j)
	    child[j] = child[j - 1];
    }
    nchild++;
    child[0] = sp;
}


rpt_stmt::pointer
rpt_stmt::nth_child(size_t n)
    const
{
    if (n >= nchild)
        return pointer();
    return child[n];
}

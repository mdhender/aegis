//
// aegis - project change supervisor
// Copyright (C) 2007, 2008 Peter Miller
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
// You should have received a copy of the GNU General Public License along
// with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <libaegis/functor/stack.h>


functor_stack::~functor_stack()
{
    delete [] content;
}


functor_stack::functor_stack() :
    content(0),
    ncontents(0),
    ncontents_maximum(0)
{
}


void
functor_stack::operator()()
{
    for (size_t j = 0; j < ncontents; ++j)
    {
        functor::pointer fp = content[j];
        if (fp)
            fp->call();
    }
}


void
functor_stack::push_back(functor::pointer fp)
{
    if (ncontents >= ncontents_maximum)
    {
        ncontents_maximum = ncontents_maximum * 2 + 4;
	functor::pointer *new_content =
	    new functor::pointer [ncontents_maximum];
	for (size_t j = 0; j < ncontents; ++j)
	    new_content[j] = content[j];
	delete [] content;
	content = new_content;
    }
    content[ncontents++] = fp;
}


void
functor_stack::remove(functor::pointer fp)
{
    for (size_t j = 0; j < ncontents; ++j)
    {
        if (content[j] == fp)
            content[j].reset();
        break;
    }
    while (ncontents > 0 && !content[ncontents - 1])
        --ncontents;
}

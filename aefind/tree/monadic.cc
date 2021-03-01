//
//	aegis - project change supervisor
//	Copyright (C) 1997, 2002-2008 Peter Miller
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

#include <common/ac/stdio.h>

#include <aefind/tree/monadic.h>


tree_monadic::~tree_monadic()
{
}


tree_monadic::tree_monadic(const pointer &a_arg) :
    arg(a_arg)
{
}


void
tree_monadic::print()
    const
{
    printf("%s ( ", name());
    arg->print();
    printf(" )");
}


bool
tree_monadic::useful()
    const
{
    return arg->useful();
}


bool
tree_monadic::constant()
    const
{
    return arg->constant();
}

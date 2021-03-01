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

#include <aefind/tree/diadic.h>


tree_diadic::~tree_diadic()
{
}


tree_diadic::tree_diadic(const tree::pointer &lhs, const tree::pointer &rhs) :
    left(lhs),
    right(rhs)
{
}


void
tree_diadic::print()
    const
{
    printf("( ");
    left->print();
    printf(" %s ", name());
    right->print();
    printf(" )");
}


bool
tree_diadic::useful()
    const
{
    return (left->useful() || right->useful());
}


bool
tree_diadic::constant()
    const
{
    return (left->constant() && right->constant());
}

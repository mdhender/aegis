//
//	aegis - project change supervisor
//	Copyright (C) 2006, 2008 Peter Miller
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

#include <common/stack.h>


static int
find_stack_direction ()
{
    static char *addr = 0;
    auto char dummy;
    if (addr == 0)
    {
	addr = &dummy;
	return find_stack_direction ();
    }
    else
    {
	return ((&dummy > addr) ? 1 : -1);
    }
}


static int stack_direction;


bool
variable_is_on_stack(void *p)
{
    if (!p)
	return false;
    if (stack_direction == 0)
	stack_direction = find_stack_direction();
    auto char dummy = 0;
    char *cp = (char *)p;
    return (stack_direction < 0 ?  (&dummy < cp) : (&dummy > cp));
}

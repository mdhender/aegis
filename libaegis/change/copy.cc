//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002-2008 Peter Miller
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

#include <libaegis/change.h>
#include <common/error.h> // for assert
#include <common/trace.h>


change::pointer
change_copy(change::pointer cp)
{
    trace(("change_copy(cp = %08lX)\n{\n", (long)cp));
    assert(cp->reference_count >= 1);
    cp->reference_count++;
    trace(("return %08lX;\n", (long)cp));
    trace(("}\n"));
    return cp;
}

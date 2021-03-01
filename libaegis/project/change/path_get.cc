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

#include <common/trace.h>
#include <libaegis/change.h>
#include <libaegis/project.h>
#include <libaegis/zero.h>


string_ty *
project_ty::change_path_get(long n)
{
    trace(("project_ty::change_path_get(this = %08lX, n = %ld)\n{\n",
	(long)this, n));
    n = magic_zero_decode(n);
    string_ty *s = 0;
    if (n == TRUNK_CHANGE_NUMBER)
	s = str_format("%s/trunk", info_path_get()->str_text);
    else
    {
	s =
	    str_format
	    (
		"%s/%ld/%3.3ld",
		changes_path_get()->str_text,
		n / 100,
		n
	    );
    }
    trace(("return \"%s\";\n", s->str_text));
    trace(("}\n"));
    return s;
}

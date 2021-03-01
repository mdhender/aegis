//
//	aegis - project change supervisor
//	Copyright (C) 2004-2006, 2008 Peter Miller
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

#include <libaegis/project.h>
#include <common/trace.h>


string_ty *
project_ty::history_path_get()
{
    trace(("project_ty::history_path_get(this = %08lX)\n{\n", (long)this));
    if (parent)
	return trunk_get()->history_path_get();
    if (!history_path)
	history_path = str_format("%s/history", home_path_get()->str_text);
    trace(("return \"%s\";\n", history_path->str_text));
    trace(("}\n"));
    return history_path;
}

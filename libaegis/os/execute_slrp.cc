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

#include <libaegis/file.h>
#include <libaegis/os.h>
#include <common/trace.h>


string_ty *
os_execute_slurp(string_ty *cmd, int flags, string_ty *dir)
{
    trace(("os_execute_slurp()\n{\n"));
    nstring s1(os_edit_filename(0));
    trace_nstring(s1);
    nstring s2 = nstring::format("( %s ) > %s", cmd->str_text, s1.c_str());
    os_execute(s2.get_ref(), flags, dir);
    s2 = read_whole_file(s1);
    os_unlink(s1);
    trace_nstring(s2);
    trace(("}\n"));
    return str_copy(s2.get_ref());
}

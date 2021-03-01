/*
 *	aegis - project change supervisor
 *	Copyright (C) 2002, 2003 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate execute_slrps
 */

#include <file.h>
#include <os.h>
#include <trace.h>


string_ty *
os_execute_slurp(string_ty *cmd, int flags, string_ty *dir)
{
    string_ty       *s1;
    string_ty       *s2;

    trace(("os_execute_slurp()\n{\n"));
    s1 = os_edit_filename(0);
    trace_string(s1->str_text);
    s2 = str_format("( %S ) > %S", cmd, s1);
    os_execute(s2, flags, dir);
    str_free(s2);
    s2 = read_whole_file(s1);
    os_unlink(s1);
    str_free(s1);
    trace_string(s2->str_text);
    trace(("}\n"));
    return s2;
}

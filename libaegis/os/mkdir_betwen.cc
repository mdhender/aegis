//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002-2005 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to manipulate mkdir_betwens
//

#include <common/error.h> // for assert
#include <libaegis/os.h>
#include <common/trace.h>
#include <libaegis/undo.h>


void
os_mkdir_between(const nstring &top, const nstring &extn, int mode)
{
    trace(("os_mkdir_between(top = %08lX, extn = %08lX, mode = 0%o)\n{\n",
	(long)&top, (long)&extn, mode));
    trace_string(top.c_str());
    trace_string(extn.c_str());
    assert(top[0] == '/');
    assert(extn[0] != '/');
    assert(extn[extn.size() - 1] != '/');
    assert(!extn.empty());
    for (const char *cp = extn.c_str(); *cp; ++cp)
    {
	if (*cp != '/')
	    continue;
	nstring s1(extn.c_str(), cp - extn.c_str());
	nstring s2(os_path_cat(top, s1));
	trace(("mkdir %s\n", s2.c_str()));
	if (!os_exists(s2))
	{
	    os_mkdir(s2, mode);
	    undo_rmdir_errok(s2);
	}
    }
    trace(("}\n"));
}


void
os_mkdir_between(string_ty *top, string_ty *extn, int mode)
{
    os_mkdir_between(nstring(str_copy(top)), nstring(str_copy(extn)), mode);
}

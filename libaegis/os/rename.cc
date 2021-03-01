//
//	aegis - project change supervisor
//	Copyright (C) 1991-2004 Peter Miller;
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
// MANIFEST: wrappers around operating system functions
//

#include <ac/errno.h>

#include <glue.h>
#include <os.h>
#include <sub.h>
#include <trace.h>


void
os_rename(const nstring &a, const nstring &b)
{
    trace(("os_rename(a = \"%s\", b = \"%s\")\n{\n", a.c_str(), b.c_str()));
    os_become_must_be_active();
    if (glue_rename(a.c_str(), b.c_str()))
    {
	int errno_old = errno;
	sub_context_ty *scp = sub_context_new();
	sub_errno_setx(scp, errno_old);
	sub_var_set_string(scp, "File_Name1", a);
	sub_var_set_string(scp, "File_Name2", b);
	fatal_intl(scp, i18n("rename(\"$filename1\", \"$filename2\"): $errno"));
	// NOTREACHED
    }
    trace(("}\n"));
}


void
os_rename(string_ty *a, string_ty *b)
{
    os_rename(nstring(str_copy(a)), nstring(str_copy(b)));
}

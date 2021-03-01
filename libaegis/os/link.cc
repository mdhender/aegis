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
#include <ac/unistd.h>

#include <glue.h>
#include <os.h>
#include <sub.h>
#include <trace.h>


void
os_link(string_ty *from, string_ty *to)
{
    trace(("os_link(from = %08lX, to = %08lX)\n{\n", (long)from, (long)to));
    os_become_must_be_active();
    trace_string(from->str_text);
    trace_string(to->str_text);
    if (glue_link(from->str_text, to->str_text))
    {
	sub_context_ty  *scp;
	int             errno_old;

	errno_old = errno;
	scp = sub_context_new();
	sub_errno_setx(scp, errno_old);
	sub_var_set_string(scp, "File_Name1", from);
	sub_var_set_string(scp, "File_Name2", to);
	fatal_intl(scp, i18n("link(\"$filename1\", \"$filename2\"): $errno"));
	// NOTREACHED
    }
    trace(("}\n"));
}

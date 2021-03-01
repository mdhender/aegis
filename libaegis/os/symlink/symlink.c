/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-2002 Peter Miller;
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
 * MANIFEST: wrappers around operating system functions
 */

#include <ac/unistd.h>
#include <ac/stddef.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <error.h>
#include <glue.h>
#include <os.h>
#include <sub.h>
#include <trace.h>


void
os_symlink(src, dst)
    string_ty       *src;
    string_ty       *dst;
{
    trace(("os_symlink()\n{\n"));
    os_become_must_be_active();
#ifdef S_IFLNK
    if (glue_symlink(src->str_text, dst->str_text))
    {
	sub_context_ty *scp;

	scp = sub_context_new();
	sub_errno_set(scp);
	sub_var_set_string(scp, "File_Name1", src);
	sub_var_set_string(scp, "File_Name2", dst);
	fatal_intl
	(
	    scp,
	    i18n("symlink(\"$filename1\", \"$filename2\"): $errno")
	);
	sub_context_delete(scp);
    }
#else
    fatal_raw
    (
	"symlink(\"%s\", \"%s\"): symbolic links not available",
	src->str_text,
	dst->str_text
    );
#endif
    trace(("}\n"));
}

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

#include <ac/errno.h>
#include <ac/unistd.h>

#include <glue.h>
#include <os.h>
#include <sub.h>
#include <trace.h>


void
os_rmdir_errok(path)
    string_ty       *path;
{
    trace(("os_rmdir_errok(path = \"%s\")\n{\n", path->str_text));
    os_become_must_be_active();
    if (glue_rmdir(path->str_text) && errno != ENOENT)
    {
	sub_context_ty  *scp;

	scp = sub_context_new();
	sub_errno_set(scp);
	sub_var_set_string(scp, "File_Name", path);
	error_intl(scp, i18n("warning: rmdir $filename: $errno"));
	sub_context_delete(scp);
    }
    trace(("}\n"));
}

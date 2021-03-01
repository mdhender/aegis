/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-2003 Peter Miller;
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

#include <glue.h>
#include <os.h>
#include <sub.h>


string_ty *
os_fingerprint(string_ty *path)
{
    int             nbytes;
    char            buf[1000];

    os_become_must_be_active();
    nbytes = file_fingerprint(path->str_text, buf, sizeof(buf));
    if (nbytes < 0)
    {
	sub_context_ty  *scp;

	scp = sub_context_new();
	sub_errno_set(scp);
	sub_var_set_string(scp, "File_Name", path);
	fatal_intl(scp, i18n("fingerprint $filename: $errno"));
	/* NOTREACHED */
    }
    return str_n_from_c(buf, nbytes);
}

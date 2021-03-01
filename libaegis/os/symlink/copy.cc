//
//	aegis - project change supervisor
//	Copyright (C) 2004 Peter Miller;
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

#include <ac/unistd.h>

#include <file.h>
#include <glue.h>
#include <os.h>
#include <trace.h>


void
os_symlink_or_copy(string_ty *src, string_ty *dst)
{
    trace(("os_symlink_or_copy(src = \"%s\", dst = \"%s\")\n{\n", src->str_text,
	dst->str_text));
    os_become_must_be_active();
#ifdef HAVE_SYMLINK
    if (glue_symlink(src->str_text, dst->str_text) < 0)
#endif
	copy_whole_file(src, dst, 1);
    trace(("}\n"));
}

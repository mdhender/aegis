//
//	aegis - project change supervisor
//	Copyright (C) 2005 Peter Miller;
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
// MANIFEST: implementation of the os_magic_file class
//

#include <common/ac/magic.h>
#include <common/ac/stdlib.h>

#include <common/env.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>
#include <common/nstring/list.h>


#ifdef SOURCE_FORGE_HACK

static void
fix_magic_path()
{
    static bool fixed;
    if (fixed)
	return;
    fixed = true;

    const char *cp = getenv("MAGIC");
    nstring_list files;
    files.split(nstring(cp), ":");
    files.push_back_unique("/etc/magic");
    files.push_back_unique("/usr/share/file/magic");
    files.push_back_unique("/usr/share/misc/magic");
    files.push_back_unique("/usr/share/misc/file/magic");
    env_set("MAGIC", files.unsplit(":").c_str());
}

#endif


nstring
os_magic_file(const nstring &filename)
{
#ifdef SOURCE_FORGE_HACK
    fix_magic_path();
#endif

    os_become_must_be_active();
    magic_t cookie = magic_open(MAGIC_MIME | MAGIC_SYMLINK);
    if (!cookie)
    {
	fail:
	sub_context_ty sc;
	sc.var_set_string("File_Name", filename);
	sc.var_set_charstar("ERRNO", magic_error(cookie));
	sc.var_override("ERRNO");
	sc.fatal_intl(i18n("read $filename: $errno"));
	// NOTREACHED
    }
    if (magic_load(cookie, 0))
	goto fail;
    const char *content_type = magic_file(cookie, filename.c_str());
    if (!content_type)
	goto fail;
    if (!*content_type)
    {
	// Make sure we never return the empty string.
	content_type = "text/plain; charset=us-ascii";
    }
    nstring result(content_type);
    magic_close(cookie);
    return result;
}


nstring
os_magic_file(string_ty *filename)
{
    // just for compatibility
    return os_magic_file(nstring(str_copy(filename)));
}

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

#include <common/ac/stdlib.h>

#include <common/nstring.h>
#include <libaegis/os.h>
#include <common/progname.h>
#include <common/uuidentifier.h>


static nstring
find_temp_dir(int at_home)
{
    if (at_home)
    {
	const char *cp = getenv("HOME");
	if (cp && *cp)
	    return cp;
    }
    return nstring(os_tmpdir());
}


string_ty *
os_edit_filename(int at_home)
{
    nstring dir = find_temp_dir(at_home);
    int name_max = 14;
    if (os_become_active())
    {
	name_max = os_pathconf_name_max(dir.get_ref());
    }
    else
    {
	os_become_orig();
	name_max = os_pathconf_name_max(dir.get_ref());
	os_become_undo();
    }
    nstring prog = nstring::format("%.*s", name_max / 2, progname_get());
    nstring glunk(universal_unique_identifier());

    //
    // Build the name of the temporary file.  If one of the parts
    // has to be made smaller, because otherwise the file name would
    // be too long, throw away some of the UUID.  On modern Unix
    // implementations this will not happen.
    //
    return
	str_format
	(
	    "%s/%s-%.*s",
	    dir.c_str(),
	    prog.c_str(),
	    (int)(name_max - prog.size() - 1),
	    glunk.c_str()
	);
}

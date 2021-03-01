//
//	aegis - project change supervisor
//	Copyright (C) 2002-2008 Peter Miller
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

#include <common/ac/errno.h>
#include <common/ac/fcntl.h>
#include <common/ac/unistd.h>

#include <libaegis/os.h>
#include <libaegis/glue.h>


int
os_readable(const nstring &path)
{
    os_become_must_be_active();
    int fd = glue_open(path.c_str(), 0, 0666);
    if (fd < 0)
	return errno;
    glue_close(fd);
    return 0;
}


int
os_readable(string_ty *path)
{
    return os_readable(nstring(path));
}

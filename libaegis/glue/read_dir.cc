//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2003-2006, 2008 Peter Miller
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

#include <common/ac/dirent.h>
#include <common/ac/errno.h>
#include <common/ac/string.h>

#include <common/mem.h>
#include <libaegis/glue.h>
#include <libaegis/os.h>


int
read_whole_dir(const char *path, char **data_p, long *data_len_p)
{
    DIR             *dp;
    struct dirent   *de;
    static char     *data;
    static size_t   data_len;
    static size_t   data_max;
    char            *np;
    size_t          len;

    os_interrupt_cope();
    errno = ENOMEM;
    dp = opendir(path);
    if (!dp)
	return -1;
    errno = 0;

    data_len = 0;
    for (;;)
    {
	de = readdir(dp);
	if (!de)
	    break;
	np = de->d_name;
	if (np[0] == '.' && (!np[1] || (np[1] == '.' && !np[2])))
	    continue;
	len = strlen(np) + 1;
	if (data_len + len > data_max)
	{
	    for (;;)
	    {
		data_max = data_max * 2 + 32;
		if (data_len + len <= data_max)
		    break;
	    }
	    char *new_data = new char [data_max];
	    memcpy(new_data, data, data_len);
	    delete [] data;
	    data = new_data;
	}
	memcpy(data + data_len, np, len);
	data_len += len;
    }
    closedir(dp);
    *data_p = data;
    *data_len_p = data_len;
    return 0;
}

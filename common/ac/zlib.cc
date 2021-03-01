//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2004-2006, 2008 Peter Miller
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

#include <errno.h>
#include <common/ac/string.h>
#include <common/ac/zlib.h>


const char *
z_error(int err)
{
    switch (err)
    {
    case Z_OK:
	return "no error";

    case Z_STREAM_END:
	return "stream end";

    case Z_NEED_DICT:
	return "need dict";

    case Z_ERRNO:
	return strerror(errno);

    case Z_STREAM_ERROR:
	return "stream error";

    case Z_DATA_ERROR:
	return "data error";

    case Z_MEM_ERROR:
	return "memory error";

    case Z_BUF_ERROR:
	return "buffer error";

    case Z_VERSION_ERROR:
	return "version error";
    }
    return "unknown zlib error";
}

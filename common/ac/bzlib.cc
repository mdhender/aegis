//
//	aegis - project change supervisor
//	Copyright (C) 2006, 2008 Peter Miller
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

#include <common/ac/bzlib.h>


#ifndef HAVE_BZ2_STRERROR

const char *
BZ2_strerror(int n)
{
    switch (n)
    {
    case BZ_OK:
	return "OK";

    case BZ_RUN_OK:
	return "RUN_OK";

    case BZ_FLUSH_OK:
	return "FLUSH_OK";

    case BZ_FINISH_OK:
	return "FINISH_OK";

    case BZ_STREAM_END:
	return "STREAM_END";

    case BZ_SEQUENCE_ERROR:
	return "Sequence error";

    case BZ_PARAM_ERROR:
	return "Parameter error";

    case BZ_MEM_ERROR:
	return "Memory allocation error";

    case BZ_DATA_ERROR:
	return "Data error";

    case BZ_DATA_ERROR_MAGIC:
	return "Wrong magic number";

    case BZ_IO_ERROR:
        // Maybe we should call strerror(errno) in this case?
        // However, not all instances of BZ2_IO_ERROR being used have a
        // 1:1 correlation with errno being set.
	return "I/O error";

    case BZ_UNEXPECTED_EOF:
	return "Unexpected end-of-file";

    case BZ_OUTBUFF_FULL:
	return "Output buffer full";

    case BZ_CONFIG_ERROR:
	return "Configuration error";
    }
    return "Unknown error";
}

#endif

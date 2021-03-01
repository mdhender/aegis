//
//	aegis - project change supervisor
//	Copyright (C) 1998, 1999, 2003-2006, 2008 Peter Miller
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
#include <common/ac/unistd.h>

#include <libaegis/fopen_nfs.h>
#include <libaegis/glue.h>


FILE *
fopen_with_stale_nfs_retry(const char *path, const char *mode)
{
	FILE		*fp;
#ifdef ESTALE
	int		ntries;
	const int	nsecs = 5;
#endif

	//
	// Try to open the file.
	//
	errno = 0;
	fp = glue_fopen(path, mode);

	//
	// Keep trying for one minute if we get a Stale NFS file handle
	// error.  Some systems suffer from this in a Very Bad Way.
	//
#ifdef ESTALE
	for (ntries = 0; ntries < 60; ntries += nsecs)
	{
		if (fp)
			break;
		if (errno != ESTALE)
			break;
		sleep(nsecs);
		errno = 0;
		fp = glue_fopen(path, mode);
	}
#endif

	//
	// Return the result, both success and failure.
	// Errors are handled elsewhere.
	//
	return fp;
}

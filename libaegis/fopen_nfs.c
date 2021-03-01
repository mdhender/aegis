/*
 *	aegis - project change supervisor
 *	Copyright (C) 1998, 1999, 2003 Peter Miller;
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
 * MANIFEST: functions to insulate fopen from stale NFS file handles
 */

#include <ac/errno.h>
#include <ac/unistd.h>

#include <fopen_nfs.h>
#include <glue.h>


FILE *
fopen_with_stale_nfs_retry(const char *path, const char *mode)
{
	FILE		*fp;
#ifdef ESTALE
	int		ntries;
	const int	nsecs = 5;
#endif

	/*
	 * Try to open the file.
	 */
	errno = 0;
	fp = glue_fopen(path, mode);

	/*
	 * Keep trying for one minute if we get a Stale NFS file handle
	 * error.  Some systems suffer from this in a Very Bad Way.
	 */
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

	/*
	 * Return the result, both success and failure.
	 * Errors are handled elsewhere.
	 */
	return fp;
}

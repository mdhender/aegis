//
//	aegis - project change supervisor
//	Copyright (C) 2001-2006, 2008 Peter Miller
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
#include <common/ac/stdlib.h>
#include <common/ac/unistd.h>

#include <libaegis/glue.h>
#include <libaegis/os.h>
#include <common/trace.h>


//
// NAME
//	copyfile - copy a file
//
// SYNOPSIS
//	int copyfile(char *src, char *dst);
//
// DESCRIPTION
//	The copyfile function complements the link and rename functions.
//
// ARGUMENTS
//	src	- pathname of source file
//	dst	- pathname of destination file
//
// RETURNS
//	0	on success
//	-1	on error, setting errno appropriately
//

int
copyfile(const char *src, const char *dst)
{
	int	src_fd;
	int	dst_fd;
	char	*buffer;
	long	max;
	long	nbytes;
	long	nbytes2;
	int	err;
	int	result;

	trace(("copyfile(\"%s\", \"%s\")\n{\n", src, dst));
	os_interrupt_cope();
	result = -1;
	src_fd = open(src, O_RDONLY, 0666);
	if (src_fd < 0)
		goto done;
	dst_fd = open(dst, O_WRONLY | O_CREAT | O_TRUNC, 0666);
	if (dst_fd < 0)
	{
		err = errno;
		close(src_fd);
		errno = err;
		goto done;
	}

	max = 1L << 13;
	errno = 0;
	buffer = (char *)malloc(max);
	if (!buffer)
	{
		err = errno ? errno : ENOMEM;
		close(dst_fd);
		close(src_fd);
		errno = err;
		goto done;
	}

	for (;;)
	{
		nbytes = read(src_fd, buffer, max);
		if (nbytes < 0)
		{
			err = errno;
			close(src_fd);
			close(dst_fd);
			free(buffer);
			errno = err;
			goto done;
		}
		if (nbytes == 0)
			break;

		nbytes2 = write(dst_fd, buffer, nbytes);
		if (nbytes2 < 0)
		{
			err = errno;
			close(src_fd);
			close(dst_fd);
			free(buffer);
			errno = err;
			goto done;
		}
		if (nbytes2 != nbytes)
		{
			close(src_fd);
			close(dst_fd);
			free(buffer);
			errno = EIO; // weird device, probably
			goto done;
		}
	}
	free(buffer);
	if (close(src_fd))
	{
		err = errno;
		close(dst_fd);
		errno = err;
		goto done;
	}
	result = close(dst_fd);

	//
	// here for all exits
	//
	done:
	trace(("return %d; /* errno = %d */\n", result, errno));
	trace(("}\n"));
	return result;
}

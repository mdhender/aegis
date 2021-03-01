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
//	catfile - copy a file
//
// SYNOPSIS
//	int catfile(char *path);
//
// DESCRIPTION
//	The catfile function is used to print the contents of
//	a file on the standard output.
//
// ARGUMENTS
//	path	- pathname of source file
//
// RETURNS
//	0	on success
//	-1	on error, setting errno appropriately
//

int
catfile(const char *path)
{
	int	fd;
	char	*buffer;
	long	max;
	long	nbytes;
	long	nbytes2;
	int	err;
	int	result;

	trace(("catfile(\"%s\")\n{\n", path));
	os_interrupt_cope();
	result = -1;
	fd = open(path, O_RDONLY, 0666);
	if (fd < 0)
		goto done;

	max = 1L << 13;
	errno = 0;
	buffer = (char *)malloc(max);
	if (!buffer)
	{
		err = errno ? errno : ENOMEM;
		close(fd);
		errno = err;
		goto done;
	}

	for (;;)
	{
		nbytes = read(fd, buffer, max);
		if (nbytes < 0)
		{
			err = errno;
			close(fd);
			free(buffer);
			errno = err;
			goto done;
		}
		if (nbytes == 0)
			break;

		nbytes2 = write(fileno(stdout), buffer, nbytes);
		if (nbytes2 < 0)
		{
			err = errno;
			close(fd);
			free(buffer);
			errno = err;
			goto done;
		}
		if (nbytes2 != nbytes)
		{
			close(fd);
			free(buffer);
			errno = EIO; // weird device, probably
			goto done;
		}
	}
	free(buffer);
	result = close(fd);

	//
	// here for all exits
	//
	done:
	trace(("return %d; /* errno = %d */\n", result, errno));
	trace(("}\n"));
	return result;
}

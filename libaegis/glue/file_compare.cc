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

#include <common/ac/errno.h>
#include <common/ac/fcntl.h>
#include <common/ac/stddef.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>
#include <common/ac/unistd.h>
#include <common/ac/sys/types.h>
#include <common/ac/sys/stat.h>

#include <libaegis/glue.h>
#include <libaegis/os.h>


//
// NAME
//	file_compare
//
// SYNOPSIS
//	int file_compare(char *, char *);
//
// DESCRIPTION
//	The file_compare program reads two files and chanks to see if
//	they are different.
//
// RETURNS
//	int;	0 if the files are the same
//		1 if the files are different
//		-1 on any error
//

int
file_compare(const char *fn1, const char *fn2)
{
	int		fd1;
	int		fd2;
	char		*buf1;
	char		*buf2;
	size_t		len;
	int		err;
	int		result;
	long		n1;
	long		n2;
	struct stat	st1;
	struct stat	st2;

	//
	// make sure they are the same size
	//
	// This will take care of most differences
	//
	os_interrupt_cope();
	if (stat(fn1, &st1))
		return -1;
	if (stat(fn2, &st2))
		return -1;
	if (st1.st_size != st2.st_size)
		return 1;

	//
	// open the files
	//
	len = (size_t)1 << 17;
	fd1 = open(fn1, O_RDONLY, 0);
	if (fd1 < 0)
		return -1;
	fd2 = open(fn2, O_RDONLY, 0);
	if (fd2 < 0)
	{
		err = errno;
		close(fd1);
		errno = err;
		return -1;
	};

	//
	// allocate the buffers
	//
	errno = 0;
	buf1 = (char *)malloc(len);
	if (!buf1)
	{
		err = errno ? errno : ENOMEM;
		close(fd1);
		close(fd2);
		errno = err;
		return -1;
	}

	errno = 0;
	buf2 = (char *)malloc(len);
	if (!buf2)
	{
		err = errno ? errno : ENOMEM;
		close(fd1);
		close(fd2);
		free(buf1);
		errno = err;
		return -1;
	}

	//
	// read the data and compare
	//
	for (;;)
	{
		n1 = read(fd1, buf1, len);
		if (n1 < 0)
		{
			bomb:
			err = errno;
			close(fd1);
			close(fd2);
			free(buf1);
			free(buf2);
			errno = err;
			return -1;
		}
		n2 = read(fd2, buf2, len);
		if (n2 < 0)
			goto bomb;
		if (!n1 && !n2)
		{
			result = 0;
			break;
		}
		if (n1 != n2)
		{
			//
			// we checked the length above,
			// but it could change while we work
			//
			result = 1;
			break;
		}
		if (memcmp(buf1, buf2, n1))
		{
			result = 1;
			break;
		}
	}
	close(fd1);
	close(fd2);
	free(buf1);
	free(buf2);
	return result;
}

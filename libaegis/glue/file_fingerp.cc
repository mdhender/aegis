//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2003, 2004 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to manipulate file_fingerps
//

#include <ac/errno.h>
#include <ac/string.h>

#include <fp/combined.h>
#include <glue.h>
#include <os.h>


int
file_fingerprint(const char *path, char *buf, int max)
{
	fingerprint_ty	*fp;
	int		result;
	int		len;
	char		tmp[1000];

	os_interrupt_cope();
	fp = fingerprint_new(&fp_combined);
	result = fingerprint_file_sum(fp, path, tmp, sizeof(tmp));
	if (result < 0)
	{
		int err = errno;
		fingerprint_delete(fp);
		errno = err;
		return -1;
	}
	len = strlen(tmp);
	if (len > max)
		len = max;
	memcpy(buf, tmp, len);
	fingerprint_delete(fp);
	return len;
}

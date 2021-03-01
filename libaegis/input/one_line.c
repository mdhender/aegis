/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999 Peter Miller;
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
 * MANIFEST: functions to manipulate one_lines
 */

#include <input.h>
#include <stracc.h>


string_ty *
input_one_line(fp)
	input_ty	*fp;
{
	int		c;
	static stracc_t	buffer;

	stracc_open(&buffer);
	for (;;)
	{
		c = input_getc(fp);
		if (c < 0)
		{
			if (buffer.length == 0)
				return 0;
			break;
		}
		if (c == '\n')
			break;
		stracc_char(&buffer, c);
	}
	return stracc_close(&buffer);
}

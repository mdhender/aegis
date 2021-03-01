/*
 *	aegis - project change supervisor
 *	Copyright (C) 2002 Peter Miller;
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
 * MANIFEST: functions to manipulate read_strict2s
 */

#include <input.h>
#include <str.h>


void
input_read_strictest(ip, buf, len)
    input_ty	*ip;
    void	*buf;
    size_t	len;
{
    long	result;

    result = input_read(ip, buf, len);
    if (result != len)
    {
	string_ty *s = str_format("short read (asked %d, got %d)", len, result);
	input_fatal_error(ip, s->str_text);
    }
}

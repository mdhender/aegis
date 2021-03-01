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
 * MANIFEST: functions to perform common output functions
 */

#include <mem.h>
#include <output/private.h>


output_ty *
output_new(vptr)
	output_vtbl_ty	*vptr;
{
	output_ty	*this;

	this = mem_alloc(vptr->size);
	this->vptr = vptr;
	return this;
}


void
output_generic_fputs(fp, s)
	output_ty	*fp;
	const char	*s;
{
	fp->vptr->write(fp, s, strlen(s));
}


void
output_generic_write(fp, data, len)
	output_ty	*fp;
	const void	*data;
	size_t		len;
{
	const unsigned char *p;

	p = data;
	while (len > 0)
	{
		fp->vptr->fputc(fp, *p++);
		--len;
	}
}

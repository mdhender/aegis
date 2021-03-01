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
 * MANIFEST: functions to manipulate outputs through a common API
 */

#include <mem.h>
#include <mprintf.h>
#include <output/private.h>


void
output_delete(fp)
	output_ty	*fp;
{
	if (fp->vptr->destructor)
		fp->vptr->destructor(fp);
	mem_free(fp);
}


#ifdef output_filename
#undef output_filename
#endif

const char *
output_filename(fp)
	output_ty	*fp;
{
	return fp->vptr->filename(fp);
}


#ifdef output_ftell
#undef output_ftell
#endif

long
output_ftell(fp)
	output_ty	*fp;
{
	return fp->vptr->ftell(fp);
}


#ifdef output_fputc
#undef output_fputc
#endif

void
output_fputc(fp, c)
	output_ty	*fp;
	int		c;
{
	fp->vptr->fputc(fp, c);
}


#ifdef output_fputs
#undef output_fputs
#endif

void
output_fputs(fp, s)
	output_ty	*fp;
	const char	*s;
{
	fp->vptr->fputs(fp, s);
}


#ifdef output_write
#undef output_write
#endif

void
output_write(fp, data, len)
	output_ty	*fp;
	const void	*data;
	size_t		len;
{
	fp->vptr->write(fp, data, len);
}


void
output_fprintf(fp, fmt sva_last)
	output_ty	*fp;
	const char	*fmt;
	sva_last_decl
{
	va_list		ap;

	sva_init(ap, fmt);
	output_vfprintf(fp, fmt, ap);
	va_end(ap);
}


void
output_vfprintf(fp, fmt, ap)
	output_ty	*fp;
	const char	*fmt;
	va_list		ap;
{
	char		*tmp;

	tmp = mem_copy_string(vmprintf(fmt, ap));
	fp->vptr->fputs(fp, tmp);
	mem_free(tmp);
}

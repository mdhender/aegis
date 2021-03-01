/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2001, 2002 Peter Miller;
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

#include <ac/string.h>

#include <error.h> /* for assert */
#include <mem.h>
#include <mprintf.h>
#include <output/private.h>
#include <str.h>
#include <trace.h>


void
output_delete(fp)
	output_ty	*fp;
{
	trace(("output_delete(fp = %08lX)\n{\ntype is output_%s\n",
		(long)fp, fp->vptr->typename));
	assert(fp);
	assert(fp->vptr);
	output_flush(fp);
	if (fp->del_cb)
	{
		output_delete_callback_ty func = fp->del_cb;
		void *arg = fp->del_cb_arg;
		fp->del_cb = 0;
		fp->del_cb_arg = 0;
		func(fp, arg);
	}
	if (fp->vptr->destructor)
		fp->vptr->destructor(fp);
	assert(fp->buffer);
	mem_free(fp->buffer);
	mem_free(fp);
	trace(("}\n"));
}


string_ty *
output_filename(fp)
	output_ty	*fp;
{
	string_ty	*result;

	trace(("output_filename(fp = %08lX)\n{\ntype is output_%s\n",
		(long)fp, fp->vptr->typename));
	assert(fp);
	assert(fp->vptr);
	assert(fp->vptr->filename);
	result = fp->vptr->filename(fp);
	trace(("return \"%s\"\n", result->str_text));
	trace(("}\n"));
	return result;
}


long
output_ftell(fp)
	output_ty	*fp;
{
	long		result;

	trace(("output_ftell(fp = %08lX)\n{\ntype is output_%s\n",
		(long)fp, fp->vptr->typename));
	assert(fp);
	assert(fp->vptr);
	assert(fp->vptr->ftell);
	result = fp->vptr->ftell(fp);
	result += fp->buffer_position - fp->buffer;
	trace(("return %ld;\n", result));
	trace(("}\n"));
	return result;
}


#ifdef output_fputc
#undef output_fputc
#endif

void
output_fputc(fp, c)
	output_ty	*fp;
	int		c;
{
	trace(("output_fputc(fp = %08lX, c = %d)\n{\ntype is output_%s\n",
		(long)fp, c, fp->vptr->typename));
	assert(fp);
	assert(fp->buffer);
	assert(fp->buffer_position >= fp->buffer);
	assert(fp->buffer_end == fp->buffer + fp->buffer_size);
	assert(fp->buffer_position <= fp->buffer_end);
	if (fp->buffer_position >= fp->buffer_end)
	{
		size_t		nbytes;

		assert(fp->vptr);
		assert(fp->vptr->write);
		nbytes = fp->buffer_position - fp->buffer;
		fp->vptr->write(fp, fp->buffer, nbytes);
		fp->buffer_position = fp->buffer;
	}
	*fp->buffer_position++ = c;
	trace(("}\n"));
}


void
output_fputs(fp, s)
	output_ty	*fp;
	const char	*s;
{
	size_t		nbytes;

	trace(("output_fputs(fp = %08lX, s = \"%s\")\n{\ntype is output_%s\n",
		(long)fp, s, fp->vptr->typename));
	nbytes = strlen(s);
	if (nbytes)
		output_write(fp, s, nbytes);
	trace(("}\n"));
}


void
output_write(fp, data, len)
	output_ty	*fp;
	const void	*data;
	size_t		len;
{
	trace(("output_write(fp = %08lX, data = %08lX, len = %ld)\n\
{\ntype is output_%s\n", (long)fp, (long)data, (long)len, fp->vptr->typename));
	assert(fp);
	assert(fp->vptr);
	assert(fp->vptr->write);
	assert(data);
	assert(len);
	if (!len)
	{
		trace(("}\n"));
		return;
	}
	if (fp->buffer_position + len <= fp->buffer_end)
	{
		memcpy(fp->buffer_position, data, len);
		fp->buffer_position += len;
	}
	else
	{
		size_t	nbytes;

		nbytes = fp->buffer_position - fp->buffer;
		if (nbytes)
		{
			fp->vptr->write(fp, fp->buffer, nbytes);
			fp->buffer_position = fp->buffer;
		}

		if (len < fp->buffer_size)
		{
			memcpy(fp->buffer, data, len);
			fp->buffer_position += len;
		}
		else
			fp->vptr->write(fp, data, len);
	}
	trace(("}\n"));
}


void
output_flush(fp)
	output_ty	*fp;
{
	trace(("output_flush(fp = %08lX)\n{\ntype is output_%s\n",
		(long)fp, fp->vptr->typename));
	assert(fp);
	assert(fp->vptr);

	if (fp->buffer_position > fp->buffer)
	{
		size_t	nbytes;

		assert(fp->vptr->write);
		nbytes = fp->buffer_position - fp->buffer;
		fp->vptr->write(fp, fp->buffer, nbytes);
		fp->buffer_position = fp->buffer;
	}

	assert(fp->vptr->flush);
	fp->vptr->flush(fp);
	trace(("}\n"));
}


void
output_end_of_line(fp)
	output_ty	*fp;
{
	/*
	 * If possible, just stuff a newline into the buffer and bail.
	 * This results in the fewest deeper calls.
	 */
	trace(("output_end_of_line(fp = %08lX)\n{\ntype is output_%s\n",
		(long)fp, fp->vptr->typename));
	if
	(
		fp->buffer_position > fp->buffer
	&&
		fp->buffer_position[-1] != '\n'
	&&
		fp->buffer_position < fp->buffer_end
	)
	{
		*fp->buffer_position++ = '\n';
		trace(("}\n"));
		return;
	}

	/*
	 * If there is something in the buffer, we need to flush it,
	 * so that the deeper eoln will have the current state.
	 */
	if (fp->buffer_position > fp->buffer)
	{
		size_t	nbytes;

		assert(fp->vptr->write);
		nbytes = fp->buffer_position - fp->buffer;
		fp->vptr->write(fp, fp->buffer, nbytes);
		fp->buffer_position = fp->buffer;
	}

	/*
	 * Now ask the deeper class to do it's end of line thing.
	 */
	assert(fp->vptr->eoln);
	fp->vptr->eoln(fp);
	trace(("}\n"));
}


int
output_page_width(fp)
	output_ty	*fp;
{
	int		result;

	assert(fp);
	assert(fp->vptr);
	assert(fp->vptr->page_width);
	trace(("output_page_width(fp = %08lX)\n{\ntype is output_%s\n",
		(long)fp, fp->vptr->typename));
	result = fp->vptr->page_width(fp);
	trace(("return %d;\n", result));
	trace(("}\n"));
	return result;
}


int
output_page_length(fp)
	output_ty	*fp;
{
	int		result;

	assert(fp);
	assert(fp->vptr);
	assert(fp->vptr->page_length);
	trace(("output_page_length(fp = %08lX)\n{\ntype is output_%s\n",
		(long)fp, fp->vptr->typename));
	assert(fp->vptr->page_length);
	result = fp->vptr->page_length(fp);
	trace(("return %d;\n", result));
	trace(("}\n"));
	return result;
}


void
output_fprintf(output_ty *fp, const char *fmt, ...)
{
	va_list		ap;

	va_start(ap, fmt);
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

	/*
	 * We have to copy it,
	 * in case a deeper one also uses output_fprintf.
	 * The moral is: avoid printf.
	 */
	tmp = mem_copy_string(vmprintf(fmt, ap));
	output_fputs(fp, tmp);
	mem_free(tmp);
}


void
output_put_str(fp, s)
	output_ty	*fp;
	string_ty	*s;
{
	if (!s || !s->str_length)
		return;
	output_write(fp, s->str_text, s->str_length);
}


void
output_delete_callback(fp, func, arg)
	output_ty	*fp;
	output_delete_callback_ty func;
	void		*arg;
{
	fp->del_cb = func;
	fp->del_cb_arg = arg;
}

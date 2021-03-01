//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2001-2006 Peter Miller
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
// MANIFEST: functions to manipulate outputs through a common API
//

#include <common/ac/string.h>

#include <common/error.h> // for assert
#include <common/mprintf.h>
#include <common/nstring.h>
#include <libaegis/output.h>
#include <common/page.h>
#include <common/trace.h>


output_ty::~output_ty()
{
    //
    // Note: calling the flush() method here is pointless, because
    // the derived class has already been destroyed.
    //
    trace(("~output(this = %08lX)\n{\n", (long)this));
    if (del_cb)
    {
	delete_callback_ty func = del_cb;
	void *arg = del_cb_arg;
	del_cb = 0;
	del_cb_arg = 0;
	func(this, arg);
    }
    assert(buffer);
    delete [] buffer;
    buffer = 0;
    trace(("}\n"));
}


output_ty::output_ty() :
    del_cb(0),
    del_cb_arg(0),
    buffer(0),
    buffer_size(0),
    buffer_position(0),
    buffer_end(0)
{
    buffer_size = (size_t)1 << 13;
    buffer = new unsigned char [buffer_size];
    buffer_position = buffer;
    buffer_end = buffer + buffer_size;
}


long
output_ty::ftell()
    const
{
    trace(("output_ty::ftell(this = %08lX)\n{\n", (long)this));
    long result = ftell_inner() + (buffer_position - buffer);
    trace(("return %ld;\n", result));
    trace(("}\n"));
    return result;
}


void
output_ty::overflow(char c)
{
    trace(("output_ty::overflow(this = %08lX, c = %d)\n{\n", (long)this, c));
    assert(buffer);
    assert(buffer_position >= buffer);
    assert(buffer_end == buffer + buffer_size);
    assert(buffer_position <= buffer_end);
    if (buffer_position >= buffer_end)
    {
	size_t nbytes = buffer_position - buffer;
	write_inner(buffer, nbytes);
	buffer_position = buffer;
    }
    *buffer_position++ = c;
    trace(("}\n"));
}


void
output_ty::fputs(const char *s)
{
    trace(("output_ty::fputs(this = %08lX, s = \"%s\")\n{\n", (long)this, s));
    size_t nbytes = strlen(s);
    if (nbytes)
	write(s, nbytes);
    trace(("}\n"));
}


void
output_ty::fputs(string_ty *s)
{
    if (!s || !s->str_length)
	return;
    write(s->str_text, s->str_length);
}


void
output_ty::fputs(const nstring &s)
{
    if (!s.empty())
	write(s.c_str(), s.length());
}


void
output_ty::write(const void *data, size_t len)
{
    trace(("output_ty::write(this = %08lX, data = %08lX, len = %ld)\n\{\n",
	(long)this, (long)data, (long)len));
    if (len)
    {
	if (buffer_position + len <= buffer_end)
	{
	    memcpy(buffer_position, data, len);
	    buffer_position += len;
	}
	else
	{
	    size_t nbytes = buffer_position - buffer;
	    if (nbytes)
	    {
		write_inner(buffer, nbytes);
		buffer_position = buffer;
	    }
	    if (len < buffer_size)
	    {
		memcpy(buffer, data, len);
		buffer_position += len;
	    }
	    else
		write_inner(data, len);
	}
    }
    trace(("}\n"));
}


void
output_ty::flush()
{
    trace(("output_ty::flush(this = %08lX)\n{\n", (long)this));
    if (buffer_position > buffer)
    {
	size_t nbytes = buffer_position - buffer;
	write_inner(buffer, nbytes);
	buffer_position = buffer;
    }
    flush_inner();
    trace(("}\n"));
}


void
output_ty::end_of_line()
{
    //
    // If possible, just stuff a newline into the buffer and bail.
    // This results in the fewest deeper calls.
    //
    trace(("output_ty::end_of_line(this = %08lX)\n{\n", (long)this));
    if
    (
	buffer_position > buffer
    &&
	buffer_position[-1] != '\n'
    &&
	buffer_position < buffer_end
    )
    {
	*buffer_position++ = '\n';
	trace(("}\n"));
	return;
    }

    //
    // If there is something in the buffer, we need to flush it,
    // so that the deeper eoln will have the current state.
    //
    if (buffer_position > buffer)
    {
	size_t nbytes = buffer_position - buffer;
	write_inner(buffer, nbytes);
	buffer_position = buffer;
    }

    //
    // Now ask the deeper class to do it's end of line thing.
    //
    end_of_line_inner();
    trace(("}\n"));
}


void
output_fprintf(output_ty *fp, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fp->vfprintf(fmt, ap);
    va_end(ap);
}


void
output_ty::fprintf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(fmt, ap);
    va_end(ap);
}


void
output_ty::vfprintf(const char *fmt, va_list ap)
{
    //
    // We have to make a temporary copy of it, in case a deeper output
    // stream also uses output_ty::fprintf to satisfy the virtual
    // write_inner call via output_ty::fputs.
    //
    // The moral is: avoid output_ty::fprintf.
    //
    nstring tmp(nstring::vformat(fmt, ap));
    fputs(tmp);
}


void
output_ty::delete_callback(output_ty::delete_callback_ty func, void *arg)
{
    del_cb = func;
    del_cb_arg = arg;
}


int
output_ty::page_width()
    const
{
    return page_width_get(-1) - 1;
}


int
output_ty::page_length()
    const
{
    return page_length_get(-1);
}


void
output_ty::flush_inner()
{
    // Do nothing.
}

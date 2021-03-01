//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2001-2006, 2008 Peter Miller
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
//	along with this program; if not, see
//	<http://www.gnu.org/licenses/>.
//

#include <common/ac/string.h>

#include <common/error.h> // for assert
#include <common/mprintf.h>
#include <common/nstring.h>
#include <libaegis/output.h>
#include <common/page.h>
#include <common/trace.h>


output::~output()
{
    //
    // Note: calling the flush() method here is pointless, because
    // the derived class has already been destroyed.
    //
    trace(("~output(this = %08lX)\n{\n", (long)this));
    callback();

    assert(buffer);
    delete [] buffer;
    buffer = 0;
    trace(("}\n"));
}


output::output() :
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
output::ftell()
    const
{
    trace(("output::ftell(this = %08lX)\n{\n", (long)this));
    long result = ftell_inner() + (buffer_position - buffer);
    trace(("return %ld;\n", result));
    trace(("}\n"));
    return result;
}


void
output::overflow(char c)
{
    trace(("output::overflow(this = %08lX, c = %d)\n{\n", (long)this, c));
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
output::fputs(const char *s)
{
    trace(("output::fputs(this = %08lX, s = \"%s\")\n{\n", (long)this, s));
    size_t nbytes = strlen(s);
    if (nbytes)
	write(s, nbytes);
    trace(("}\n"));
}


void
output::fputs(string_ty *s)
{
    if (!s || !s->str_length)
	return;
    write(s->str_text, s->str_length);
}


void
output::fputs(const nstring &s)
{
    if (!s.empty())
	write(s.c_str(), s.length());
}


void
output::write(const void *data, size_t len)
{
    trace(("output::write(this = %08lX, data = %08lX, len = %ld)\n{\n",
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
output::flush()
{
    trace(("output::flush(this = %08lX)\n{\n", (long)this));
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
output::end_of_line()
{
    //
    // If possible, just stuff a newline into the buffer and bail.
    // This results in the fewest deeper calls.
    //
    trace(("output::end_of_line(this = %08lX)\n{\n", (long)this));
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
output_fprintf(output::pointer fp, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fp->vfprintf(fmt, ap);
    va_end(ap);
}


void
output::fprintf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(fmt, ap);
    va_end(ap);
}


void
output::vfprintf(const char *fmt, va_list ap)
{
    //
    // We have to make a temporary copy of it, in case a deeper output
    // stream also uses output::fprintf to satisfy the virtual
    // write_inner call via output::fputs.
    //
    // The moral is: avoid output::fprintf.
    //
    nstring tmp(nstring::vformat(fmt, ap));
    fputs(tmp);
}


void
output::register_delete_callback(functor::pointer fp)
{
    callback.push_back(fp);
}


void
output::unregister_delete_callback(functor::pointer fp)
{
    callback.remove(fp);
}


int
output::page_width()
    const
{
    return page_width_get(-1) - 1;
}


int
output::page_length()
    const
{
    return page_length_get(-1);
}


void
output::flush_inner()
{
    // Do nothing.
}

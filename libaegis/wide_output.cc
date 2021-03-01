//
//      aegis - project change supervisor
//      Copyright (C) 1999-2006, 2008 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <common/ac/string.h>

#include <common/error.h> // for assert
#include <common/mem.h>
#include <common/page.h>
#include <common/str.h>
#include <common/trace.h>
#include <common/wstring.h>
#include <libaegis/wide_output.h>


wide_output::~wide_output()
{
    trace(("wide_output::~wide_output(this = %08lX)\n{\n", (long)this));

    //
    // run any delete callbacks specified
    //
    callback();

    //
    // now get rid of it
    //
    delete [] buffer;
    buffer = 0;
    buffer_size = 0;
    buffer_position = 0;
    buffer_end = 0;
    trace(("}\n"));
}


wide_output::wide_output() :
    buffer(0),
    buffer_size(0),
    buffer_position(0),
    buffer_end(0)
{
    buffer_size = (size_t)1 << 11;
    buffer = (wchar_t *)mem_alloc(buffer_size * sizeof(wchar_t));
    buffer_position = buffer;
    buffer_end = buffer + buffer_size;
}


void
wide_output::overflow(wchar_t wc)
{
    trace(("wide_output::overflow(this = %08lX, wc = %04lX)\n{\n", (long)this,
        (long)wc));
    assert(buffer);
    assert(buffer_size);
    assert(buffer_position >= buffer);
    assert(buffer_end == buffer + buffer_size);
    assert(buffer_position <= buffer_end);
    if (buffer_position >= buffer_end)
    {
        write_inner(buffer, buffer_size);
        buffer_position = buffer;
    }
    *buffer_position++ = wc;
    trace(("}\n"));
}


void
wide_output::put_ws(const wchar_t *s)
{
    trace(("wide_output::put_ws(fp = %08lX, s = %08lX)\n{\n", (long)this,
        (long)s));
    if (s)
    {
        const wchar_t *wse = s;
        while (*wse)
            ++wse;
        if (wse > s)
            write(s, wse - s);
    }
    trace(("}\n"));
}


void
wide_output::write(const wstring &s)
{
    trace(("wide_output::write(this = %08lX)\n{\n", (long)this));
    write(s.c_str(), s.size());
    trace(("}\n"));
}


void
wide_output::write(const wchar_t *data, size_t len)
{
    trace(("wide_output::write(this = %08lX, data = %08lX, len = %ld)\n{\n",
        (long)this, (long)data, (long)len));
    assert(data);
    // assert(len); ideal, but not necessary
    if (buffer_position + len <= buffer_end)
    {
        memcpy(buffer_position, data, len * sizeof(wchar_t));
        buffer_position += len;
    }
    else
    {
        size_t nwc = buffer_position - buffer;
        write_inner(buffer, nwc);
        buffer_position = buffer;

        if (len < buffer_size)
        {
            memcpy(buffer, data, len * sizeof(wchar_t));
            buffer_position += len;
        }
        else
            write_inner(data, len);
    }
    trace(("}\n"));
}


void
wide_output::flush()
{
    trace(("wide_output::flush(this = %08lX)\n{\n", (long)this));
    if (buffer_position > buffer)
    {
        size_t nwc = buffer_position - buffer;
        write_inner(buffer, nwc);
        buffer_position = buffer;
    }
    flush_inner();
    trace(("}\n"));
}


void
wide_output::end_of_line()
{
    //
    // If possible, just stuff a newline into the buffer and bail.
    // This results in the fewest deeper calls.
    //
    trace(("wide_output::end_of_line(this = %08lX)\n{\n", (long)this));
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
        size_t nwc = buffer_position - buffer;
        write_inner(buffer, nwc);
        buffer_position = buffer;
    }

    //
    // Now ask the deeper instance to do it's end of line thing.
    //
    end_of_line_inner();
    trace(("}\n"));
}


void
wide_output::register_delete_callback(functor::pointer cb)
{
    callback.push_back(cb);
}


void
wide_output::unregister_delete_callback(functor::pointer cb)
{
    callback.remove(cb);
}


void
wide_output::put_wstr(wstring_ty *wsp)
{
    if (!wsp || !wsp->wstr_length)
        return;
    write(wsp->wstr_text, wsp->wstr_length);
}


void
wide_output::fputs(string_ty *s)
{
    if (!s || !s->str_length)
        return;
    wstring_ty *s2 = wstr_n_from_c(s->str_text, s->str_length);
    put_wstr(s2);
    wstr_free(s2);
}


void
wide_output::put_cstr(const char *s)
{
    if (!s || !*s)
        return;
    wstring_ty *s2 = wstr_from_c(s);
    put_wstr(s2);
    wstr_free(s2);
}


int
wide_output::page_width()
{
    return page_width_get(-1) - 1;
}


int
wide_output::page_length()
{
    return page_length_get(-1);
}

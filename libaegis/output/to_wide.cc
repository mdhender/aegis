//
// aegis - project change supervisor
// Copyright (C) 1999, 2001-2008, 2011, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>
#include <common/ac/wchar.h>

#include <common/language.h>
#include <common/mem.h>
#include <common/str.h>
#include <common/trace.h>
#include <libaegis/output.h>
#include <libaegis/output/to_wide.h>
#include <libaegis/wide_output.h>


static mbstate_t initial_state;


output_to_wide::~output_to_wide()
{
    trace(("~output_to_wide(this = %p)\n{\n", this));

    //
    // Make sure all buffered data has been passed to our write_inner
    // method.
    //
    flush();

    //
    // If there are any input characters left, they are probably a
    // partial which is never going to be completed.
    //
    language_human();
    while (input_len)
    {
        //
        // The state represents the state before the multi-byte
        // character is scanned.  If there is an error, we want
        // to be able to restore it.
        //
        mbstate_t sequester = input_state;
        wchar_t wc = 0;
        int n = mbrtowc(&wc, input_buf, input_len, &input_state);
        if (n == 0)
        {
            //
            // This only happens if there is a NUL in
            // the input stream.  It shouldn't happen,
            // so ignore it.
            //
            n = 1;
            goto skip_one;
        }
        if (n < 0)
        {
            //
            // Invalid multi byte sequence.  (Even though a -2 return
            // value means it could simply be incomplete, we know
            // it's illegal because we are never getting any more
            // characters.)  Restore the state.
            //
            input_state = sequester;

            //
            // It's an illegal sequence.  Use the first
            // character in the buffer, and shuffle the
            // rest down.
            //
            wc = (unsigned char)input_buf[0];
            n = 1;
        }

        //
        // stash the output
        //
        if (output_len >= output_max)
        {
            size_t new_output_max = 32 + 2 * output_max;
            wchar_t *new_output_buf = new wchar_t [new_output_max];
            if (output_len)
            {
                size_t nbytes = sizeof(output_buf[0]) * output_len;
                memcpy(new_output_buf, output_buf, nbytes);
            }
            delete [] output_buf;
            output_buf = new_output_buf;
            output_max = new_output_max;
        }
        output_buf[output_len++] = wc;

        //
        // the one wchar_t used n chars
        //
        skip_one:
        assert((size_t)n <= input_len);
        if ((size_t)n >= input_len)
            input_len = 0;
        else
        {
            memmove(input_buf, input_buf + n, input_len - n);
            input_len -= n;
        }
    }
    language_C();

    //
    // Let the buffers go.
    //
    delete [] input_buf;
    input_buf = 0;
    delete [] output_buf;
    output_buf = 0;

    //
    // paranoia
    //
    input_len = 0;
    input_max = 0;
    input_state = initial_state;
    output_len = 0;
    output_max = 0;
    trace(("}\n"));
}


output_to_wide::output_to_wide(const wide_output::pointer &a_deeper) :
    deeper(a_deeper),
    input_buf(0),
    input_len(0),
    input_max(0),
    input_state(initial_state),
    input_bol(true),
    output_buf(0),
    output_len(0),
    output_max(0)
{
}


output::pointer
output_to_wide::open(const wide_output::pointer &a_deeper)
{
    return pointer(new output_to_wide(a_deeper));
}


void
output_to_wide::flush_inner(void)
{
    trace(("output_to_wide::flush_inner(this = %p)\n{\n", this));
    if (output_len > 0)
    {
        deeper->write(output_buf, output_len);
        output_len = 0;
    }
    deeper->flush();
    trace(("}\n"));
}


nstring
output_to_wide::filename(void)
    const
{
    trace(("output_to_wide::filename(this = %p)\n{\n", this));
    nstring result = deeper->filename();
    trace(("return \"%s\";\n", result.c_str()));
    trace(("}\n"));
    return result;
}


long
output_to_wide::ftell_inner(void)
    const
{
    trace(("output_to_wide::ftell_inner(fp = %p)\n", this));
    return -1;
}


void
output_to_wide::write_inner(const void *input_p, size_t len)
{
    trace(("output_to_wide::write_inner(this = %p, data = %p, "
        "len = %ld)\n{\n", this, input_p, (long)len));
    language_human();
    const char *ip = (const char *)input_p;
    while (len > 0)
    {
        unsigned char c = *ip++;
        --len;
        trace(("c = %s\n", unctrl(c)));

        //
        // Track whether we are at the start of a line.
        // (This makes the assumption that \n will not be part
        // of any multi-byte sequence.)
        //
        input_bol = (c == '\n');

        //
        // The NUL character is not legal in the output stream.
        // Throw it away if we see it.
        //
        assert(c);
        if (c == 0)
            continue;

        //
        // drop the character into the buffer
        //
        if (input_len >= input_max)
        {
            size_t new_input_max = 4 + 2 * input_max;
            char *new_input_buf = new char [new_input_max];
            if (input_len)
                memcpy(new_input_buf, input_buf, input_len);
            delete [] input_buf;
            input_buf = new_input_buf;
            input_max = new_input_max;
        }
        input_buf[input_len++] = c;

        //
        // The state represents the state before the multi-byte
        // character is scanned.  If there is an error, we want
        // to be able to restore it.
        //
        reprocess_buffer:
        mbstate_t sequester = input_state;
        wchar_t wc = 0;
        int n = mbrtowc(&wc, input_buf, input_len, &input_state);
        if (n == 0)
        {
            //
            // This only happens if there is a NUL in
            // the ip stream.  It shouldn't happen,
            // so ignore it.
            //
            n = 1;
            goto skip_one;
        }
        if (n < 0)
        {
            //
            // Invalid multi byte sequence.
            // Restore the state.
            //
            input_state = sequester;

            //
            // If the n bytes starting at s do not contain a complete
            // multibyte character, mbrtowc returns (size_t)(-2).  This
            // can happen even if input_len >= MB_CUR_MAX,
            // if the multibyte string contains redundant shift
            // sequences.
            //
            if (n == -2)
            {
                continue;
            }

            //
            // It's an illegal sequence.  Use the first
            // character in the buffer, and shuffle the
            // rest down.
            //
            wc = (unsigned char)input_buf[0];
            n = 1;
        }

        //
        // stash the output
        //
        if (output_len >= output_max)
        {
            size_t new_output_max = 32 + 2 * output_max;
            wchar_t *new_output_buf = new wchar_t [new_output_max];
            if (output_len)
            {
                size_t nbytes = sizeof(output_buf[0]) * output_len;
                memcpy(new_output_buf, output_buf, nbytes);
            }
            delete [] output_buf;
            output_buf = new_output_buf;
            output_max = new_output_max;
        }
        output_buf[output_len++] = wc;

        //
        // If the output buffer is starting to fill up, empty it.
        //
        if (output_len >= 1024)
        {
            language_C();
            flush_inner();
            language_human();
        }

        //
        // the one wchar_t used n chars
        //
        skip_one:
        assert((size_t)n <= input_len);
        if ((size_t)n >= input_len)
            input_len = 0;
        else
        {
            memmove
            (
                input_buf,
                input_buf + n,
                input_len - n
            );
            input_len -= n;
            goto reprocess_buffer;
        }
    }
    language_C();
    trace(("}\n"));
}


int
output_to_wide::page_width(void)
    const
{
    trace(("output_to_wide::page_width(this = %p)\n", this));
    return deeper->page_width();
}


int
output_to_wide::page_length(void)
    const
{
    trace(("output_to_wide::page_length(this = %p)\n", this));
    return deeper->page_length();
}


void
output_to_wide::end_of_line_inner(void)
{
    trace(("output_to_wide::end_of_line_inner(this = %p)\n{\n", this));
    if (!input_bol)
        fputc('\n');
    trace(("}\n"));
}


nstring
output_to_wide::type_name(void)
    const
{
    return "to_wide";
}


// vim: set ts=8 sw=4 et :

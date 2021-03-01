//
//      aegis - project change supervisor
//      Copyright (C) 1999, 2001-2005 Peter Miller;
//      All rights reserved.
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 2 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to manipulate to_wides
//

#include <common/ac/stdlib.h>
#include <common/ac/string.h>
#include <common/ac/wchar.h>

#include <common/error.h> // for assert
#include <common/language.h>
#include <libaegis/output.h>
#include <libaegis/output/to_wide.h>
#include <common/str.h>
#include <common/trace.h>
#include <libaegis/wide_output.h>


static mbstate_t initial_state;


void
output_to_wide_ty::flush_inner()
{
    trace(("output_to_wide::flush(this = %08lX)\n{\n", (long)this));
    if (output_len > 0)
    {
        wide_output_write(deeper, output_buf, output_len);
        output_len = 0;
    }
    wide_output_flush(deeper);
    trace(("}\n"));
}


output_to_wide_ty::~output_to_wide_ty()
{
    trace(("~output_to_wide(this = %08lX)\n{\n", (long)this));

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
            memmove
	    (
		input_buf,
		input_buf + n,
		input_len - n
	    );
            input_len -= n;
        }
    }
    language_C();

    //
    // Delete the deeper output if we were asked to.
    //
    if (close_on_close)
        wide_output_delete(deeper);
    deeper = 0;

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
    close_on_close = 0;
    input_len = 0;
    input_max = 0;
    input_state = initial_state;
    output_len = 0;
    output_max = 0;
    trace(("}\n"));
}


string_ty *
output_to_wide_ty::filename()
    const
{
    trace(("output_to_wide::filename(this = %08lX)\n{\n", (long)this));
    string_ty *result = wide_output_filename(deeper);
    trace(("return \"%s\";\n", result->str_text));
    trace(("}\n"));
    return result;
}


long
output_to_wide_ty::ftell_inner()
    const
{
    trace(("output_to_wide_ty::ftell_inner(fp = %08lX)\n", (long)this));
    return -1;
}


void
output_to_wide_ty::write_inner(const void *input_p, size_t len)
{
    trace(("output_to_wide_ty::write_inner(this = %08lX, data = %08lX, "
	"len = %ld)\n{\n", (long)this, (long)input_p, (long)len));
    language_human();
    const char *input = (const char *)input_p;
    while (len > 0)
    {
        unsigned char c = *input++;
        --len;
	trace(("c = %s\n", unctrl(c)));

        //
        // Track whether we are at the start of a line.
        // (This makes the assumption that \n will no be part
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
            // the input stream.  It shouldn't happen,
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
output_to_wide_ty::page_width()
    const
{
    trace(("output_to_wide::page_width(this = %08lX)\n", (long)this));
    return wide_output_page_width(deeper);
}


int
output_to_wide_ty::page_length()
    const
{
    trace(("output_to_wide::page_length(this = %08lX)\n", (long)this));
    return wide_output_page_length(deeper);
}


void
output_to_wide_ty::end_of_line_inner()
{
    trace(("output_to_wide::end_of_line_inner(this = %08lX)\n{\n", (long)this));
    if (!input_bol)
        fputc('\n');
    trace(("}\n"));
}


const char *
output_to_wide_ty::type_name()
    const
{
    return "to_wide";
}


output_to_wide_ty::output_to_wide_ty(wide_output_ty *arg1, bool arg2) :
    deeper(arg1),
    close_on_close(arg2),
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

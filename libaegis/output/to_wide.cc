//
//      aegis - project change supervisor
//      Copyright (C) 1999, 2001-2004 Peter Miller;
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

#include <ac/stdlib.h>
#include <ac/string.h>

#include <error.h> // for assert
#include <language.h>
#include <mem.h>
#include <output.h>
#include <output/private.h>
#include <output/to_wide.h>
#include <str.h>
#include <trace.h>
#include <wide_output.h>


typedef struct output_to_wide_ty output_to_wide_ty;
struct output_to_wide_ty
{
    output_ty       inherited;
    wide_output_ty  *deeper;
    int             delete_on_close;

    char            *input_buf;
    size_t          input_len;
    size_t          input_max;
    mbstate_t       input_state;                                //lint !e43
    int             input_bol;

    wchar_t         *output_buf;
    size_t          output_len;
    size_t          output_max;
};

static mbstate_t initial_state; //lint !e86


static void
output_to_wide_flush(output_ty *fp)
{
    output_to_wide_ty *this_thing;

    trace(("output_to_wide::flush(fp = %08lX)\n{\n", (long)fp));
    this_thing = (output_to_wide_ty *)fp;
    if (this_thing->output_len > 0)
    {
        wide_output_write(this_thing->deeper,
                          this_thing->output_buf,
                          this_thing->output_len);
        this_thing->output_len = 0;
    }
    wide_output_flush(this_thing->deeper);
    trace(("}\n"));
}


static void
output_to_wide_destructor(output_ty *fp)
{
    output_to_wide_ty *this_thing;

    trace(("output_to_wide::destructor(fp = %08lX)\n{\n", (long)fp));
    this_thing = (output_to_wide_ty *)fp;

    //
    // If there are any input characters left, they are probably a
    // partial which is never going to be completed.
    //
    language_human();
    while (this_thing->input_len)
    {
        int             n;
        wchar_t         wc;
        mbstate_t       sequester;                              //lint !e86

        //
        // The state represents the state before the multi-byte
        // character is scanned.  If there is an error, we want
        // to be able to restore it.
        //
        sequester = this_thing->input_state;
        n = mbrtowc(&wc,
                    this_thing->input_buf,
                    this_thing->input_len,
                    &this_thing->input_state);
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
            this_thing->input_state = sequester;

            //
            // It's an illegal sequence.  Use the first
            // character in the buffer, and shuffle the
            // rest down.
            //
            wc = (unsigned char)this_thing->input_buf[0];
            n = 1;
        }

        //
        // stash the output
        //
        if (this_thing->output_len >= this_thing->output_max)
        {
            size_t          nbytes;

            this_thing->output_max = 32 + 2 * this_thing->output_max;
            nbytes = sizeof(this_thing->output_buf[0]) * this_thing->output_max;
            this_thing->output_buf =
                (wchar_t *)mem_change_size(this_thing->output_buf, nbytes);
        }
        this_thing->output_buf[this_thing->output_len++] = wc;

        //
        // the one wchar_t used n chars
        //
        skip_one:
        assert((size_t)n <= this_thing->input_len);
        if ((size_t)n >= this_thing->input_len)
            this_thing->input_len = 0;
        else
        {
            memmove
	    (
		this_thing->input_buf,
		this_thing->input_buf + n,
		this_thing->input_len - n
	    );
            this_thing->input_len -= n;
        }
    }
    language_C();

    //
    // Flush the output buffer
    //
    output_to_wide_flush(fp);

    //
    // Delete the deeper output if we were asked to.
    //
    if (this_thing->delete_on_close)
        wide_output_delete(this_thing->deeper);

    //
    // Let the buffers go.
    //
    if (this_thing->input_buf)
        mem_free(this_thing->input_buf);
    if (this_thing->output_buf)
        mem_free(this_thing->output_buf);

    //
    // paranoia
    //
    this_thing->deeper = 0;
    this_thing->delete_on_close = 0;
    this_thing->input_buf = 0;
    this_thing->input_len = 0;
    this_thing->input_max = 0;
    this_thing->input_state = initial_state;
    this_thing->output_buf = 0;
    this_thing->output_len = 0;
    this_thing->output_max = 0;
    trace(("}\n"));
}


static string_ty *
output_to_wide_filename(output_ty *fp)
{
    output_to_wide_ty *this_thing;
    string_ty       *result;

    trace(("output_to_wide::filename(fp = %08lX)\n{\n", (long)fp));
    this_thing = (output_to_wide_ty *)fp;
    result = wide_output_filename(this_thing->deeper);
    trace(("return \"%s\";\n", result->str_text));
    trace(("}\n"));
    return result;
}


static long
output_to_wide_ftell(output_ty *fp)
{
    trace(("output_to_wide::ftell(fp = %08lX)\n", (long)fp));
    return -1;
}


static void
output_to_wide_write(output_ty *fp, const void *input_p, size_t len)
{
    const char      *input;
    output_to_wide_ty *this_thing;

    trace(("output_to_wide::write(fp = %08lX, data = %08lX, len = %ld)\n{\n",
        (long)fp, (long)input_p, (long)len));
    this_thing = (output_to_wide_ty *)fp;
    language_human();
    input = (const char *)input_p;
    while (len > 0)
    {
        int             c;
        int             n;
        wchar_t         wc;
        mbstate_t       sequester;                              //lint !e86

        c = (unsigned char)*input++;
        --len;
	trace(("c = %s\n", unctrl(c)));

        //
        // Track whether we are at the start of a line.
        // (This makes the assumption that \n will no be part
        // of any multi-byte sequence.)
        //
        this_thing->input_bol = (c == '\n');

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
        if (this_thing->input_len >= this_thing->input_max)
        {
            this_thing->input_max = 4 + 2 * this_thing->input_max;
            this_thing->input_buf =
                (char *)
		mem_change_size(this_thing->input_buf, this_thing->input_max);
        }
        this_thing->input_buf[this_thing->input_len++] = c;

        //
        // The state represents the state before the multi-byte
        // character is scanned.  If there is an error, we want
        // to be able to restore it.
        //
	reprocess_buffer:
        sequester = this_thing->input_state;
        n = mbrtowc(&wc,
                    this_thing->input_buf,
                    this_thing->input_len,
                    &this_thing->input_state);
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
            this_thing->input_state = sequester;

            //
	    // If the n bytes starting at s do not contain a complete
	    // multibyte character, mbrtowc returns (size_t)(-2).  This
            // can happen even if this_thing->input_len >= MB_CUR_MAX,
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
            wc = (unsigned char)this_thing->input_buf[0];
            n = 1;
        }

        //
        // stash the output
        //
        if (this_thing->output_len >= this_thing->output_max)
        {
            size_t          nbytes;

            this_thing->output_max = 32 + 2 * this_thing->output_max;
            nbytes =
                sizeof(this_thing->output_buf[0]) * this_thing->output_max;
            this_thing->output_buf =
                (wchar_t *)mem_change_size(this_thing->output_buf, nbytes);
        }
        this_thing->output_buf[this_thing->output_len++] = wc;

        //
        // If the output buffer is starting to fill up, empty it.
        //
        if (this_thing->output_len >= 1024)
        {
            language_C();
            output_to_wide_flush(fp);
            language_human();
        }

        //
        // the one wchar_t used n chars
        //
        skip_one:
        assert((size_t)n <= this_thing->input_len);
        if ((size_t)n >= this_thing->input_len)
            this_thing->input_len = 0;
        else
        {
            memmove
	    (
		this_thing->input_buf,
		this_thing->input_buf + n,
		this_thing->input_len - n
	    );
            this_thing->input_len -= n;
	    goto reprocess_buffer;
        }
    }
    language_C();
    trace(("}\n"));
}


static int
output_to_wide_page_width(output_ty *fp)
{
    output_to_wide_ty *this_thing;

    trace(("output_to_wide::width(fp = %08lX)\n", (long)fp));
    this_thing = (output_to_wide_ty *)fp;
    return wide_output_page_width(this_thing->deeper);
}


static int
output_to_wide_page_length(output_ty *fp)
{
    output_to_wide_ty *this_thing;

    trace(("output_to_wide::length(fp = %08lX)\n", (long)fp));
    this_thing = (output_to_wide_ty *)fp;
    return wide_output_page_length(this_thing->deeper);
}


static void
output_to_wide_eoln(output_ty *fp)
{
    output_to_wide_ty *this_thing;

    trace(("output_to_wide::eoln(fp = %08lX)\n{\n", (long)fp));
    this_thing = (output_to_wide_ty *)fp;
    if (!this_thing->input_bol)
        output_fputc(fp, '\n');
    trace(("}\n"));
}


static output_vtbl_ty vtbl =
{
    sizeof(output_to_wide_ty),
    output_to_wide_destructor,
    output_to_wide_filename,
    output_to_wide_ftell,
    output_to_wide_write,
    output_to_wide_flush,
    output_to_wide_page_width,
    output_to_wide_page_length,
    output_to_wide_eoln,
    "to_wide",
};


output_ty *
output_to_wide_open(wide_output_ty *deeper, int delete_on_close)
{
    output_ty       *result;
    output_to_wide_ty *this_thing;

    trace(("output_to_wide::new(deeper = %08lX)\n{\n", (long)deeper));
    result = output_new(&vtbl);
    this_thing = (output_to_wide_ty *)result;
    this_thing->deeper = deeper;
    this_thing->delete_on_close = delete_on_close;
    this_thing->input_buf = 0;
    this_thing->input_len = 0;
    this_thing->input_max = 0;
    this_thing->input_state = initial_state;
    this_thing->input_bol = 1;
    this_thing->output_buf = 0;
    this_thing->output_len = 0;
    this_thing->output_max = 0;
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}

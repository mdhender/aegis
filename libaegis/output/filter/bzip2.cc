//
// aegis - project change supervisor
// Copyright (C) 2006, 2008, 2011, 2012 Peter Miller
// Copyright (C) 2007, 2009 Walter Franzini
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
// along with this program.  If not, see <http://www.gnu.org/licences/>.
//

#include <common/trace.h>

#include <common/mem.h>
#include <libaegis/output/filter/bzip2.h>
#include <libaegis/sub.h>


void
output_filter_bzip2::bzlib_fatal_error(int err)
{
    sub_context_ty sc;
    sc.var_set_charstar("ERRNO", BZ2_strerror(err));
    sc.var_override("ERRNO");
    sc.var_set_string("File_Name", output_filter::filename());
    sc.fatal_intl(i18n("gzip $filename: $errno"));
}


output_filter_bzip2::~output_filter_bzip2()
{
    trace(("output_filter_bzip2::~output_filter_bzip2(this = %p)\n{\n", this));

    //
    // Make sure all buffered data has been passed to our write_inner
    // method.
    //
    flush();

    //
    // finish sending the compressed stream
    //
    for (;;)
    {
        trace(("Before call to BZ2_bzCompress FIN:\n"));
        trace(("stream.avail_in = %d\n", stream.avail_in));
        trace(("stream.next_in = %p\n", stream.next_in));
        trace(("stream.avail_out = %d\n", stream.avail_out));
        trace(("stream.next_out = %p\n", stream.next_out));
        int err = BZ2_bzCompress(&stream, BZ_FINISH);
        if (err != BZ_FINISH_OK && err != BZ_STREAM_END)
            bzlib_fatal_error(err);
        trace(("After call to BZ2_bzCompress FIN:\n"));
        trace(("stream.avail_in = %d\n", stream.avail_in));
        trace(("stream.next_in = %p\n", stream.next_in));
        trace(("stream.avail_out = %d\n", stream.avail_out));
        trace(("stream.next_out = %p\n", stream.next_out));
        if (stream.avail_out < BUFFER_SIZE)
        {
            int n = BUFFER_SIZE - stream.avail_out;
            deeper_write(buf, n);
            stream.avail_out = BUFFER_SIZE;
            stream.next_out = buf;
        }
        if (err == BZ_STREAM_END)
            break;
    }

    //
    // Clean up any resources we were using.
    //
    delete [] buf;
    buf = 0;
    trace(("}\n"));
}


output_filter_bzip2::output_filter_bzip2(const output::pointer &a_deeper) :
    output_filter(a_deeper),
    buf(new char [BUFFER_SIZE]),
    pos(0),
    bol(true)
{
    trace(("output_filter_bzip2::output_filter_bzip2(this = %p)\n{\n", this));
    stream.bzalloc = 0;
    stream.bzfree = 0;
    stream.opaque = 0;
    stream.avail_in = 0;
    stream.next_in = 0;
    stream.avail_out = BUFFER_SIZE;
    stream.next_out = buf;

    int block_size_100k = 3;
    int verbosity = 0;
    int work_factor = 30;
    int err =
        BZ2_bzCompressInit(&stream, block_size_100k, verbosity, work_factor);
    if (err != BZ_OK)
        bzlib_fatal_error(err);
    trace(("stream.avail_in = %d\n", stream.avail_in));
    trace(("stream.next_in = %p\n", stream.next_in));
    trace(("stream.avail_out = %d\n", stream.avail_out));
    trace(("stream.next_out = %p\n", stream.next_out));
    trace(("}\n"));
}


output::pointer
output_filter_bzip2::create(const output::pointer &a_deeper)
{
    return pointer(new output_filter_bzip2(a_deeper));
}


long
output_filter_bzip2::ftell_inner(void)
    const
{
    trace(("output_filter_bzip2::ftell_inner(this = %p) = %ld\n", this, pos));
    return pos;
}


void
output_filter_bzip2::write_inner(const void *data, size_t len)
{
    trace(("output_filter_bzip2::write_inner(this = %p, data = %p, len = %ld)\n"
        "{\n", this, data, (long)len));
    if (!data)
        bzlib_fatal_error(BZ_PARAM_ERROR);
    if (len == 0)
    {
        trace(("}\n"));
        return;
    }
    bol = (((const char *)buf)[len - 1] == '\n');
    pos += len;
    stream.avail_in = len;
    stream.next_in = (char *)data;
    for (;;)
    {
        trace(("Before call to BZ2_bzCompress:\n"));
        trace(("stream.avail_in = %d\n", stream.avail_in));
        trace(("stream.next_in = %p\n", stream.next_in));
        trace(("stream.avail_out = %d\n", stream.avail_out));
        trace(("stream.next_out = %p\n", stream.next_out));
        int err = BZ2_bzCompress(&stream, BZ_RUN);
        if (err != BZ_RUN_OK)
            bzlib_fatal_error(err);
        trace(("After call to BZ2_bzCompress:\n"));
        trace(("stream.avail_in = %d\n", stream.avail_in));
        trace(("stream.next_in = %p\n", stream.next_in));
        trace(("stream.avail_out = %d\n", stream.avail_out));
        trace(("stream.next_out = %p\n", stream.next_out));
        if (stream.avail_out < BUFFER_SIZE)
        {
            int n = BUFFER_SIZE - stream.avail_out;
            deeper_write(buf, n);
            stream.avail_out = BUFFER_SIZE;
            stream.next_out = buf;
        }
        if (stream.avail_in == 0)
        {
            trace(("}\n"));
            return;
        }
    }
}


void
output_filter_bzip2::end_of_line_inner(void)
{
    if (!bol)
        write_inner("\n", 1);
}


nstring
output_filter_bzip2::type_name(void)
    const
{
    return ("bzip2 " + output_filter::type_name());
}


// vim: set ts=8 sw=4 et :

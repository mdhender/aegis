//
//      aegis - project change supervisor
//      Copyright (C) 2006-2008, 2012 Peter Miller
//      Copyright (C) 2007, 2009 Walter Franzini
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
//      along with this program.  If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <common/ac/string.h>
#include <common/ac/sys/resource.h>

#include <common/mem.h>
#include <libaegis/input/bunzip2.h>
#include <libaegis/sub.h>


void
input_bunzip2::bzlib_fatal_error(int err)
{
    if (err >= 0)
        return;
    sub_context_ty sc;
    sc.var_set_charstar("ERRNO", BZ2_strerror(err));
    sc.var_override("ERRNO");
    sc.var_set_string("File_Name", deeper->name());
    sc.fatal_intl(i18n("gunzip $filename: $errno"));
}


input_bunzip2::~input_bunzip2()
{
    //
    // Ask bzlib to free any stream resources it may be using.
    //
    int err = BZ2_bzDecompressEnd(&stream);
    if (err < 0)
        bzlib_fatal_error(err);

    //
    // Return unused input to the deeper stream.
    //
    if (stream.avail_in)
    {
        deeper->unread(stream.next_in, stream.avail_in);
        stream.next_in = 0;
        stream.avail_in = 0;
    }

    delete [] buf;
    buf = 0;
    deeper.close();
}


input_bunzip2::input_bunzip2(input &arg) :
    deeper(arg),
    end_of_file(false),
    pos(0),
    buf(new char [BUFFER_SIZE])
{
    stream.bzalloc = 0;
    stream.bzfree = 0;
    stream.opaque = 0;

    int verbosity = 0;

    //
    // If we are running with limited resources we use a slower
    // algorithm that use less memory.
    //
    int small = 0;
    struct rlimit memory_limit;
    getrlimit(RLIMIT_AS, &memory_limit);
    if (memory_limit.rlim_cur != RLIM_INFINITY)
        small = 1;
    int err = BZ2_bzDecompressInit(&stream, verbosity, small);
    if (err != BZ_OK)
        bzlib_fatal_error(err);
    stream.avail_in = 0;
    stream.next_in = 0;
}


ssize_t
input_bunzip2::read_inner(void *data, size_t len)
{
    if (end_of_file)
        return 0;

    if (!data)
    {
        bzlib_fatal_error(BZ_PARAM_ERROR);
        //NOTREACHED
    }
    if (len == 0)
        return 0;

    stream.avail_out = len;
    stream.next_out = (char *)data;

    for (;;)
    {
        if (stream.avail_in == 0)
        {
            long n = deeper->read(buf, BUFFER_SIZE);
            stream.avail_in = n;
            stream.next_in = buf;
        }

        int err = BZ2_bzDecompress(&stream);
        switch (err)
        {
        case BZ_OK:
            if
            (
                deeper->at_end()
            &&
                stream.avail_in == 0
            &&
                stream.avail_out > 0
            )
            {
                bzlib_fatal_error(BZ_UNEXPECTED_EOF);
                //NOTREACHED
            }
            break;

        case BZ_STREAM_END:
            {
                end_of_file = true;
                int nbytes = (len - stream.avail_out);
                pos += nbytes;
                return nbytes;
            }

        default:
            bzlib_fatal_error(err);
            //NOTREACHED
        }

        if (stream.avail_out == 0)
        {
            pos += len;
            return len;
        }
    }
}


off_t
input_bunzip2::ftell_inner()
{
    return pos;
}


nstring
input_bunzip2::name()
{
    if (filename.empty())
    {
        nstring s = deeper->name();
        if (s.ends_with_nocase(".bz"))
            filename = nstring(s.c_str(), s.size() - 3);
        else if (s.ends_with_nocase(".bz2"))
            filename = nstring(s.c_str(), s.size() - 4);
        else if (s.ends_with_nocase(".tbz"))
        {
            filename =
                nstring::format("%.*s.tar", (int)s.size() - 4, s.c_str());
        }
        else
            filename = s;
    }
    return filename;
}


off_t
input_bunzip2::length()
{
    //
    // We have no idea how long the decompressed stream will be.
    //
    return -1;
}


bool
input_bunzip2::candidate(input &p_deeper)
{
    //
    // Check for the magic number.
    //
    unsigned char magic[4];
    long n = p_deeper->read(magic, 4);
    p_deeper->unread(magic, n);
    return
        (
            n == 4
        &&
            magic[0] == 'B'
        &&
            magic[1] == 'Z'
        &&
            magic[2] == 'h'
        &&
            magic[3] >= '1'
        &&
            magic[3] <= '9'
        );
}


void
input_bunzip2::keepalive()
{
    deeper->keepalive();
}


input
input_bunzip2_open(input &deeper)
{
    if (!input_bunzip2::candidate(deeper))
    {
        //
        // If it is not actually a compressed file,
        // simply return the deeper file.  This will
        // give much better performance.
        //
        return deeper;
    }
    return new input_bunzip2(deeper);
}


bool
input_bunzip2::is_remote()
    const
{
    return deeper->is_remote();
}


// vim: set ts=8 sw=4 et :

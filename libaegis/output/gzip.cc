//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002-2006, 2008 Peter Miller
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
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <common/mem.h>
#include <libaegis/output/gzip.h>
#include <libaegis/sub.h>


#ifndef Z_BUFSIZE
#ifdef MAXSEG_64K
#define Z_BUFSIZE 4096 // minimize memory usage for 16-bit DOS
#else
#define Z_BUFSIZE 16384
#endif
#endif

static int gzip_magic[2] = {0x1f, 0x8b}; // gzip magic header


void
output_gzip::drop_dead(int err)
{
    sub_context_ty sc;
    sc.var_set_charstar("ERRNO", z_error(err));
    sc.var_override("ERRNO");
    sc.var_set_string("File_Name", deeper->filename());
    sc.fatal_intl(i18n("gzip $filename: $errno"));
}


//
// Outputs a long in LSB order to the given file
//	(little endian)
//

static void
output_long_le(output::pointer fp, uLong x)
{
    for (int n = 0; n < 4; n++)
    {
	fp->fputc((unsigned char)x);
	x >>= 8;
    }
}


output_gzip::~output_gzip()
{
    //
    // Make sure all buffered data has been passed to our write_inner
    // method.
    //
    flush();

    //
    // finish sending the compressed stream
    //
    stream.avail_in = 0; // should be zero already anyway
    if (stream.avail_out == 0)
    {
	    deeper->write(outbuf, Z_BUFSIZE);
	    stream.next_out = outbuf;
	    stream.avail_out = Z_BUFSIZE;
    }
    for (;;)
    {
	int err = deflate(&stream, Z_FINISH);
	if (err < 0)
	    drop_dead(err);
	uInt len = Z_BUFSIZE - stream.avail_out;
	if (!len)
	    break;
	deeper->write(outbuf, len);
	stream.next_out = outbuf;
	stream.avail_out = Z_BUFSIZE;
    }

    //
    // and the trailer
    //
    output_long_le(deeper, crc);
    output_long_le(deeper, stream.total_in);

    //
    // Clean up any resources we were using.
    //
    if (stream.state != NULL)
	deflateEnd(&stream);
    delete [] outbuf;
    outbuf = 0;
}


output_gzip::output_gzip(const output::pointer &arg) :
    deeper(arg),
    outbuf(new Byte [Z_BUFSIZE]),
    crc(0),
    pos(0),
    bol(true)
{
    crc = crc32(0L, Z_NULL, 0);
    stream.avail_in = 0;
    stream.avail_out = 0;
    stream.next_in = NULL;
    stream.next_out = NULL;
    stream.opaque = (voidpf)0;
    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;

    //
    // Set the parameters for the compression.
    // Note: windowBits is passed < 0 to suppress zlib header.
    //
    int err =
	deflateInit2
	(
	    &stream,
	    Z_BEST_COMPRESSION, // level
	    Z_DEFLATED,	        // method
	    -MAX_WBITS,		// windowBits
	    DEF_MEM_LEVEL,      // memLevel
	    Z_DEFAULT_STRATEGY  // strategy
	);
    if (err != Z_OK)
	drop_dead(err);

    stream.next_out = outbuf;
    stream.avail_out = Z_BUFSIZE;

    //
    // Write a very simple .gz header:
    //
    deeper->fputc(gzip_magic[0]);
    deeper->fputc(gzip_magic[1]);
    deeper->fputc(Z_DEFLATED);
    deeper->fputc(0); // flags
    output_long_le(deeper, 0L); // time
    deeper->fputc(0); // xflags
    deeper->fputc(3); // always use unix OS_CODE
}


output::pointer
output_gzip::create(const output::pointer &a_deeper)
{
    return pointer(new output_gzip(a_deeper));
}


nstring
output_gzip::filename()
    const
{
    return deeper->filename();
}


long
output_gzip::ftell_inner()
    const
{
    return pos;
}


void
output_gzip::write_inner(const void *buf, size_t len)
{
    if (len > 0)
	bol = (((const char *)buf)[len - 1] == '\n');
    stream.next_in = (Bytef *)buf;
    stream.avail_in = len;
    while (stream.avail_in != 0)
    {
	if (stream.avail_out == 0)
	{
	    deeper->write(outbuf, Z_BUFSIZE);
	    stream.next_out = outbuf;
	    stream.avail_out = Z_BUFSIZE;
	}
	int err = deflate(&stream, Z_NO_FLUSH);
	if (err != Z_OK)
	    drop_dead(err);
    }
    crc = crc32(crc, (Bytef *)buf, len);
    pos += len;
}


int
output_gzip::page_width()
    const
{
    return deeper->page_width();
}


int
output_gzip::page_length()
    const
{
    return deeper->page_length();
}


void
output_gzip::end_of_line_inner()
{
    if (!bol)
	fputc('\n');
}


const char *
output_gzip::type_name()
    const
{
    return "gzip";
}

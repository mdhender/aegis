//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2001-2005 Peter Miller;
//	All rights reserved.
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
// MANIFEST: functions to manipulate gunzips
//

#include <ac/zlib.h>
#include <ac/string.h>

#include <input/gunzip.h>
#include <sub.h>


#ifndef Z_BUFSIZE
#  ifdef MAXSEG_64K
#    define Z_BUFSIZE 4096 // minimize memory usage for 16-bit DOS
#  else
#    define Z_BUFSIZE 16384
#  endif
#endif

// gzip flag byte
#define ASCII_FLAG   0x01 // bit 0 set: file probably ascii text
#define HEAD_CRC     0x02 // bit 1 set: header CRC present
#define EXTRA_FIELD  0x04 // bit 2 set: extra field present
#define ORIG_NAME    0x08 // bit 3 set: original file name present
#define COMMENT      0x10 // bit 4 set: file comment present
#define RESERVED     0xE0 // bits 5..7: reserved


void
input_gunzip::zlib_fatal_error(int err)
{
    if (err >= 0)
	return;
    sub_context_ty sc;
    if (stream.msg)
	sc.var_set_format("ERRNO", "%s (%s)", z_error(err), stream.msg);
    else
	sc.var_set_charstar("ERRNO", z_error(err));
    sc.var_override("ERRNO");
    sc.var_set_string("File_Name", deeper->name());
    sc.fatal_intl(i18n("gunzip $filename: $errno"));
}


input_gunzip::~input_gunzip()
{
    int err = inflateEnd(&stream);
    if (err < 0)
	zlib_fatal_error(err);
    if (close_on_close)
	delete deeper;
    deeper = 0;
    delete buf;
    buf = 0;
}


input_gunzip::input_gunzip(input_ty *arg1, bool arg2) :
    deeper(arg1),
    close_on_close(arg2),
    z_eof(false),
    crc(crc32(0L, Z_NULL, 0)),
    pos(0),
    buf(new Byte [Z_BUFSIZE])
{
    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;
    stream.opaque = (voidpf)0;
    stream.next_in = Z_NULL;
    stream.avail_in = 0;
    stream.next_out = Z_NULL;
    stream.avail_out = 0;

    //
    // windowBits is passed < 0 to tell that there is no zlib header.
    // Note that in this case inflate *requires* an extra "dummy" byte
    // after the compressed stream in order to complete decompression
    // and return Z_STREAM_END. Here the gzip CRC32 ensures that 4
    // bytes are present after the compressed stream.
    //
    int err = inflateInit2(&stream, -MAX_WBITS);
    if (err < 0)
	zlib_fatal_error(err);

    //
    // Now read the file header.
    //
    read_header();
}


long
input_gunzip::getLong()
{
    long result = 0;
    for (int j = 0; j < 4; ++j)
    {
	int c = deeper->getc();
	if (c < 0)
    	    fatal_error("gunzip: premature end of file");
	result += c << (j * 8);
    }
    return result;
}


long
input_gunzip::read_inner(void *data, size_t len)
{
    if (z_eof)
	return 0;

    Bytef *start = (Bytef *)data; // starting point for crc computation
    stream.next_out = (Bytef *)data;
    stream.avail_out = len;

    while (stream.avail_out > 0)
    {
	if (stream.avail_in == 0)
	{
	    stream.next_in = buf;
    	    stream.avail_in = deeper->read(buf, Z_BUFSIZE);
	    //
	    // There should always be something left on the
	    // input, because we have the CRC and Length
	    // to follow.  Fatal error if not.
	    //
	    if (stream.avail_in <= 0)
	    {
		deeper->fatal_error("gunzip: premature end of file");
	    }
	}
	int err = inflate(&stream, Z_PARTIAL_FLUSH);
	if (err < 0)
    	    zlib_fatal_error(err);
	if (err == Z_STREAM_END)
	{
    	    z_eof = true;

	    //
	    // Push back the unused portion of the input stream.
	    // (The way we wrote it, there shouldn't be much.)
	    //
	    while (stream.avail_in > 0)
	    {
		stream.avail_in--;
		deeper->ungetc(stream.next_in[stream.avail_in]);
	    }

	    //
	    // Fall out of the loop.
	    //
	    break;
	}
    }

    //
    // Calculate the running CRC
    //
    long result = stream.next_out - start;
    crc = crc32(crc, start, (uInt)result);

    //
    // Update the file position.
    //
    pos += result;

    //
    // At end-of-file we need to do some checking.
    //
    if (z_eof)
    {
	//
	// Check CRC
	//
	// Watch out for 64-bit machines.  This is what
	// those aparrently redundant 0xFFFFFFFF are for.
	//
	if ((getLong() & 0xFFFFFFFF) != (crc & 0xFFFFFFFF))
	    fatal_error("gunzip: checksum mismatch");

	//
	// The uncompressed length here may be different
	// from pos in case of concatenated .gz
	// files.  But we don't write them that way,
	// so give an error if it happens.
	//
	// We shouldn't have 64-bit problems in this case.
	//
	if (getLong() != pos)
	    fatal_error("gunzip: length mismatch");
    }

    //
    // Return success (failure always goes via input_format_error,
    // or zlib_fatal_error).
    //
    return result;
}


long
input_gunzip::ftell_inner()
{
    return pos;
}


nstring
input_gunzip::name()
{
    if (filename.empty())
    {
	nstring s = deeper->name();
	if (s.ends_with_nocase(".z"))
	    filename = nstring(s.c_str(), s.size() - 2);
	else if (s.ends_with_nocase(".gz"))
	    filename = nstring(s.c_str(), s.size() - 3);
	else if (s.ends_with_nocase(".tgz"))
	{
	    filename =
		nstring::format("%.*s.tar", (int)s.size() - 4, s.c_str());
	}
	else
	    filename = s;
    }
    return filename;
}


long
input_gunzip::length()
{
    //
    // We have no idea how long the decompressed stream will be.
    //
    return -1;
}


//
// Check the gzip header of a gz_stream opened for reading. Set the
// stream mode to transparent if the gzip magic header is not present;
// set err to Z_DATA_ERROR if the magic header is present but the
// rest of the header is incorrect.
//
// IN assertion: the stream this has already been created sucessfully;
// stream.avail_in is zero for the first time, but may be non-zero
// for concatenated .gz files.
//
static int gz_magic[2] = {0x1f, 0x8b}; // gzip magic header

bool
input_gunzip::candidate(input_ty *deeper)
{
    //
    // Check for the magic number.
    // If it isn't present, assume transparent mode.
    //
    int c = deeper->getc();
    if (c < 0)
	return false;
    if (c != gz_magic[0])
    {
	deeper->ungetc(c);
	return false;
    }
    c = deeper->getc();
    if (c < 0)
    {
	deeper->ungetc(gz_magic[0]);
	return false;
    }
    if (c != gz_magic[1])
    {
	deeper->ungetc(c);
	deeper->ungetc(gz_magic[0]);
	return false;
    }
    deeper->ungetc(gz_magic[1]);
    deeper->ungetc(gz_magic[0]);
    return true;
}


void
input_gunzip::read_header()
{
    //
    // Check for the magic number.
    // If it isn't present, assume transparent mode.
    //
    int c1 = deeper->getc();
    int c2 = deeper->getc();
    if (c1 != gz_magic[0] || c2 != gz_magic[1])
	deeper->fatal_error("gunzip: wrong magic number");

    //
    // Magic number present, now we require the rest of the header
    // to be present and correctly formed.
    //
    int method = deeper->getc();
    if (method != Z_DEFLATED)
	deeper->fatal_error("gunzip: not deflated encoding");
    int flags = deeper->getc();
    if (flags < 0 || (flags & RESERVED) != 0)
	deeper->fatal_error("gunzip: unknown flags");

    // Discard time, xflags and OS code:
    for (uInt len = 0; len < 6; len++)
	if (deeper->getc() < 0)
    	    deeper->fatal_error("gunzip: short file");

    if (flags & EXTRA_FIELD)
    {
	// skip the extra field
	int elen = deeper->getc();
	if (elen < 0)
	    deeper->fatal_error("gunzip: invalid character value");
	int c = deeper->getc();
	if (c < 0)
	    deeper->fatal_error("gunzip: short file");
	elen += (c << 8);
	while (elen-- > 0)
	{
	    if (deeper->getc() < 0)
		deeper->fatal_error("gunzip: short file");
	}
    }
    if (flags & ORIG_NAME)
    {
	// skip the original file name
	for (;;)
	{
    	    int c = deeper->getc();
    	    if (c < 0)
       		deeper->fatal_error("gunzip: short file");
    	    if (c == 0)
       		break;
	}
    }
    if (flags & COMMENT)
    {
	// skip the .gz file comment
	for (;;)
	{
    	    int c = deeper->getc();
    	    if (c < 0)
       		deeper->fatal_error("gunzip: short file");
    	    if (c == 0)
       		break;
	}
    }
    if (flags & HEAD_CRC)
    {
	// skip the header crc
	for (int len = 0; len < 2; len++)
    	    if (deeper->getc() < 0)
       		deeper->fatal_error("gunzip: short file");
    }
}


void
input_gunzip::keepalive()
{
    deeper->keepalive();
}


input_ty *
input_gunzip_open(input_ty *deeper)
{
    if (!input_gunzip::candidate(deeper))
    {
	//
	// If it is not actually a compressed file,
	// simply return the deeper file.  This will
	// give much better performance.
	//
	return deeper;
    }
    return new input_gunzip(deeper);
}


bool
input_gunzip::is_remote()
    const
{
    return deeper->is_remote();
}

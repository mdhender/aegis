//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2001-2004 Peter Miller;
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
#include <input/private.h>
#include <mem.h>
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


typedef struct input_gunzip_ty input_gunzip_ty;
struct input_gunzip_ty
{
    input_ty	    inherited;
    input_ty	    *deeper;
    z_stream	    stream;
    int		    z_eof;
    uLong	    crc;
    long	    pos;
    Byte	    *buf;
    string_ty	    *filename;
};


static void
zlib_fatal_error(input_gunzip_ty *this_thing, int err)
{
    sub_context_ty  *scp;

    if (err >= 0)
	    return;
    scp = sub_context_new();
    if (this_thing->stream.msg)
    {
	sub_var_set_format
	(
	    scp,
	    "ERRNO",
	    "%s (%s)",
	    z_error(err),
	    this_thing->stream.msg
	);
    }
    else
	sub_var_set_charstar(scp, "ERRNO", z_error(err));
    sub_var_override(scp, "ERRNO");
    sub_var_set_string(scp, "File_Name", input_name(this_thing->deeper));
    fatal_intl(scp, i18n("gunzip $filename: $errno"));
}


static void
input_gunzip_destructor(input_ty *fp)
{
    input_gunzip_ty *this_thing;
    int		    err;

    this_thing = (input_gunzip_ty *)fp;
    err = inflateEnd(&this_thing->stream);
    if (err < 0)
	zlib_fatal_error(this_thing, err);
    input_delete(this_thing->deeper);
    mem_free(this_thing->buf);
    this_thing->deeper = 0;
    if (this_thing->filename)
	str_free(this_thing->filename);
}


static long
getLong(input_gunzip_ty *this_thing)
{
    long	    result;
    int		    j;
    int		    c;

    result = 0;
    for (j = 0; j < 4; ++j)
    {
	c = input_getc(this_thing->deeper);
	if (c < 0)
	{
    	    input_fatal_error
	    (
		(input_ty *)this_thing,
		"gunzip: premature end of file"
	    );
	}
	result += c << (j * 8);
    }
    return result;
}


static long
input_gunzip_read(input_ty *fp, void *data, size_t len)
{
    input_gunzip_ty *this_thing;
    Bytef	    *start;
    int		    err;
    long	    result;

    this_thing = (input_gunzip_ty *)fp;
    if (this_thing->z_eof)
	return 0;

    start = (Bytef *)data; // starting point for crc computation
    this_thing->stream.next_out = (Bytef *)data;
    this_thing->stream.avail_out = len;

    while (this_thing->stream.avail_out > 0)
    {
	if (this_thing->stream.avail_in == 0)
	{
	    this_thing->stream.next_in = this_thing->buf;
    	    this_thing->stream.avail_in =
       		input_read(this_thing->deeper, this_thing->buf, Z_BUFSIZE);
	    //
	    // There should always be something left on the
	    // input, because we have the CRC and Length
	    // to follow.  Fatal error if not.
	    //
	    if (this_thing->stream.avail_in <= 0)
	    {
		input_fatal_error
		(
		    this_thing->deeper,
		    "gunzip: premature end of file"
		);
	    }
	}
	err = inflate(&this_thing->stream, Z_PARTIAL_FLUSH);
	if (err < 0)
    	    zlib_fatal_error(this_thing, err);
	if (err == Z_STREAM_END)
	{
    	    this_thing->z_eof = 1;

	    //
	    // Push back the unused portion of the input stream.
	    // (The way we wrote it, there shouldn't be much.)
	    //
	    while (this_thing->stream.avail_in > 0)
	    {
		this_thing->stream.avail_in--;
		input_ungetc
		(
	    	    this_thing->deeper,
	    	    this_thing->stream.next_in[this_thing->stream.avail_in]
		);
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
    result = this_thing->stream.next_out - start;
    this_thing->crc = crc32(this_thing->crc, start, (uInt)result);

    //
    // Update the file position.
    //
    this_thing->pos += result;

    //
    // At end-of-file we need to do some checking.
    //
    if (this_thing->z_eof)
    {
	//
	// Check CRC
	//
	// Watch out for 64-bit machines.  This is what
	// those aparrently redundant 0xFFFFFFFF are for.
	//
	if ((getLong(this_thing) & 0xFFFFFFFF) !=
            (this_thing->crc & 0xFFFFFFFF))
	    input_fatal_error((input_ty *)this_thing,
                              "gunzip: checksum mismatch");

	//
	// The uncompressed length here may be different
	// from this_thing->pos in case of concatenated .gz
	// files.  But we don't write them that way,
	// so give an error if it happens.
	//
	// We shouldn't have 64-bit problems in this case.
	//
	if (getLong(this_thing) != this_thing->pos)
	    input_fatal_error((input_ty *)this_thing,
                              "gunzip: length mismatch");
    }

    //
    // Return success (failure always goes via input_format_error,
    // or zlib_fatal_error).
    //
    return result;
}


static long
input_gunzip_ftell(input_ty *fp)
{
    input_gunzip_ty *this_thing;

    this_thing = (input_gunzip_ty *)fp;
    return this_thing->pos;
}


static int
end_with(string_ty *haystack, const char *needle)
{
    size_t	    len;
    char	    *s;

    len = strlen(needle);
    if (haystack->str_length < len)
	return 0;
    s = haystack->str_text + haystack->str_length - len;
    return (0 == strncasecmp(s, needle, len));
}



static string_ty *
input_gunzip_name(input_ty *fp)
{
    input_gunzip_ty *this_thing;

    this_thing = (input_gunzip_ty *)fp;
    if (!this_thing->filename)
    {
	string_ty	*s;

	s = input_name(this_thing->deeper);
	if (end_with(s, ".z"))
	    this_thing->filename = str_n_from_c(s->str_text, s->str_length - 2);
	else if (end_with(s, ".gz"))
	    this_thing->filename = str_n_from_c(s->str_text, s->str_length - 3);
	else if (end_with(s, ".tgz"))
	{
	    this_thing->filename =
		str_format("%.*s.tar", s->str_length - 4, s->str_text);
	}
	else
	    this_thing->filename = str_copy(s);
    }
    return this_thing->filename;
}


static long
input_gunzip_length(input_ty *fp)
{
    //
    // We have no idea how long the decompressed stream will be.
    //
    return -1;
}


//
// Check the gzip header of a gz_stream opened for reading. Set the
// stream mode to transparent if the gzip magic header is not present;
// set this_thing->err to Z_DATA_ERROR if the magic header is present but the
// rest of the header is incorrect.
//
// IN assertion: the stream this has already been created sucessfully;
// this_thing->stream.avail_in is zero for the first time, but may be non-zero
// for concatenated .gz files.
//

static int
check_header(input_ty *deeper)
{
    int		    method;
    int		    flags;
    uInt	    len;
    int		    c;
    static int	    gz_magic[2] = {0x1f, 0x8b}; // gzip magic header

    //
    // Check for the magic number.
    // If it isn't present, assume transparent mode.
    //
    c = input_getc(deeper);
    if (c < 0)
	return 0;
    if (c != gz_magic[0])
    {
	input_ungetc(deeper, c);
	return 0;
    }
    c = input_getc(deeper);
    if (c < 0)
    {
	input_ungetc(deeper, gz_magic[0]);
	return 0;
    }
    if (c != gz_magic[1])
    {
	input_ungetc(deeper, c);
	input_ungetc(deeper, gz_magic[0]);
	return 0;
    }

    //
    // Magic number present, now we require the rest of the header
    // to be present and correctly formed.
    //
    method = input_getc(deeper);
    if (method != Z_DEFLATED)
	input_fatal_error(deeper, "gunzip: not deflated encoding");
    flags = input_getc(deeper);
    if (flags < 0 || (flags & RESERVED) != 0)
	input_fatal_error(deeper, "gunzip: unknown flags");

    // Discard time, xflags and OS code:
    for (len = 0; len < 6; len++)
	if (input_getc(deeper) < 0)
    	    input_fatal_error(deeper, "gunzip: short file");

    if (flags & EXTRA_FIELD)
    {
	// skip the extra field
	len = input_getc(deeper);
	if (len < 0)
	    input_fatal_error(deeper, "gunzip: invalid character value");
	c = input_getc(deeper);
	if (c < 0)
	    input_fatal_error(deeper, "gunzip: short file");
	len += (c << 8);
	while (len-- > 0)
	{
	    if (input_getc(deeper) < 0)
		input_fatal_error(deeper, "gunzip: short file");
	}
    }
    if (flags & ORIG_NAME)
    {
	// skip the original file name
	for (;;)
	{
    	    c = input_getc(deeper);
    	    if (c < 0)
       		input_fatal_error(deeper, "gunzip: short file");
    	    if (c == 0)
       		break;
	}
    }
    if (flags & COMMENT)
    {
	// skip the .gz file comment
	for (;;)
	{
    	    c = input_getc(deeper);
    	    if (c < 0)
       		input_fatal_error(deeper, "gunzip: short file");
    	    if (c == 0)
       		break;
	}
    }
    if (flags & HEAD_CRC)
    {
	// skip the header crc
	for (len = 0; len < 2; len++)
    	    if (input_getc(deeper) < 0)
       		input_fatal_error(deeper, "gunzip: short file");
    }
    return 1;
}


static void
input_gunzip_keepalive(input_ty *fp)
{
    input_gunzip_ty *ip;

    ip = (input_gunzip_ty *)fp;
    input_keepalive(ip->deeper);
}


static input_vtbl_ty vtbl =
{
    sizeof(input_gunzip_ty),
    input_gunzip_destructor,
    input_gunzip_read,
    input_gunzip_ftell,
    input_gunzip_name,
    input_gunzip_length,
    input_gunzip_keepalive,
};


input_ty *
input_gunzip(input_ty *deeper)
{
    input_ty	    *result;
    input_gunzip_ty *this_thing;
    int		    err;

    //
    // Verify and skip the .gz file header.
    //
    if (!check_header(deeper))
    {
	//
	// If it is not actually a compressed file,
	// simply return the deeper file.  This will
	// give much better performance.
	//
	return deeper;
    }

    result = input_new(&vtbl);
    this_thing = (input_gunzip_ty *)result;
    this_thing->deeper = deeper;
    this_thing->stream.zalloc = (alloc_func)0;
    this_thing->stream.zfree = (free_func)0;
    this_thing->stream.opaque = (voidpf)0;
    this_thing->stream.next_in = Z_NULL;
    this_thing->stream.avail_in = 0;
    this_thing->stream.next_out = Z_NULL;
    this_thing->stream.avail_out = 0;
    this_thing->buf = (Byte *)mem_alloc(Z_BUFSIZE);
    this_thing->crc = crc32(0L, Z_NULL, 0);
    this_thing->pos = 0;
    this_thing->z_eof = 0;
    this_thing->filename = 0;

    //
    // windowBits is passed < 0 to tell that there is no zlib header.
    // Note that in this case inflate *requires* an extra "dummy" byte
    // after the compressed stream in order to complete decompression
    // and return Z_STREAM_END. Here the gzip CRC32 ensures that 4
    // bytes are present after the compressed stream.
    //
    err = inflateInit2(&this_thing->stream, -MAX_WBITS);
    if (err < 0)
	zlib_fatal_error(this_thing, err);

    return result;
}

/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2001, 2002 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate gunzips
 */

#include <ac/zlib.h>
#include <ac/string.h>

#include <input/gunzip.h>
#include <input/private.h>
#include <mem.h>
#include <sub.h>


#ifndef Z_BUFSIZE
#  ifdef MAXSEG_64K
#    define Z_BUFSIZE 4096 /* minimize memory usage for 16-bit DOS */
#  else
#    define Z_BUFSIZE 16384
#  endif
#endif

/* gzip flag byte */
#define ASCII_FLAG   0x01 /* bit 0 set: file probably ascii text */
#define HEAD_CRC     0x02 /* bit 1 set: header CRC present */
#define EXTRA_FIELD  0x04 /* bit 2 set: extra field present */
#define ORIG_NAME    0x08 /* bit 3 set: original file name present */
#define COMMENT      0x10 /* bit 4 set: file comment present */
#define RESERVED     0xE0 /* bits 5..7: reserved */


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


static void zlib_fatal_error _((input_gunzip_ty	*, int));

static void
zlib_fatal_error(this, err)
    input_gunzip_ty *this;
    int		    err;
{
    sub_context_ty  *scp;

    if (err >= 0)
	    return;
    scp = sub_context_new();
    if (this->stream.msg)
    {
	sub_var_set_format
	(
	    scp,
	    "ERRNO",
	    "%s (%s)",
	    z_error(err),
	    this->stream.msg
	);
    }
    else
	sub_var_set_charstar(scp, "ERRNO", z_error(err));
    sub_var_override(scp, "ERRNO");
    sub_var_set_string(scp, "File_Name", input_name(this->deeper));
    fatal_intl(scp, i18n("gunzip $filename: $errno"));
}


static void input_gunzip_destructor _((input_ty *));

static void
input_gunzip_destructor(fp)
    input_ty	    *fp;
{
    input_gunzip_ty *this;
    int		    err;

    this = (input_gunzip_ty *)fp;
    err = inflateEnd(&this->stream);
    if (err < 0)
	zlib_fatal_error(this, err);
    input_delete(this->deeper);
    mem_free(this->buf);
    this->deeper = 0;
    if (this->filename)
	str_free(this->filename);
}


static long getLong _((input_gunzip_ty *));

static long
getLong(this)
    input_gunzip_ty *this;
{
    long	    result;
    int		    j;
    int		    c;

    result = 0;
    for (j = 0; j < 4; ++j)
    {
	c = input_getc(this->deeper);
	if (c < 0)
	{
    	    input_fatal_error
	    (
		(input_ty *)this,
		"gunzip: premature end of file"
	    );
	}
	result += c << (j * 8);
    }
    return result;
}


static long input_gunzip_read _((input_ty *, void *, size_t));

static long
input_gunzip_read(fp, data, len)
    input_ty	    *fp;
    void	    *data;
    size_t	    len;
{
    input_gunzip_ty *this;
    Bytef	    *start;
    int		    err;
    long	    result;

    this = (input_gunzip_ty *)fp;
    if (this->z_eof)
	return 0;

    start = data; /* starting point for crc computation */
    this->stream.next_out = data;
    this->stream.avail_out = len;

    while (this->stream.avail_out > 0)
    {
	if (this->stream.avail_in == 0)
	{
	    this->stream.next_in = this->buf;
    	    this->stream.avail_in =
       		input_read(this->deeper, this->buf, Z_BUFSIZE);
	    /*
	     * There should always be something left on the
	     * input, because we have the CRC and Length
	     * to follow.  Fatal error if not.
	     */
	    if (this->stream.avail_in <= 0)
	    {
		input_fatal_error
		(
		    this->deeper,
		    "gunzip: premature end of file"
		);
	    }
	}
	err = inflate(&this->stream, Z_PARTIAL_FLUSH);
	if (err < 0)
    	    zlib_fatal_error(this, err);
	if (err == Z_STREAM_END)
	{
    	    this->z_eof = 1;

	    /*
	     * Push back the unused portion of the input stream.
	     * (The way we wrote it, there shouldn't be much.)
	     */
	    while (this->stream.avail_in > 0)
	    {
		this->stream.avail_in--;
		input_ungetc
		(
	    	    this->deeper,
	    	    this->stream.next_in[this->stream.avail_in]
		);
	    }

	    /*
	     * Fall out of the loop.
	     */
	    break;
	}
    }

    /*
     * Calculate the running CRC
     */
    result = this->stream.next_out - start;
    this->crc = crc32(this->crc, start, (uInt)result);

    /*
     * Update the file position.
     */
    this->pos += result;

    /*
     * At end-of-file we need to do some checking.
     */
    if (this->z_eof)
    {
	/*
	 * Check CRC
	 *
	 * Watch out for 64-bit machines.  This is what
	 * those aparrently redundant 0xFFFFFFFF are for.
	 */
	if ((getLong(this) & 0xFFFFFFFF) != (this->crc & 0xFFFFFFFF))
	    input_fatal_error((input_ty *)this, "gunzip: checksum mismatch");

	/*
	 * The uncompressed length here may be different
	 * from this->pos in case of concatenated .gz
	 * files.  But we don't write them that way,
	 * so give an error if it happens.
	 *
	 * We shouldn't have 64-bit problems in this case.
	 */
	if (getLong(this) != this->pos)
	    input_fatal_error((input_ty *)this, "gunzip: length mismatch");
    }

    /*
     * Return success (failure always goes via input_format_error,
     * or zlib_fatal_error).
     */
    return result;
}


static long input_gunzip_ftell _((input_ty *));

static long
input_gunzip_ftell(fp)
    input_ty	    *fp;
{
    input_gunzip_ty *this;

    this = (input_gunzip_ty *)fp;
    return this->pos;
}


static int end_with _((string_ty *, const char *));

static int
end_with(haystack, needle)
    string_ty	    *haystack;
    const char	    *needle;
{
    size_t	    len;
    char	    *s;

    len = strlen(needle);
    if (haystack->str_length < len)
	return 0;
    s = haystack->str_text + haystack->str_length - len;
    return (0 == strncasecmp(s, needle, len));
}



static string_ty *input_gunzip_name _((input_ty *));

static string_ty *
input_gunzip_name(fp)
    input_ty	    *fp;
{
    input_gunzip_ty *this;

    this = (input_gunzip_ty *)fp;
    if (!this->filename)
    {
	string_ty	*s;

	s = input_name(this->deeper);
	if (end_with(s, ".z"))
	    this->filename = str_n_from_c(s->str_text, s->str_length - 2);
	else if (end_with(s, ".gz"))
	    this->filename = str_n_from_c(s->str_text, s->str_length - 3);
	else if (end_with(s, ".tgz"))
	{
	    this->filename =
		str_format("%.*s.tar", s->str_length - 4, s->str_text);
	}
	else
	    this->filename = str_copy(s);
    }
    return this->filename;
}


static long input_gunzip_length _((input_ty *));

static long
input_gunzip_length(fp)
    input_ty	    *fp;
{
    /*
     * We have no idea how long the decompressed stream will be.
     */
    return -1;
}


/*
 * Check the gzip header of a gz_stream opened for reading. Set the
 * stream mode to transparent if the gzip magic header is not present;
 * set this->err to Z_DATA_ERROR if the magic header is present but the
 * rest of the header is incorrect.
 *
 * IN assertion: the stream this has already been created sucessfully;
 * this->stream.avail_in is zero for the first time, but may be non-zero
 * for concatenated .gz files.
 */

static int check_header _((input_ty *));

static int
check_header(deeper)
    input_ty	    *deeper;
{
    int		    method;
    int		    flags;
    uInt	    len;
    int		    c;
    static int	    gz_magic[2] = {0x1f, 0x8b}; /* gzip magic header */

    /*
     * Check for the magic number.
     * If it isn't present, assume transparent mode.
     */
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

    /*
     * Magic number present, now we require the rest of the header
     * to be present and correctly formed.
     */
    method = input_getc(deeper);
    if (method != Z_DEFLATED)
	input_fatal_error(deeper, "gunzip: not deflated encoding");
    flags = input_getc(deeper);
    if (flags < 0 || (flags & RESERVED) != 0)
	input_fatal_error(deeper, "gunzip: unknown flags");

    /* Discard time, xflags and OS code: */
    for (len = 0; len < 6; len++)
	if (input_getc(deeper) < 0)
    	    input_fatal_error(deeper, "gunzip: short file");

    if (flags & EXTRA_FIELD)
    {
	/* skip the extra field */
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
	/* skip the original file name */
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
	/* skip the .gz file comment */
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
	/* skip the header crc */
	for (len = 0; len < 2; len++)
    	    if (input_getc(deeper) < 0)
       		input_fatal_error(deeper, "gunzip: short file");
    }
    return 1;
}


static input_vtbl_ty vtbl =
{
    sizeof(input_gunzip_ty),
    input_gunzip_destructor,
    input_gunzip_read,
    input_gunzip_ftell,
    input_gunzip_name,
    input_gunzip_length,
};


input_ty *
input_gunzip(deeper)
    input_ty	    *deeper;
{
    input_ty	    *result;
    input_gunzip_ty *this;
    int		    err;

    /*
     * Verify and skip the .gz file header.
     */
    if (!check_header(deeper))
    {
	/*
	 * If it is not actually a compressed file,
	 * simply return the deeper file.  This will
	 * give much better performance.
	 */
	return deeper;
    }

    result = input_new(&vtbl);
    this = (input_gunzip_ty *)result;
    this->deeper = deeper;
    this->stream.zalloc = (alloc_func)0;
    this->stream.zfree = (free_func)0;
    this->stream.opaque = (voidpf)0;
    this->stream.next_in = Z_NULL;
    this->stream.avail_in = 0;
    this->stream.next_out = Z_NULL;
    this->stream.avail_out = 0;
    this->buf = mem_alloc(Z_BUFSIZE);
    this->crc = crc32(0L, Z_NULL, 0);
    this->pos = 0;
    this->z_eof = 0;
    this->filename = 0;

    /*
     * windowBits is passed < 0 to tell that there is no zlib header.
     * Note that in this case inflate *requires* an extra "dummy" byte
     * after the compressed stream in order to complete decompression
     * and return Z_STREAM_END. Here the gzip CRC32 ensures that 4
     * bytes are present after the compressed stream.
     */
    err = inflateInit2(&this->stream, -MAX_WBITS);
    if (err < 0)
	zlib_fatal_error(this, err);

    return result;
}

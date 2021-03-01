/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999 Peter Miller;
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
 * MANIFEST: functions to gzip output streams
 */

#include <ac/zlib.h>

#include <mem.h>
#include <output/gzip.h>
#include <output/private.h>
#include <sub.h>


#ifndef Z_BUFSIZE
#ifdef MAXSEG_64K
#define Z_BUFSIZE 4096 /* minimize memory usage for 16-bit DOS */
#else
#define Z_BUFSIZE 16384
#endif
#endif

static int gzip_magic[2] = {0x1f, 0x8b}; /* gzip magic header */


typedef struct output_gzip_ty output_gzip_ty;
struct output_gzip_ty
{
	output_ty	inherited;
	output_ty	*deeper;
	z_stream	stream;
	Byte		*outbuf;	/* output buffer */
	uLong		crc;		/* crc32 of uncompressed data */
	long		pos;
};


static void drop_dead _((output_gzip_ty *, int));

static void
drop_dead(this, err)
	output_gzip_ty	*this;
	int		err;
{
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set(scp, "ERRNO", "%s", z_error(err)); 
	sub_var_override(scp, "ERRNO");
	sub_var_set(scp, "File_Name", "%s", output_filename(this->deeper));
	fatal_intl(scp,  i18n("gzip $filename: $errno"));
}


/*
 * Outputs a long in LSB order to the given file
 *	(little endian)
 */

static void output_long_le _((output_ty *, uLong));

static void
output_long_le(fp, x)
	output_ty	*fp;
	uLong		x;
{
	int		n;

	for (n = 0; n < 4; n++)
	{
		output_fputc(fp, (int)(x & 0xff));
		x >>= 8;
	}
}


static void destructor _((output_ty *));

static void
destructor(fp)
	output_ty	*fp;
{
	output_gzip_ty	*this;
	int		err;
	uInt		len;

	/*
	 * finish sending the compressed stream
	 */
	this = (output_gzip_ty *)fp;
	this->stream.avail_in = 0; /* should be zero already anyway */
	if (this->stream.avail_out == 0)
	{
		output_write(this->deeper, this->outbuf, Z_BUFSIZE);
		this->stream.next_out = this->outbuf;
		this->stream.avail_out = Z_BUFSIZE;
	}
	for (;;)
	{
		err = deflate(&this->stream, Z_FINISH);
		if (err < 0)
			drop_dead(this, err);
		len = Z_BUFSIZE - this->stream.avail_out;
		if (!len)
			break;
		output_write(this->deeper, this->outbuf, len);
		this->stream.next_out = this->outbuf;
		this->stream.avail_out = Z_BUFSIZE;
	}

	/*
	 * and the trailer
	 */
        output_long_le(this->deeper, this->crc);
        output_long_le(this->deeper, this->stream.total_in);

	/*
	 * Clean up any resources we were using.
	 */
	if (this->stream.state != NULL)
		deflateEnd(&this->stream);
	mem_free(this->outbuf);

	/*
	 * Finish the deeper stream.
	 */
	output_delete(this->deeper);
}


static const char *filename _((output_ty *));

static const char *
filename(fp)
	output_ty	*fp;
{
	output_gzip_ty	*this;

	this = (output_gzip_ty *)fp;
	return output_filename(this->deeper);
}


static long otell _((output_ty *));

static long
otell(fp)
	output_ty	*fp;
{
	output_gzip_ty	*this;

	this = (output_gzip_ty *)fp;
	return this->pos;
}


static void owrite _((output_ty *, const void *, size_t));

static void
owrite(fp, buf, len)
	output_ty	*fp;
	const void	*buf;
	size_t		len;
{
	output_gzip_ty	*this;
	int		err;

	this = (output_gzip_ty *)fp;
	this->stream.next_in = (Bytef *)buf;
	this->stream.avail_in = len;
	while (this->stream.avail_in != 0)
	{
		if (this->stream.avail_out == 0)
		{
			output_write(this->deeper, this->outbuf, Z_BUFSIZE);
			this->stream.next_out = this->outbuf;
			this->stream.avail_out = Z_BUFSIZE;
		}
		err = deflate(&this->stream, Z_NO_FLUSH);
		if (err != Z_OK)
			drop_dead(this, err);
	}
	this->crc = crc32(this->crc, (Bytef *)buf, len);
	this->pos += len;
}


static void oputc _((output_ty *, int));

static void
oputc(fp, c)
	output_ty	*fp;
	int		c;
{
	char		buf[1];

	buf[0] = c;
	owrite(fp, buf, (size_t)1);
}


static output_vtbl_ty vtbl =
{
	sizeof(output_gzip_ty),
	"gzip",
	destructor,
	filename,
	otell,
	oputc,
	output_generic_fputs,
	owrite,
};


output_ty *
output_gzip(deeper)
	output_ty	*deeper;
{
	output_ty	*result;
	output_gzip_ty	*this;
	int		err;

	result = output_new(&vtbl);
	this = (output_gzip_ty *)result;
	this->deeper = deeper;
	this->pos = 0;

	this->crc = crc32(0L, Z_NULL, 0);
	this->outbuf = Z_NULL;
	this->stream.avail_in = 0;
	this->stream.avail_out = 0;
	this->stream.next_in = NULL;
	this->stream.next_out = NULL;
	this->stream.opaque = (voidpf)0;
	this->stream.zalloc = (alloc_func)0;
	this->stream.zfree = (free_func)0;

	/*
	 * Set the parameters for the compression.
	 * Note: windowBits is passed < 0 to suppress zlib header.
	 */
	err =
		deflateInit2
		(
			&this->stream,
			Z_BEST_COMPRESSION,	/* level */
			Z_DEFLATED,		/* method */
			-MAX_WBITS,		/* windowBits */
			DEF_MEM_LEVEL,		/* memLevel */
			Z_DEFAULT_STRATEGY	/* strategy */
		);
	if (err != Z_OK)
		drop_dead(this, err);

	this->outbuf = mem_alloc(Z_BUFSIZE);
	this->stream.next_out = this->outbuf;
	this->stream.avail_out = Z_BUFSIZE;

	/*
	 * Write a very simple .gz header:
	 */
	output_fputc(deeper, gzip_magic[0]);
	output_fputc(deeper, gzip_magic[1]);
	output_fputc(deeper, Z_DEFLATED);
	output_fputc(deeper, 0); /* flags */
	output_long_le(deeper, (long)0); /* time */
	output_fputc(deeper, 0); /* xflags */
	output_fputc(deeper, 3); /* always use unix OS_CODE */

	return result;
}

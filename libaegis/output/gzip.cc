//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002-2004 Peter Miller;
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
// MANIFEST: functions to gzip output streams
//

#include <ac/zlib.h>

#include <mem.h>
#include <output/gzip.h>
#include <output/private.h>
#include <sub.h>


#ifndef Z_BUFSIZE
#ifdef MAXSEG_64K
#define Z_BUFSIZE 4096 // minimize memory usage for 16-bit DOS
#else
#define Z_BUFSIZE 16384
#endif
#endif

static int gzip_magic[2] = {0x1f, 0x8b}; // gzip magic header


typedef struct output_gzip_ty output_gzip_ty;
struct output_gzip_ty
{
	output_ty	inherited;
	output_ty	*deeper;
	z_stream	stream;
	Byte		*outbuf;	// output buffer
	uLong		crc;		// crc32 of uncompressed data
	long		pos;
	int		bol;
};


static void
drop_dead(output_gzip_ty *this_thing, int err)
{
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_charstar(scp, "ERRNO", z_error(err));
	sub_var_override(scp, "ERRNO");
	sub_var_set_string(scp,
                           "File_Name",
                           output_filename(this_thing->deeper));
	fatal_intl(scp,  i18n("gzip $filename: $errno"));
}


//
// Outputs a long in LSB order to the given file
//	(little endian)
//

static void
output_long_le(output_ty *fp, uLong x)
{
	int		n;

	for (n = 0; n < 4; n++)
	{
		output_fputc(fp, (int)(x & 0xff));
		x >>= 8;
	}
}


static void
output_gzip_destructor(output_ty *fp)
{
	output_gzip_ty	*this_thing;
	int		err;
	uInt		len;

	//
	// finish sending the compressed stream
	//
	this_thing = (output_gzip_ty *)fp;
	this_thing->stream.avail_in = 0; // should be zero already anyway
	if (this_thing->stream.avail_out == 0)
	{
		output_write(this_thing->deeper, this_thing->outbuf, Z_BUFSIZE);
		this_thing->stream.next_out = this_thing->outbuf;
		this_thing->stream.avail_out = Z_BUFSIZE;
	}
	for (;;)
	{
		err = deflate(&this_thing->stream, Z_FINISH);
		if (err < 0)
			drop_dead(this_thing, err);
		len = Z_BUFSIZE - this_thing->stream.avail_out;
		if (!len)
			break;
		output_write(this_thing->deeper, this_thing->outbuf, len);
		this_thing->stream.next_out = this_thing->outbuf;
		this_thing->stream.avail_out = Z_BUFSIZE;
	}

	//
	// and the trailer
	//
        output_long_le(this_thing->deeper, this_thing->crc);
        output_long_le(this_thing->deeper, this_thing->stream.total_in);

	//
	// Clean up any resources we were using.
	//
	if (this_thing->stream.state != NULL)
		deflateEnd(&this_thing->stream);
	mem_free(this_thing->outbuf);

	//
	// Finish the deeper stream.
	//
	output_delete(this_thing->deeper);
}


static string_ty *
output_gzip_filename(output_ty *fp)
{
	output_gzip_ty	*this_thing;

	this_thing = (output_gzip_ty *)fp;
	return output_filename(this_thing->deeper);
}


static long
output_gzip_ftell(output_ty *fp)
{
	output_gzip_ty	*this_thing;

	this_thing = (output_gzip_ty *)fp;
	return this_thing->pos;
}


static void
output_gzip_write(output_ty *fp, const void *buf, size_t len)
{
	output_gzip_ty	*this_thing;
	int		err;

	this_thing = (output_gzip_ty *)fp;
	if (len > 0)
		this_thing->bol = (((const char *)buf)[len - 1] == '\n');
	this_thing->stream.next_in = (Bytef *)buf;
	this_thing->stream.avail_in = len;
	while (this_thing->stream.avail_in != 0)
	{
		if (this_thing->stream.avail_out == 0)
		{
			output_write(this_thing->deeper,
                                     this_thing->outbuf,
                                     Z_BUFSIZE);
			this_thing->stream.next_out = this_thing->outbuf;
			this_thing->stream.avail_out = Z_BUFSIZE;
		}
		err = deflate(&this_thing->stream, Z_NO_FLUSH);
		if (err != Z_OK)
			drop_dead(this_thing, err);
	}
	this_thing->crc = crc32(this_thing->crc, (Bytef *)buf, len);
	this_thing->pos += len;
}


static int
output_gzip_page_width(output_ty *fp)
{
	output_gzip_ty *this_thing;

	this_thing = (output_gzip_ty *)fp;
	return output_page_width(this_thing->deeper);
}


static int
output_gzip_page_length(output_ty *fp)
{
	output_gzip_ty *this_thing;

	this_thing = (output_gzip_ty *)fp;
	return output_page_length(this_thing->deeper);
}


static void
output_gzip_eoln(output_ty *fp)
{
	output_gzip_ty *this_thing;

	this_thing = (output_gzip_ty *)fp;
	if (!this_thing->bol)
		output_fputc(fp, '\n');
}


static output_vtbl_ty vtbl =
{
	sizeof(output_gzip_ty),
	output_gzip_destructor,
	output_gzip_filename,
	output_gzip_ftell,
	output_gzip_write,
	output_generic_flush, // don't actually do anything
	output_gzip_page_width,
	output_gzip_page_length,
	output_gzip_eoln,
	"gzip",
};


output_ty *
output_gzip(output_ty *deeper)
{
	output_ty	*result;
	output_gzip_ty	*this_thing;
	int		err;

	result = output_new(&vtbl);
	this_thing = (output_gzip_ty *)result;
	this_thing->deeper = deeper;
	this_thing->pos = 0;

	this_thing->crc = crc32(0L, Z_NULL, 0);
	this_thing->outbuf = Z_NULL;
	this_thing->stream.avail_in = 0;
	this_thing->stream.avail_out = 0;
	this_thing->stream.next_in = NULL;
	this_thing->stream.next_out = NULL;
	this_thing->stream.opaque = (voidpf)0;
	this_thing->stream.zalloc = (alloc_func)0;
	this_thing->stream.zfree = (free_func)0;

	//
	// Set the parameters for the compression.
	// Note: windowBits is passed < 0 to suppress zlib header.
	//
	err =
		deflateInit2
		(
			&this_thing->stream,
			Z_BEST_COMPRESSION,	// level
			Z_DEFLATED,		// method
			-MAX_WBITS,		// windowBits
			DEF_MEM_LEVEL,		// memLevel
			Z_DEFAULT_STRATEGY	// strategy
		);
	if (err != Z_OK)
		drop_dead(this_thing, err);

	this_thing->outbuf = (Byte *)mem_alloc(Z_BUFSIZE);
	this_thing->stream.next_out = this_thing->outbuf;
	this_thing->stream.avail_out = Z_BUFSIZE;
	this_thing->bol = 1;

	//
	// Write a very simple .gz header:
	//
	output_fputc(deeper, gzip_magic[0]);
	output_fputc(deeper, gzip_magic[1]);
	output_fputc(deeper, Z_DEFLATED);
	output_fputc(deeper, 0); // flags
	output_long_le(deeper, (long)0); // time
	output_fputc(deeper, 0); // xflags
	output_fputc(deeper, 3); // always use unix OS_CODE

	return result;
}

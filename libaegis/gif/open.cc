//
//	aegis - project change supervisor
//	Copyright (C) 1998, 1999, 2002-2008 Peter Miller
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

#include <common/ac/errno.h>
#include <common/ac/stdio.h>
#include <common/ac/string.h>

#include <common/error.h>
#include <libaegis/gif.h>
#include <libaegis/gif/private.h>
#include <common/mem.h>
#include <libaegis/sub.h>
#include <common/trace.h>

//
// This symbol is defined if you want the code to tolerate
// some really stupid deviations from the stated GIF standard.
//
#define TOLERATE_FOOLS


struct table_ty
{
    int		    emit;
    int		    chain;
};

enum state_ty
{
    state_normal,
    state_before_clear,
    state_after_clear
};


static void
format_error(const char *fn)
{
    sub_context_ty  *scp;

    scp = sub_context_new();
    sub_var_set_charstar(scp, "File_Name", fn);
    fatal_intl(scp, i18n("read $filename: format error"));
    // NOTREACHED
}


static int
gif_getc(FILE *fp, const char *fn)
{
    int		    c;

    c = getc(fp);
    if (c == EOF)
    {
	if (ferror(fp))
	{
	    sub_context_ty  *scp;
	    int             errno_old;

	    errno_old = errno;
	    scp = sub_context_new();
	    sub_errno_setx(scp, errno_old);
	    sub_var_set_charstar(scp, "File_Name", fn);
	    fatal_intl(scp, i18n("read $filename: $errno"));
	    // NOTREACHED
	}
	format_error(fn);
    }
    return c;
}


static void
read_image(FILE *fp, const char *fn, int row_bytes, unsigned char *data,
    int width, int height, int interlaced)
{
    int		    c;
    int		    unpack_buffer;
    int		    unpack_buffer_len;
    int		    unpack_runlen;

    int		    clear_cmd;
    int		    end_cmd;

    int		    code_prev = 0; // last chain
    int		    code_cur; // current chain
    int		    code_size;
    int		    code_size_minimum;
    int		    code_mask;
    int		    pixel_prev = 0; // last pixel of last chain
    int		    pixel_cur; // last pixel of current chain (eventually)
    int		    phase;
    int		    x;
    int  	    y;
    unsigned char   *dp;

    table_ty	    table[1 << MAX_BITS];
    int		    table_length =  0;
    int		    table_pos_minimum;
    int		    table_pos =	    0;

    state_ty	    state;

    unsigned char   lifo[1 << MAX_BITS];
    unsigned char   *lifo_ptr;

    c = gif_getc(fp, fn);
    trace(("code_size = %d;", c));
    if (c < 2 || c > 8)
    {
	//
	// bad code size
	//
	format_error(fn);
    }
    code_size = c;
    clear_cmd = (1 << code_size);
    end_cmd = clear_cmd + 1;
    table_pos_minimum = clear_cmd + 2;

    code_size++;
    code_size_minimum = code_size;
    code_mask = (1 << code_size) - 1;

    x = 0;
    y = 0;
    phase = 0;
    dp = data;
    unpack_buffer = 0;
    unpack_buffer_len = 0;
    unpack_runlen = 0;
    state = state_before_clear;
    lifo_ptr = lifo;
    for (;;)
    {
	//
	// read in the next code
	//
	while (unpack_buffer_len < code_size)
	{
	    if (!unpack_runlen)
	    {
		unpack_runlen = gif_getc(fp, fn);
		if (!unpack_runlen)
		{
		    //
		    // zero length packet too early
		    //
		    format_error(fn);
		}
	    }
	    c = gif_getc(fp, fn);
	    --unpack_runlen;
	    unpack_buffer |= c << unpack_buffer_len;
	    unpack_buffer_len += 8;
	}
	code_cur = unpack_buffer & code_mask;
	trace(("code_cur = 0x%.*X", (code_size+3)/4, code_cur));
	unpack_buffer >>= code_size;
	unpack_buffer_len -= code_size;

	//
	// see if we are finished
	//
	if (code_cur == end_cmd)
	    break;

	//
	// see if it is a clear code
	// reset everything
	//
	if (code_cur == clear_cmd)
	{
	    code_size = code_size_minimum;
	    table_length = (1 << code_size);
	    code_mask = table_length - 1;
	    trace(("code_size = %d;", code_size));
	    trace(("table_length = %d;", table_length));
	    trace(("code_mask = 0x%04X;", code_mask));
	    table_pos = table_pos_minimum;
	    trace(("table_pos = 0x%02X;", table_pos));
	    state = state_after_clear;
	    continue;
	}
	switch ((int)state)
	{
	case state_before_clear:
	    //
	    // clear code missing
	    //
	    trace(("clear code missing\n"));
	    format_error(fn);
	    break;

	case state_after_clear:
	    //
	    // immediately after clear
	    // we must also initialize the "last" info
	    // Also, the data after a clear MUST be raw.
	    //
	    if (code_cur > code_mask)
	    {
		//
		// code after clear must be raw pixel
		//
		trace(("code after clear must be raw pixel\n"));
		format_error(fn);
	    }
	    pixel_prev = code_cur;
	    trace(("pixel_prev = 0x%03X;", pixel_prev));
	    break;
	}

	//
	// initially, pixel_cur is the table index
	//
	pixel_cur = code_cur;

	//
	// if it is the next element in the table,
	// then repeat the last pixel and the last chain.
	//
	if (pixel_cur > table_pos)
	{
	    //
	    // code off end of table
	    //
	    trace(("code off end of table\n"));
	    format_error(fn);
	}
	if (pixel_cur >= table_pos)
	{
	    pixel_cur = code_prev;
	    *lifo_ptr++ = pixel_prev;
	}

	//
	// follow the chain through the table
	//
	while (pixel_cur >= clear_cmd)
	{
	    *lifo_ptr++ = table[pixel_cur].emit;
	    pixel_cur = table[pixel_cur].chain;
	}
	pixel_prev = pixel_cur;
	trace(("pixel_prev = 0x%03X;", pixel_prev));
	*lifo_ptr++ = pixel_cur;

	//
	// emit the sequence, in reverse
	//
	while (lifo_ptr > lifo)
	{
	    if (y >= height)
	    {
		//
		// too many pixels!
		//
		trace(("too many pixels\n"));
		format_error(fn);
	    }
	    *dp++ = *--lifo_ptr;
	    trace(("pixel[%d][%d] = 0x%02X;", y, x, dp[-1]));
	    if (++x >= width)
	    {
		x = 0;
		if (interlaced)
		{
		    switch (phase)
		    {
		    case 0:
			y += 8;
			if (y >= height)
			{
			    phase++;
			    y = 4;
			}
			break;

		    case 1:
			y += 8;
			if (y >= height)
			{
			    phase++;
			    y = 2;
			}
			break;

		    case 2:
			y += 4;
			if (y >= height)
			{
			    phase++;
			    y = 1;
			}
			break;

		    case 3:
			y += 2;
			if (y >= height)
			    y = height;
			break;
		    }
		}
		else
		    ++y;
		dp = data + y * row_bytes;
	    }
	}

	//
	// add the next element to the table
	//
	if (state == state_normal)
	{
	    if (table_pos >= (1 << MAX_BITS))
	    {
		//
		// tries to write code off end of table
		//
		trace(("tries to write code off end of table"));
		format_error(fn);
	    }
	    trace(("table_pos = 0x%02X;", table_pos));
	    table[table_pos].chain = code_prev;
	    trace(("table[0x%03X].chain = 0x%03X;", table_pos, code_prev));
	    table[table_pos].emit = pixel_cur;
	    trace(("table[0x%03X].emit = 0x%03X;", table_pos, pixel_cur));
	    table_pos++;
	}
	code_prev = code_cur;
	trace(("code_prev = 0x%03X;", code_prev));
	state = state_normal;

	//
	// move the table position on
	//
	if (table_pos >= table_length)
	{
	    if (code_size < MAX_BITS)
	    {
		code_size++;
		table_length <<= 1;
		code_mask = table_length - 1;
		trace(("code_size = %d;", code_size));
		trace(("table_length = %d;", table_length));
		trace(("code_mask = 0x%04X;", code_mask));
	    }
	    else
		state = state_before_clear;
	}
    }

    //
    // check that we were given all of the lines
    //
    // Many implementations don't write the last line
    // (probably anencephalic programmers),
    // and some assume that absent lines mean pixel number 0.
    //
#ifndef TOLERATE_FOOLS
    if (y != height)
    {
	trace(("too few pixels (y = %d, height, x = %d, width = %d)\n",
	    y, height, x, width));
	format_error(fn);
    }
#endif

    //
    // check that all of last packet was used
    //
    // Many imlementations always write 255 byte packets,
    // even when the last packet is only partially full.
    //
#ifndef TOLERATE_FOOLS
    if (unpack_runlen)
    {
	trace(("last packet not all used"));
	format_error(fn);
    }
#endif

    //
    // Check for zero-length packet at end of image.
    //
    // Many implementations leave it out.
    //
    c = gif_getc(fp, fn);
    if (c)
    {
#ifdef TOLERATE_FOOLS
	ungetc(c, fp);
#else
	trace(("image 0 terminator missing"));
	format_error(fn);
#endif
    }
}


static int
get_le_short(FILE *fp, const char *fn)
{
    int		    c1;
    int		    c2;

    //
    // must use temp vars
    // to guarantee order of evaluation
    // (see ANSI C std, discussion of sequence points)
    //
    c1 = gif_getc(fp, fn);
    c2 = gif_getc(fp, fn);
    return (c1 + (c2 << 8));
}


gif_ty *
gif_open(const char *fn, int mode)
{
    gif_ty	    *result;
    int		    j;
    FILE	    *fp;
    unsigned char   *cp;
    int		    c;
    int		    bits_per_pixel;
    int		    read_colormap;

    trace(("gif_open(fn = \"%s\", mode = %d)", fn, mode));

    //
    // initialize things
    //
    result = (gif_ty *)mem_alloc(sizeof(gif_ty));
    for (j = 0; j < 256; ++j)
    {
	result->colormap[j][0] = j;
	result->colormap[j][1] = j;
	result->colormap[j][2] = j;
    }
    result->image_flat = 0;
    result->image = 0;
    result->mode = (gif_mode_ty)mode;
    result->mime = 0;

    //
    // open the file
    //
    if (fn)
    {
	result->fn = mem_copy_string(fn);
	fp = fopen(fn, "rb");
	if (!fp)
	{
	    sub_context_ty  *scp;
	    int             errno_old;

	    errno_old = errno;
	    scp = sub_context_new();
	    sub_errno_setx(scp, errno_old);
	    sub_var_set_charstar(scp, "File_Name", fn);
	    fatal_intl(scp, i18n("open $filename: $errno"));
	    // NOTREACHED
	}
    }
    else
    {
	result->fn = 0;
	fn = "standard input";
	fp = stdin;
    }

    //
    // make sure the magic number is there
    //
    for (cp = gif_magic_number; *cp; ++cp)
    {
	c = gif_getc(fp, fn);
	if (c != *cp)
	    format_error(fn);
    }

    //
    // get the size
    //
    result->width = get_le_short(fp, fn);
    if (!result->width || result->width > (int)SIZE_MAX)
	format_error(fn);
    result->height = get_le_short(fp, fn);
    if (!result->height || result->height > (int)SIZE_MAX)
	format_error(fn);

    //
    // allocate the image
    //
    result->image_flat = (unsigned char *)mem_alloc(
        (size_t)result->width * (size_t)result->height);
    result->image = (unsigned char **)mem_alloc(
        (size_t)result->height * sizeof(unsigned char *));
    for (j = 0; j < result->height; ++j)
	result->image[j] = result->image_flat + j * result->width;

    //
    // get encoding info
    //
    c = gif_getc(fp, fn);
    bits_per_pixel = 1 + (c & 7);
    if (c & 0x80)
	read_colormap = 1 << bits_per_pixel;
    else
	read_colormap = 0;

    //
    // get background color
    //
    c = gif_getc(fp, fn);
    memset
    (
	result->image_flat,
	c,
	(size_t)result->width * (size_t)result->height
    );

    //
    // this byte should be zero
    //
    c = gif_getc(fp, fn);
    if (c != 0)
	format_error(fn);

    //
    // read the colormap
    //
    if (read_colormap)
    {
	for (j = 0; j < read_colormap; ++j)
	    for (c = 0; c < 3; ++c)
		result->colormap[j][c] = gif_getc(fp, fn);
	}

    //
    // read the image
    //
    for (;;)
    {
	c = gif_getc(fp, fn);
	switch (c)
	{
	default:
	    format_error(fn);
	    break;

	case ';':
	    if (fp != stdin)
		fclose(fp);
	    return result;

	case '!':
	    //
	    // extension block - ignore
	    //
	    c = gif_getc(fp, fn);
	    while (c > 0)
	    {
		gif_getc(fp, fn);
		--c;
	    }
	    break;

	case ',':
	    {
		int		image_left;
		int		image_top;
		int		image_width;
		int		image_height;
		int		flags;
		int		interlaced;

		//
		// image specifier
		//
		image_left = get_le_short(fp, fn);
		if (image_left >= result->width)
		    format_error(fn);
		image_top = get_le_short(fp, fn);
		if (image_top >= result->height)
		    format_error(fn);
		image_width = get_le_short(fp, fn);
		if (image_left + image_width > result->width)
		    format_error(fn);
		image_height = get_le_short(fp, fn);
		if (image_top + image_height > result->height)
		    format_error(fn);
		flags = gif_getc(fp, fn);
		if (flags & 0x80)
		{
		    //
		    // uses local color map,
		    // which we can't cope with
		    //
		    format_error(fn);
		}
		interlaced = (flags & 0x40) != 0;

		read_image
		(
		    fp,
		    fn,
		    result->width,
		    result->image_flat + image_left + image_top * result->width,
		    image_width,
		    image_height,
		    interlaced
		);
	    }
	    break;
	}
    }
}

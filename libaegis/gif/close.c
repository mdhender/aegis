/*
 *	aegis - project change supervisor
 *	Copyright (C) 1998, 1999, 2002, 2003 Peter Miller;
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
 * MANIFEST: function to close GIF files
 */

#include <ac/stdio.h>

#include <error.h>
#include <gif.h>
#include <gif/private.h>
#include <mem.h>
#include <sub.h>
#include <trace.h>

static unsigned char block[256];
static int	block_len;
static unsigned long pack_buffer;
static int	pack_buffer_len;


typedef struct table_ty table_ty;
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
typedef enum state_ty state_ty;


static void
gif_putc(FILE *fp, const char *fn, int c)
{
    putc(c, fp);
    if (ferror(fp))
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_errno_set(scp);
	sub_var_set_charstar(scp, "File_Name", fn);
	fatal_intl(scp, i18n("write $filename: $errno"));
	/* NOTREACHED */
    }
}


static void
put_le_short(FILE *fp, const char *fn, int n)
{
    gif_putc(fp, fn, n & 0xFF);
    gif_putc(fp, fn, (n >> 8) & 0xFF);
}


static void
oof_flush(FILE *fp, const char *fn)
{
    int		    j;

    if (!block_len)
	return;
    gif_putc(fp, fn, block_len);
    for (j = 0; j < block_len; ++j)
	gif_putc(fp, fn, block[j]);
    block_len = 0;
}


static void
oof(FILE *fp, const char *fn, int c)
{
    if (block_len >= 255)
	oof_flush(fp, fn);
    block[block_len++] = c;
}


static void
output(FILE *fp, const char *fn, int code, int code_size)
{
    trace(("output(fp = %08lX, fn = %08lX, code = 0x%.*X, code_size = %d)",
	(long)fp, (long)fn, (code_size + 3) / 4, code, code_size));
    pack_buffer |= code << pack_buffer_len;
    pack_buffer_len += code_size;
    trace(("pack_buffer = 0x%.*lX", (pack_buffer_len + 3) / 4, pack_buffer));
    trace(("pack_buffer_len = %d;", pack_buffer_len));

    while (pack_buffer_len >= 8)
    {
	oof(fp, fn, pack_buffer & 0xFF);
	pack_buffer >>= 8;
	pack_buffer_len -= 8;
	trace(("pack_buffer = 0x%.*lX",
	    (pack_buffer_len + 3) / 4, pack_buffer));
	trace(("pack_buffer_len = %d;", pack_buffer_len));
    }
}


#define IMPOSSIBLE 0xFFFF


static void
write_image(FILE *fp, const char *fn, unsigned char *data, int width,
    int height, int init_bits)
{
    int		    clear_cmd;
    int		    code_cur =	    0;	/* position in chain being built */
    int		    code_max;		/* maximum code, given code_size */
    int		    code_next;		/* candidate for chain extension */
    int		    code_prev =	    0;	/* last code emitted */
    int		    code_size;		/* number of bits/code */
    int		    code_size_minimum;
    int		    end_cmd;
    unsigned short  invert[1 << (MAX_BITS + 1)];
    int		    invert_shift;
    int		    invert_pos;
    int		    j;
    int		    pixel_cur =	    0;	/* last pixel of current chain */
    int		    pixel_prev =    0;	/* last pixel of previous chain */
    int		    pixel_next;		/* candidate for chain extension */
    int		    pixel_first =   0;	/* first pixel of current chain */
    state_ty	    state;
    table_ty	    table[1 << MAX_BITS];
    int		    table_pos =	    0;	/* first unused entry */
    int		    table_pos_minimum;
    table_ty	    *tp;
    int		    x;
    int		    y;
    int		    new_chain;

    trace(("write_image(fp = %08lX, fn = %8.8lX, data = %8.8lX, width = %d, "
	"height = %d, init_bits = %d)", (long)fp, (long)fn, (long)data, width,
	height, init_bits));

    /*
     * Write out the initial code size
     */
    if (init_bits < 2)
	init_bits = 2;
    gif_putc(fp, fn, init_bits);

    /*
     * Set up the necessary values
     */
    pack_buffer = 0;
    pack_buffer_len = 0;
    code_size = init_bits + 1;
    code_max = 1 << code_size;
    code_size_minimum = code_size;
    clear_cmd = 1 << init_bits;
    end_cmd = clear_cmd + 1;
    table_pos_minimum = clear_cmd + 2;
    invert_shift = (MAX_BITS + 1) - init_bits;

    for (j = 0; j < clear_cmd; ++j)
    {
	tp = &table[j];
	tp->chain = 0;
	tp->emit = j;
    }

    x = 0;
    y = 0;
    state = state_before_clear;
    for (;;)
    {
	if (state == state_before_clear)
	{
	    output(fp, fn, clear_cmd, code_size);
	    for (j = 0; j < (1 << (MAX_BITS + 1)); ++j)
		invert[j] = 0;
	    table_pos = table_pos_minimum;
	    trace(("table_pos = 0x%03X;", table_pos));
	    code_size = code_size_minimum;
	    trace(("code_size = %d;", code_size));
	    code_max = 1 << code_size;
	    trace(("code_max = 0x%03X;", code_max));
	    state = state_after_clear;
	    code_cur = IMPOSSIBLE; /* no current chain being built */
	    code_prev = IMPOSSIBLE; /* no previously emitted code */
	    pixel_cur = IMPOSSIBLE; /* no current chain being built */
	    pixel_prev = IMPOSSIBLE; /* no previously emitted code */
	    trace(("code_cur = 0x%03X;", code_cur));
	    trace(("code_prev = 0x%03X;", code_prev));
	    trace(("pixel_cur = 0x%03X;", pixel_cur));
	    trace(("pixel_prev = 0x%03X;", pixel_prev));
	}

	if (y >= height)
	{
	    if (code_cur != IMPOSSIBLE)
	    {
		output(fp, fn, code_cur, code_size);
		if (table_pos > code_max && code_size < MAX_BITS)
		    code_size++;
	    }
	    break;
	}

	trace(("x == %d, y == %d", x, y));
	pixel_next = *data++;
	trace(("pixel_next = 0x%02X;", pixel_next));
	if (++x >= width)
	{
	    x = 0;
	    ++y;
	}

	/*
	 * look through the inverted list for this combination
	 */
	if (code_cur != IMPOSSIBLE)
	{
	    invert_pos = (pixel_next << invert_shift) ^ code_cur;
	    for (;;)
	    {
		code_next = invert[invert_pos];
		if (code_next == 0)
		    break;
		tp = &table[code_next];
		if (tp->chain == code_cur && tp->emit == pixel_next)
		    break;
		invert_pos++;
		if (invert_pos >= (1 << (MAX_BITS + 1)))
		    invert_pos = 0;
	    }
	    trace(("invert_pos = %d; code_next = 0x%03X;", invert_pos,
		code_next));

	    /*
	     * if it already exists, then chain along it
	     */
	    if (code_next != 0)
	    {
		code_cur = code_next;
		trace(("code_cur = 0x%03X;", code_cur));
		pixel_cur = pixel_next;
		trace(("pixel_cur = 0x%03X;", pixel_cur));
		continue;
	    }
	}

	/*
	 * if it does not exist, but it is an extension of the last one,
	 * make one up on the end of the table
	 */
	if
	(
	    state == state_normal
	&&
	    code_cur == code_prev
	&&
	    pixel_first == pixel_next
	)
	{
	    code_cur = table_pos;
	    trace(("code_cur == 0x%03X;", code_cur));
	    pixel_cur = pixel_prev;
	    trace(("pixel_cur == 0x%03X;", pixel_cur));
	    new_chain = 0;
	}
	else
	    new_chain = 1;

	/*
	 * emit the current code
	 */
	if (code_cur != IMPOSSIBLE)
	{
	    output(fp, fn, code_cur, code_size);

	    /*
	     * add it to the table
	     */
	    if (state == state_normal && table_pos < (1 << MAX_BITS))
	    {
		trace(("table_pos == 0x%03X;", table_pos));
		tp = &table[table_pos];
		tp->emit = pixel_first;
		tp->chain = code_prev;
		invert_pos = (pixel_cur << invert_shift) ^ code_prev;
		invert[invert_pos] = table_pos;
		trace(("table[0x%03X].emit = 0x%03X;", table_pos, tp->emit));
		trace(("table[0x%03X].chain = 0x%03X;", table_pos, tp->chain));
		trace(("invert[invert_pos = %d] = 0x%03X;", invert_pos,
		    table_pos));
		++table_pos;
	    }
	    state = state_normal;

	    /*
	     * fix the code size
	     */
	    if (table_pos >= code_max)
	    {
		if (code_size < MAX_BITS)
		{
		    code_size++;
		    trace(("code_size = %d;", code_size));
		    code_max = 1 << code_size;
		    trace(("code_max = 0x%03X;", code_max));
		}
		else
		    state = state_before_clear;
	    }

	    /*
	     * only have a code_prev after emitting something
	     */
	    code_prev = code_cur;
	    trace(("code_prev = 0x%03X;", code_prev));
	    pixel_prev = pixel_cur;
	    trace(("pixel_prev = 0x%03X;", pixel_prev));
	}

	/*
	 * new chain starts from the pixel we didn't use
	 */
	if (new_chain)
	{
	    if (state == state_before_clear)
	    {
		--data;
		if (x)
		    --x;
		else
		{
		    --y;
		    x = width - 1;
		}
		code_cur = IMPOSSIBLE;
		pixel_cur = IMPOSSIBLE;
	    }
	    else
	    {
		code_cur = pixel_next;
		pixel_cur = pixel_next;
		pixel_first = pixel_next;
	    }
	}
	else
	{
	    code_cur = IMPOSSIBLE;
	    pixel_cur = IMPOSSIBLE;
	}
	trace(("code_cur = 0x%03X;", code_cur));
	trace(("pixel_cur = 0x%03X;", pixel_cur));
    }

    /*
     * finish up
     */
    output(fp, fn, end_cmd, code_size);
    if (pack_buffer_len)
	oof(fp, fn, pack_buffer & 0xFF);
    oof_flush(fp, fn);

    /*
     * empty last packet
     */
    gif_putc(fp, fn, 0);
}


static void
flush(gif_ty *gp)
{
    FILE	    *fp;
    const char	    *fn;
    int		    color_resolution;
    int		    bits_per_pixel;
    unsigned char   *cp;
    int		    j;
    int		    c;

    /*
     * initialize things
     */
    trace(("gif_flush(gp = %8.8lX)", (long)gp));
    if (gp->fn)
    {
	fn = gp->fn;
	fp = fopen(fn, "wb");
	if (!fp)
	{
	    sub_context_ty  *scp;

	    scp = sub_context_new();
	    sub_errno_set(scp);
	    sub_var_set_charstar(scp, "File_Name", fn);
	    fatal_intl(scp, i18n("create $filename: $errno"));
	    /* NOTREACHED */
	}
    }
    else
    {
	fn = "standard output";
	fp = stdout;
    }

    color_resolution = 8;
    bits_per_pixel = 8;

    /*
     * write out the header
     */
    if (gp->mime)
	fprintf(fp, "Content-Type: image/gif\n\n");
    for (cp = gif_magic_number; *cp; ++cp)
	gif_putc(fp, fn, *cp);
    put_le_short(fp, fn, gp->width);
    put_le_short(fp, fn, gp->height);
    gif_putc
    (
	fp,
	fn,
	(0x80 | ((unsigned)(color_resolution - 1) << 4) | (bits_per_pixel - 1))
    );
    gif_putc(fp, fn, 0); /* background */
    gif_putc(fp, fn, 0); /* future expansion */

    /*
     * global color map
     */
    for (j = 0; j < 256; ++j)
	for (c = 0; c < 3; ++c)
	    gif_putc(fp, fn, gp->colormap[j][c]);

    /*
     * image separator
     */
    gif_putc(fp, fn, ',');

    /*
     * image header
     *	 left, top, width, height
     */
    put_le_short(fp, fn, 0);
    put_le_short(fp, fn, 0);
    put_le_short(fp, fn, gp->width);
    put_le_short(fp, fn, gp->height);

    /*
     * the image is not interlaced
     * and has no local color map
     */
    gif_putc(fp, fn, 0);

    /*
     * write the image
     */
    write_image
    (
	fp,
	fn,
	gp->image_flat,
	gp->width,
	gp->height,
	bits_per_pixel
    );

    /*
     * file terminator
     */
    gif_putc(fp, fn, ';');

    /*
     * finish up
     */
    if (fflush(fp))
    {
	sub_context_ty  *scp;

	scp = sub_context_new();
	sub_var_set_charstar(scp, "File_Name", fn);
	fatal_intl(scp, i18n("write $filename: $errno"));
	/* NOTREACHED */
    }
    if (fp != stdout)
	fclose(fp);
}


void
gif_close(gif_ty *gp)
{
    if (gp->mode == gif_mode_rdwr)
	flush(gp);
    if (gp->fn)
	mem_free(gp->fn);
    mem_free(gp->image_flat);
    mem_free(gp->image);
    mem_free(gp);
}

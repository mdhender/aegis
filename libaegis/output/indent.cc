//
//	aegis - project change supervisor
//	Copyright (C) 1991-1993, 1995, 1998, 1999, 2002-2004 Peter Miller;
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
// MANIFEST: functions to filter output to be indented
//

#include <output/indent.h>
#include <output/private.h>
#include <str.h>
#include <trace.h>


//
// This defines the width of a tab on output.
//
#define INDENT 8


struct output_indent_ty
{
    output_ty	    inherited;
    output_ty	    *deeper;
    int		    depth;
    int		    in_col;
    int		    out_col;
    int		    continuation_line;
    long	    pos;
};


static void
output_indent_destructor(output_ty *fp)
{
    output_indent_ty *this_thing;

    trace(("output_indent::destructor()\n{\n"));
    this_thing = (output_indent_ty *)fp;
    trace_pointer(this_thing->deeper);
    if (this_thing->out_col)
	output_fputc(this_thing->deeper, '\n');
    output_delete(this_thing->deeper);
    this_thing->deeper = 0;
    trace(("}\n"));
}


static string_ty *
output_indent_filename(output_ty *fp)
{
    output_indent_ty *this_thing;

    this_thing = (output_indent_ty *)fp;
    return output_filename(this_thing->deeper);
}


static long
output_indent_ftell(output_ty *fp)
{
    output_indent_ty *this_thing;

    this_thing = (output_indent_ty *)fp;
    return this_thing->pos;
}


//
// Function Name:
//	indent_putchar
//
// Description:
//	The indent_putchar function is used to emity characters.
//	It keeps track of (){}[] pairs and indents between them.
//	Leading whitespace is suppressed and replaced with its own
//	idea of indenting.
//
// Preconditions:
//	none
//
// validation:
//	none
//
// Passed:
//	'c' the character to emit.
//

static void
output_indent_write(output_ty *fp, const void *p, size_t len)
{
    const unsigned char *data;
    output_indent_ty *this_thing;

    data = (unsigned char *)p;
    this_thing = (output_indent_ty *)fp;
    while (len > 0)
    {
	int		c;

	c = *data++;
	--len;
	this_thing->pos++;
	switch (c)
	{
	case '\n':
	    output_fputc(this_thing->deeper, '\n');
	    this_thing->in_col = 0;
	    this_thing->out_col = 0;
	    if (this_thing->continuation_line == 1)
		this_thing->continuation_line = 2;
	    else
		this_thing->continuation_line = 0;
	    break;

	case ' ':
	    if (this_thing->out_col)
		this_thing->in_col++;
	    break;

	case '\t':
	    if (this_thing->out_col)
		this_thing->in_col = (this_thing->in_col / INDENT + 1) * INDENT;
	    break;

	case '\1':
	    if (!this_thing->out_col)
		break;
	    if (this_thing->in_col >= INDENT * (this_thing->depth + 2))
		this_thing->in_col++;
	    else
		this_thing->in_col = INDENT * (this_thing->depth + 2);
	    break;

	case '}':
	case ')':
	case ']':
	    this_thing->depth--;
	    // fall through

	default:
	    if (!this_thing->out_col && c != '#' &&
                this_thing->continuation_line != 2)
		this_thing->in_col += INDENT * this_thing->depth;
	    if (!this_thing->out_col)
	    {
		//
		// Only emit tabs into the output if we are at
		// the start of a line.
		//
		for (;;)
		{
		    int		    x;

		    if (this_thing->out_col + 1 >= this_thing->in_col)
			break;
		    x = ((this_thing->out_col / INDENT) + 1) * INDENT;
		    if (x > this_thing->in_col)
			break;
		    output_fputc(this_thing->deeper, '\t');
		    this_thing->out_col = x;
		}
	    }
	    while (this_thing->out_col < this_thing->in_col)
	    {
		output_fputc(this_thing->deeper, ' ');
		this_thing->out_col++;
	    }
	    if (c == '{' || c == '(' || c == '[')
		this_thing->depth++;
	    output_fputc(this_thing->deeper, c);
	    this_thing->in_col++;
	    this_thing->out_col = this_thing->in_col;
	    this_thing->continuation_line = (c == '\\');
	    break;
	}
    }
}


static int
output_indent_page_width(output_ty *fp)
{
    output_indent_ty *this_thing;

    this_thing = (output_indent_ty *)fp;
    return output_page_width(this_thing->deeper);
}


static int
output_indent_page_length(output_ty *fp)
{
    output_indent_ty *this_thing;

    this_thing = (output_indent_ty *)fp;
    return output_page_length(this_thing->deeper);
}


static void
output_indent_eoln(output_ty *fp)
{
    output_indent_ty *this_thing;

    this_thing = (output_indent_ty *)fp;
    if (this_thing->in_col)
	output_fputc(fp, '\n');
}


static output_vtbl_ty vtbl =
{
    sizeof(output_indent_ty),
    output_indent_destructor,
    output_indent_filename,
    output_indent_ftell,
    output_indent_write,
    output_generic_flush,
    output_indent_page_width,
    output_indent_page_length,
    output_indent_eoln,
    "indent",
};


output_ty *
output_indent(output_ty	*deeper)
{
    output_ty	    *result;
    output_indent_ty *this_thing;

    trace(("output_indent(deeper = %08lX)\n{\n", (long)deeper));
    result = output_new(&vtbl);
    this_thing = (output_indent_ty *)result;
    this_thing->deeper = deeper;
    this_thing->depth = 0;
    this_thing->in_col = 0;
    this_thing->out_col = 0;
    this_thing->continuation_line = 0;
    this_thing->pos = 0;
    trace(("}\n"));
    return result;
}


//
// Function Name:
//	indent_more
//
// Description:
//	The indent_more function is used to increase the indenting
//	beyond the automatically calculated indent.
//
// Preconditions:
//	There must be a matching indent_less call.
//
// validataion:
//	none
//
// Passed:
//	nothing
//
// Returns:
//	nothing
//

void
output_indent_more(output_ty *fp)
{
    output_indent_ty *this_thing;

    if (fp->vptr != &vtbl)
	return;
    this_thing = (output_indent_ty *)fp;
    this_thing->depth++;
}


//
// Function Name:
//	indent_less
//
// Description:
//	The indent_less function is used to decrease the indenting
//	to less than the automatically calculated indent.
//
// Preconditions:
//	There must be a matching indent_more call.
//
// validataion:
//	none
//
// Passed:
//	nothing
//
// Returns:
//	nothing
//

void
output_indent_less(output_ty *fp)
{
    output_indent_ty *this_thing;

    if (fp->vptr != &vtbl)
	return;
    this_thing = (output_indent_ty *)fp;
    if (this_thing->depth > 0)
	this_thing->depth--;
}

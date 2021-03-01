/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1995, 1998, 1999 Peter Miller;
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
 * MANIFEST: functions to filter output to be indented
 */

#include <output/indent.h>
#include <output/private.h>
#include <str.h>
#include <trace.h>


/*
 * This defines the width of a tab on output.
 */
#define INDENT 8


typedef struct output_indent_ty output_indent_ty;
struct output_indent_ty
{
	output_ty	inherited;
	output_ty	*deeper;
	int		depth;
	int		in_col;
	int		out_col;
	int		continuation_line;
	long		pos;
};


static void output_indent_destructor _((output_ty *));

static void
output_indent_destructor(fp)
	output_ty	*fp;
{
	output_indent_ty *this;

	trace(("output_indent::destructor()\n{\n"/*}*/));
	this = (output_indent_ty *)fp;
	trace_pointer(this->deeper);
	if (this->out_col)
		output_fputc(this->deeper, '\n');
	output_delete(this->deeper);
	this->deeper = 0;
	trace((/*{*/"}\n"));
}


static string_ty *output_indent_filename _((output_ty *));

static string_ty *
output_indent_filename(fp)
	output_ty	*fp;
{
	output_indent_ty *this;

	this = (output_indent_ty *)fp;
	return output_filename(this->deeper);
}


static long output_indent_ftell _((output_ty *));

static long
output_indent_ftell(fp)
	output_ty	*fp;
{
	output_indent_ty *this;

	this = (output_indent_ty *)fp;
	return this->pos;
}


/*
 * Function Name:
 *	indent_putchar
 *
 * Description:
 *	The indent_putchar function is used to emity characters.
 *	It keeps track of (){}[] pairs and indents between them.
 *	Leading whitespace is suppressed and replaced with its own
 *	idea of indenting.
 *
 * Preconditions:
 *	none
 *
 * validation:
 *	none
 *
 * Passed:
 *	'c' the character to emit.
 */

static void output_indent_write _((output_ty *, const void *, size_t));

static void
output_indent_write(fp, p, len)
	output_ty	*fp;
	const void	*p;
	size_t		len;
{
	const unsigned char *data;
	output_indent_ty *this;

	data = p;
	this = (output_indent_ty *)fp;
	while (len > 0)
	{
		int c = *data++;
		--len;

		this->pos++;
		switch (c)
		{
		case '\n':
			output_fputc(this->deeper, '\n');
			this->in_col = 0;
			this->out_col = 0;
			if (this->continuation_line == 1)
				this->continuation_line = 2;
			else
				this->continuation_line = 0;
			break;
	
		case ' ':
			if (this->out_col)
				this->in_col++;
			break;
	
		case '\t':
			if (this->out_col)
				this->in_col = (this->in_col / INDENT + 1) * INDENT;
			break;
	
		case '\1':
			if (!this->out_col)
				break;
			if (this->in_col >= INDENT * (this->depth + 2))
				this->in_col++;
			else
				this->in_col = INDENT * (this->depth + 2);
			break;
	
		case /*{*/'}':
		case /*(*/')':
		case /*[*/']':
			this->depth--;
			/* fall through */
	
		default:
			if (!this->out_col && c != '#' && this->continuation_line != 2)
				this->in_col += INDENT * this->depth;
			if (!this->out_col)
			{
				/*
				 * Only emit tabs into the output if we are at
				 * the start of a line.
				 */
				for (;;)
				{
					int	x;
	
					if (this->out_col + 1 >= this->in_col)
						break;
					x = ((this->out_col / INDENT) + 1) * INDENT;
					if (x > this->in_col)
						break;
					output_fputc(this->deeper, '\t');
					this->out_col = x;
				}
			}
			while (this->out_col < this->in_col)
			{
				output_fputc(this->deeper, ' ');
				this->out_col++;
			}
			if (c == '{'/*}*/ || c == '('/*)*/ || c == '['/*]*/)
				this->depth++;
			output_fputc(this->deeper, c);
			this->in_col++;
			this->out_col = this->in_col;
			this->continuation_line = (c == '\\');
			break;
		}
	}
}


static int output_indent_page_width _((output_ty *));

static int
output_indent_page_width(fp)
	output_ty	*fp;
{
	output_indent_ty *this;

	this = (output_indent_ty *)fp;
	return output_page_width(this->deeper);
}


static int output_indent_page_length _((output_ty *));

static int
output_indent_page_length(fp)
	output_ty	*fp;
{
	output_indent_ty *this;

	this = (output_indent_ty *)fp;
	return output_page_length(this->deeper);
}


static void output_indent_eoln _((output_ty *));

static void
output_indent_eoln(fp)
	output_ty	*fp;
{
	output_indent_ty *this;

	this = (output_indent_ty *)fp;
	if (this->in_col)
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
output_indent(deeper)
	output_ty	*deeper;
{
	output_ty	*result;
	output_indent_ty *this;

	trace(("output_indent(deeper = %08lX)\n{\n"/*}*/, (long)deeper));
	result = output_new(&vtbl);
	this = (output_indent_ty *)result;
	this->deeper = deeper;
	this->depth = 0;
	this->in_col = 0;
	this->out_col = 0;
	this->continuation_line = 0;
	this->pos = 0;
	trace((/*{*/"}\n"));
	return result;
}


/*
 * Function Name:
 *	indent_more
 *
 * Description:
 *	The indent_more function is used to increase the indenting
 *	beyond the automatically calculated indent.
 *
 * Preconditions:
 *	There must be a matching indent_less call.
 *
 * validataion:
 *	none
 *
 * Passed:
 *	nothing
 *
 * Returns:
 *	nothing
 */

void
output_indent_more(fp)
	output_ty	*fp;
{
	output_indent_ty *this;

	if (fp->vptr != &vtbl)
		return;
	this = (output_indent_ty *)fp;
	this->depth++;
}


/*
 * Function Name:
 *	indent_less
 *
 * Description:
 *	The indent_less function is used to decrease the indenting
 *	to less than the automatically calculated indent.
 *
 * Preconditions:
 *	There must be a matching indent_more call.
 *
 * validataion:
 *	none
 *
 * Passed:
 *	nothing
 *
 * Returns:
 *	nothing
 */

void
output_indent_less(fp)
	output_ty	*fp;
{
	output_indent_ty *this;

	if (fp->vptr != &vtbl)
		return;
	this = (output_indent_ty *)fp;
	if (this->depth > 0)
		this->depth--;
}

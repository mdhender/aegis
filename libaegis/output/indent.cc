//
//	aegis - project change supervisor
//	Copyright (C) 1991-1993, 1995, 1998, 1999, 2002-2005 Peter Miller;
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

#include <libaegis/output/indent.h>
#include <common/str.h>
#include <common/trace.h>


output_indent_ty::~output_indent_ty()
{
    trace(("output_indent::destructor()\n{\n"));

    //
    // Make sure all buffered data has been passed to our write_inner
    // method.
    //
    flush();

    trace_pointer(deeper);
    if (out_col)
	deeper->fputc('\n');
    if (close_on_close)
	delete deeper;
    deeper = 0;
    trace(("}\n"));
}


output_indent_ty::output_indent_ty(output_ty *arg1, bool arg2) :
    deeper(arg1),
    close_on_close(arg2),
    depth(0),
    in_col(0),
    out_col(0),
    continuation_line(0),
    pos(0)
{
    trace(("output_indent_ty(deeper = %08lX)\n", (long)deeper));
}


string_ty *
output_indent_ty::filename()
    const
{
    return deeper->filename();
}


long
output_indent_ty::ftell_inner()
    const
{
    return pos;
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

void
output_indent_ty::write_inner(const void *p, size_t len)
{
    const unsigned char *data = (unsigned char *)p;
    while (len > 0)
    {
	unsigned char c = *data++;
	--len;
	++pos;
	switch (c)
	{
	case '\n':
	    deeper->fputc('\n');
	    in_col = 0;
	    out_col = 0;
	    if (continuation_line == 1)
		continuation_line = 2;
	    else
		continuation_line = 0;
	    break;

	case ' ':
	    if (out_col)
		++in_col;
	    break;

	case '\t':
	    if (out_col)
		in_col = (in_col / INDENT + 1) * INDENT;
	    break;

	case '\1':
	    if (!out_col)
		break;
	    if (in_col >= INDENT * (depth + 2))
		in_col++;
	    else
		in_col = INDENT * (depth + 2);
	    break;

	case '}':
	case ')':
	case ']':
	    --depth;
	    // fall through

	default:
	    if (!out_col && c != '#' && continuation_line != 2)
		in_col += INDENT * depth;
	    if (!out_col)
	    {
		//
		// Only emit tabs into the output if we are at
		// the start of a line.
		//
		for (;;)
		{
		    if (out_col + 1 >= in_col)
			break;
		    int x = ((out_col / INDENT) + 1) * INDENT;
		    if (x > in_col)
			break;
		    deeper->fputc('\t');
		    out_col = x;
		}
	    }
	    while (out_col < in_col)
	    {
		deeper->fputc(' ');
		++out_col;
	    }
	    if (c == '{' || c == '(' || c == '[')
		++depth;
	    deeper->fputc(c);
	    ++in_col;
	    out_col = in_col;
	    continuation_line = (c == '\\');
	    break;
	}
    }
}


int
output_indent_ty::page_width()
    const
{
    return deeper->page_width();
}


int
output_indent_ty::page_length()
    const
{
    return deeper->page_length();
}


void
output_indent_ty::end_of_line_inner()
{
    if (in_col)
	fputc('\n');
}


const char *
output_indent_ty::type_name()
    const
{
    return "indent";
}


void
output_indent_ty::indent_more()
{
    ++depth;
}


void
output_indent_ty::indent_less()
{
    if (depth > 0)
	--depth;
}

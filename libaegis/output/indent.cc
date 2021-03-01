//
//	aegis - project change supervisor
//	Copyright (C) 1991-1993, 1995, 1998, 1999, 2002-2006, 2008 Peter Miller
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

#include <libaegis/output/indent.h>
#include <common/str.h>
#include <common/trace.h>


output_indent::~output_indent()
{
    trace(("output_indent::destructor()\n{\n"));

    //
    // Make sure all buffered data has been passed to our write_inner
    // method.
    //
    flush();

    if (out_col)
	deeper->fputc('\n');
    trace(("}\n"));
}


output_indent::output_indent(const output::pointer &a_deeper) :
    deeper(a_deeper),
    depth(0),
    in_col(0),
    out_col(0),
    continuation_line(0),
    pos(0)
{
    trace(("output_indent::output_indent(this = %08lX, deeper = %08lX)\n",
        (long)this, (long)a_deeper.get()));
}


output_indent::ipointer
output_indent::create(const output::pointer &a_deeper)
{
    return ipointer(new output_indent(a_deeper));
}


nstring
output_indent::filename()
    const
{
    return deeper->filename();
}


long
output_indent::ftell_inner()
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
output_indent::write_inner(const void *p, size_t len)
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
output_indent::page_width()
    const
{
    return deeper->page_width();
}


int
output_indent::page_length()
    const
{
    return deeper->page_length();
}


void
output_indent::end_of_line_inner()
{
    if (in_col)
	fputc('\n');
}


const char *
output_indent::type_name()
    const
{
    return "indent";
}


void
output_indent::indent_more()
{
    ++depth;
}


void
output_indent::indent_less()
{
    if (depth > 0)
	--depth;
}

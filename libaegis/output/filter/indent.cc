//
// aegis - project change supervisor
// Copyright (C) 1991-1999, 2002-2006, 2008, 2011, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/trace.h>

#include <libaegis/output/filter/indent.h>


output_filter_indent::~output_filter_indent()
{
    trace(("output_filter_indent::destructor()\n{\n"));

    //
    // Make sure all buffered data has been passed to our write_inner
    // method.
    //
    flush();
    end_of_line_inner();
    trace(("}\n"));
}


output_filter_indent::output_filter_indent(const output::pointer &a_deeper) :
    output_filter(a_deeper),
    depth(0),
    in_col(0),
    out_col(0),
    continuation_line(0),
    pos(0)
{
    trace(("output_filter_indent::output_filter_indent(this = %p, "
        "deeper = %p)\n", this, a_deeper.get()));
}


output_filter_indent::ipointer
output_filter_indent::create(const output::pointer &a_deeper)
{
    return ipointer(new output_filter_indent(a_deeper));
}


long
output_filter_indent::ftell_inner(void)
    const
{
    return pos;
}


//
// Function Name:
//      indent_putchar
//
// Description:
//      The indent_putchar function is used to emity characters.
//      It keeps track of (){}[] pairs and indents between them.
//      Leading whitespace is suppressed and replaced with its own
//      idea of indenting.
//
// Preconditions:
//      none
//
// validation:
//      none
//
// Passed:
//      'c' the character to emit.
//

void
output_filter_indent::write_inner(const void *p, size_t len)
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
            deeper_fputc('\n');
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
                    deeper_fputc('\t');
                    out_col = x;
                }
            }
            while (out_col < in_col)
            {
                deeper_fputc(' ');
                ++out_col;
            }
            if (c == '{' || c == '(' || c == '[')
                ++depth;
            deeper_fputc(c);
            ++in_col;
            out_col = in_col;
            continuation_line = (c == '\\');
            break;
        }
    }
}


void
output_filter_indent::end_of_line_inner(void)
{
    if (in_col)
        write_inner("\n", 1);
}


nstring
output_filter_indent::type_name(void)
    const
{
    return ("indent " + output_filter::type_name());
}


void
output_filter_indent::indent_more(void)
{
    ++depth;
}


void
output_filter_indent::indent_less(void)
{
    if (depth > 0)
        --depth;
}


// vim: set ts=8 sw=4 et :

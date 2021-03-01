//
//	aegis - project change supervisor
//	Copyright (C) 1991-1993, 1995, 1999, 2002-2006, 2008 Peter Miller
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

#include <common/ac/stdio.h>
#include <common/ac/stdarg.h>
#include <common/ac/errno.h>

#include <common/error.h>
#include <fmtgen/indent.h>
#include <common/trace.h>


#define INDENT 4

static FILE	*fp;
static const char *fn;
static int	depth;
static int	in_col;
static int	out_col;
static int	continuation_line;
static int	within_string;


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
indent_more(void)
{
    ++depth;
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
indent_less(void)
{
    --depth;
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
indent_putchar(int c)
{
    assert(fp);
    switch (c)
    {
    case '\n':
	fputc('\n', fp);
#ifdef DEBUG
	fflush(fp);
#endif
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
	if (in_col >= INDENT * depth + 16)
	    ++in_col;
	else
	    in_col = INDENT * depth + 16;
	break;

    case '"':
    case '\'':
	if (within_string == 0)
	    within_string = c;
	else if (continuation_line != 1 && within_string == c)
	    within_string = 0;
	goto normal;

    case '}':
    case ')':
    case ']':
	if (!within_string)
	    --depth;
	// fall through

    default:
	normal:
	if (!out_col && c != '#' && continuation_line != 2)
	    in_col += INDENT * depth;
	while (((out_col + 8) & -8) <= in_col && out_col + 1 < in_col)
	{
	    fputc('\t', fp);
	    out_col = (out_col + 8) & -8;
	}
	while (out_col < in_col)
	{
	    fputc(' ', fp);
	    ++out_col;
	}
	if (!within_string && (c == '{' || c == '(' || c == '['))
	    ++depth;
	fputc(c, fp);
	in_col++;
	out_col = in_col;
	continuation_line = (c == '\\');
	break;
    }
    if (ferror(fp))
	nfatal("write \"%s\"", fn);
}


//
// Function Name:
//	indent_printf
//
// Description:
//	As putchar is to indent_putchasr, printf is to indent_printf.
//
// Preconditions:
//	none
//
// Validation:
//	none
//
// Passed:
//	's' format string
//	... and optional arguments
//
// Returns:
//	nothing
//

void
indent_printf(const char *s, ...)
{
    va_list	    ap;
    char	    buffer[2000];

    va_start(ap, s);
    vsnprintf(buffer, sizeof(buffer), s, ap);
    va_end(ap);
    for (s = buffer; *s; ++s)
	indent_putchar(*s);
}


void
indent_open(const char *s)
{
    trace(("indent_open(s = %08lX)\n{\n", (long)s));
    if (!s)
    {
	fp = stdout;
	s = "(stdout)";
    }
    else
    {
	trace_string(s);
	fp = fopen(s, "w");
	if (!fp)
    	    nfatal("open \"%s\"", s);
    }
    trace_pointer(fp);
    fn = s;
    depth = 0;
    in_col = 0;
    out_col = 0;
    continuation_line = 0;
    trace(("}\n"));
}


void
indent_close(void)
{
    trace(("indent_close()\n{\n"));
    trace_pointer(fp);
    if (out_col)
	indent_putchar('\n');
    if (fflush(fp))
	nfatal("write \"%s\"", fn);
    if (fp != stdout && fclose(fp))
	nfatal("close \"%s\"", fn);
    fp = 0;
    fn = 0;
    trace(("}\n"));
}

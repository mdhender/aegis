/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1995 Peter Miller;
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
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * MANIFEST: functions to automatically indent output
 */

#include <stdio.h>
#include <ac/stdarg.h>
#include <errno.h>

#include <error.h>
#include <indent.h>
#include <trace.h>


#define INDENT 8

static	FILE	*fp;
static	char	*fn;
static int	depth;
static int	in_col;
static int	out_col;
static	int	continuation_line;


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
indent_more()
{
	++depth;
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
indent_less()
{
	--depth;
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

void
indent_putchar(c)
	char	c;
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
		if (in_col >= INDENT * (depth + 2))
			++in_col;
		else
			in_col = INDENT * (depth + 2);
		break;

	case /*{*/'}':
	case /*(*/')':
	case /*[*/']':
		--depth;
		/* fall through */

	default:
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
		if (c == '{'/*}*/ || c == '('/*)*/ || c == '['/*]*/)
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


/*
 * Function Name:
 *	indent_printf
 *
 * Description:
 *	As putchar is to indent_putchasr, printf is to indent_printf.
 *
 * Preconditions:
 *	none
 *
 * Validation:
 *	none
 *
 * Passed:
 *	's' format string
 *	... and optional arguments
 *
 * Returns:
 *	nothing
 */

/*VARARGS1*/
void
indent_printf(s sva_last)
	char		*s;
	sva_last_decl
{
	va_list		ap;
	char		buffer[2000];

	sva_init(ap, s);
	vsprintf(buffer, s, ap);
	va_end(ap);
	for (s = buffer; *s; ++s)
		indent_putchar(*s);
}


void
indent_open(s)
	char	*s;
{
	trace(("indent_open(s = %08lX)\n{\n"/*}*/, s));
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
	trace((/*{*/"}\n"));
}


void
indent_close()
{
	trace(("indent_close()\n{\n"/*}*/));
	trace_pointer(fp);
	if (out_col)
		indent_putchar('\n');
	if (fflush(fp))
		nfatal("write \"%s\"", fn);
	if (fp != stdout && fclose(fp))
		nfatal("close \"%s\"", fn);
	fp = 0;
	fn = 0;
	trace((/*{*/"}\n"));
}

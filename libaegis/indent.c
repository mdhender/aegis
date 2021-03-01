/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1995, 1998 Peter Miller;
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
 * MANIFEST: functions to automatically indent output
 */

#include <ac/stdio.h>
#include <errno.h>
#include <ac/stdarg.h>

#include <error.h>
#include <fopen_nfs.h>
#include <glue.h>
#include <indent.h>
#include <sub.h>
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
		glue_fputc('\n', fp);
#ifdef DEBUG
		glue_fflush(fp);
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
		if (!out_col)
		{
			/*
			 * Only emit tabs into the output if we are at
			 * the start of a line.
			 */
			while (((out_col + 8) & -8) <= in_col && out_col + 1 < in_col)
			{
				glue_fputc('\t', fp);
				out_col = (out_col + 8) & -8;
			}
		}
		while (out_col < in_col)
		{
			glue_fputc(' ', fp);
			++out_col;
		}
		if (c == '{'/*}*/ || c == '('/*)*/ || c == '['/*]*/)
			++depth;
		glue_fputc(c, fp);
		in_col++;
		out_col = in_col;
		continuation_line = (c == '\\');
		break;
	}
	if (glue_ferror(fp))
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name", "%s", fn);
		fatal_intl(scp, i18n("write $filename: $errno"));
		/* NOTREACHED */
	}
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
		fp = fopen_with_stale_nfs_retry(s, "w");
		if (!fp)
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_errno_set(scp);
			sub_var_set(scp, "File_Name", "%s", s);
			fatal_intl(scp, i18n("open $filename: $errno"));
			/* NOTREACHED */
		}
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
	if (glue_fflush(fp))
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name", "%s", fn);
		fatal_intl(scp, i18n("write $filename: $errno"));
		/* NOTREACHED */
	}
	if (fp != stdout && glue_fclose(fp))
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name", "%s", fn);
		fatal_intl(scp, i18n("close $filename: $errno"));
		/* NOTREACHED */
	}
	fp = 0;
	fn = 0;
	trace((/*{*/"}\n"));
}

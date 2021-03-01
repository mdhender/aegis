/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993 Peter Miller.
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
 * MANIFEST: functions to provide consistent treatment of -Help options
 */

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <arglex2.h>
#include <error.h>
#include <gonzo.h>
#include <help.h>
#include <option.h>
#include <pager.h>
#include <patchlevel.h>
#include <trace.h>
#include <undo.h>


static char *cr[] =
{
	"%s version %v",
	"Copyright (C) 1990, 1991, 1992, 1993 Peter Miller.",
	"All rights reserved.",
	"",
	"The %s program comes with ABSOLUTELY NO WARRANTY;",
	"for details use the '%s -VERSion Warranty' command.",
	"The %s program is free software, and you are welcome to",
	"redistribute it under certain conditions;",
	"for details use the '%s -VERSion Redistribution' command.",
};

static char *au[] =
{
	"Peter Miller   UUCP     uunet!munnari!bmr.gov.au!pmiller",
	"/\\/\\*          Internet pmiller@bmr.gov.au",
};


static void interpret _((FILE *, char **, int, char *));

static void
interpret(fp, text, text_len, prefix)
	FILE	*fp;
	char	**text;
	int	text_len;
	char	*prefix;
{
	char	*progname;
	int	j;

	trace(("interpret()\n{\n"/*}*/));
	progname = option_get_progname();
	for (j = 0; j < text_len; ++j)
	{
		char	indent[10];
		char	*ip1;
		char	*ip2;
		char	*s;

		s = text[j];
		strcpy(indent, prefix);
		ip2 = indent + strlen(indent);
		ip1 = s;
		while (isspace(*ip1))
			*ip2++ = *ip1++;
		*ip2 = 0;
		if (ip1[0] == '%' && isupper(ip1[1]) && !ip1[2])
		{
			switch (ip1[1])
			{
			default:
				assert(0);
				break;

			case 'C':
				interpret(fp, cr, SIZEOF(cr), indent);
				break;

			case 'A':
				interpret(fp, au, SIZEOF(au), indent);
				break;
			}
			continue;
		}
		if (*prefix)
			fputs(prefix, fp);
		while (*s)
		{
			if (*s != '%')
				putc(*s, fp);
			else
			{
				switch (*++s)
				{
				default:
					assert(0);
					break;

				case '%':
					putc(*s, fp);
					break;

				case 's':
					fputs(progname, fp);
					break;

				case 'v':
					fputs(PATCHLEVEL, fp);
					break;
				}
			}
			++s;
		}
		fputc('\n', fp);
		if (ferror(fp))
			pager_error(fp);
	}
	trace((/*{*/"}\n"));
}


void
help(text, text_len, usage)
	char	**text;
	int	text_len;
	void	(*usage)_((void));
{
	FILE	*fp;

	/*
	 * collect the rest of thge command line,
	 * if necessary
	 */
	trace(("help(text = %08lX, text_len = %d, usage = %08lX)\n{\n"/*}*/,
		text, text_len, usage));
	if (usage)
	{
		arglex();
		while (arglex_token != arglex_token_eoln)
			generic_argument(usage);
	}

	/*
	 * paginate output if appropriate
	 */
	fp = pager_open();

	/*
	 * do what they asked
	 */
	interpret(fp, text, text_len, "");

	/*
	 * close the paginator
	 */
	pager_close(fp);
	trace((/*{*/"}\n"));
}


void
generic_argument(usage)
	void	(*usage)_((void));
{
	trace(("generic_argument()\n{\n"/*}*/));
	switch (arglex_token)
	{
	default:
		bad_argument(usage);
		/* NOTREACHED */

	case arglex_token_library:
		if (arglex() != arglex_token_string)
			usage();
		gonzo_library_append(arglex_value.alv_string);
		arglex();
		break;

	case arglex_token_page_length:
		if (arglex() != arglex_token_number)
			usage();
		option_set_page_length(arglex_value.alv_number);
		arglex();
		break;

	case arglex_token_page_width:
		if (arglex() != arglex_token_number)
			usage();
		option_set_page_width(arglex_value.alv_number);
		arglex();
		break;

	case arglex_token_terse:
		option_set_terse();
		arglex();
		break;

	case arglex_token_trace:
		if (arglex() != arglex_token_string)
			usage();
		for (;;)
		{
			trace_enable(arglex_value.alv_string);
			if (arglex() != arglex_token_string)
				break;
		}
#ifndef DEBUG
		error
		(
"Warning: the -TRace option is only effective when the %s program \
is compiled using the DEBUG define in the conf/main.h include file.",
			option_get_progname()
		);
#endif
		break;

	case arglex_token_verbose:
		option_set_verbose();
		arglex();
		break;
	}
	trace((/*{*/"}\n"));
}


void
bad_argument(usage)
	void	(*usage)_((void));
{
	trace(("bad_argument()\n{\n"/*}*/));
	switch (arglex_token)
	{
	case arglex_token_string:
		error("misplaced file name (\"%s\")", arglex_value.alv_string);
		break;

	case arglex_token_number:
		error("misplaced number (%s)", arglex_value.alv_string);
		break;

	case arglex_token_option:
		error("unknown \"%s\" option", arglex_value.alv_string);
		break;

	case arglex_token_eoln:
		error("command line too short");
		break;

	default:
		error("misplaced \"%s\" option", arglex_value.alv_string);
		break;
	}
	usage();
	trace((/*{*/"}\n"));
	quit(1);
	/* NOTREACHED */
}

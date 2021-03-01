/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1994, 1995 Peter Miller;
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
 * MANIFEST: functions to perform lexical analysis on aegis' data files
 */

#include <ctype.h>
#include <stdio.h>
#include <ac/stdlib.h>
#include <ac/stdarg.h>
#include <errno.h>

#include <error.h>
#include <lex.h>
#include <lex/file.h>
#include <lex/env.h>
#include <str.h>
#include <gram.gen.h>	/* must be after <str.h> */
#include <trace.h>


static	lex_input_ty	*input;
static	int		line_number;
static	int		line_number_start;
static	int		error_count;
extern	gram_STYPE	gram_lval;
static	char		buffer[1 << 14];


void
lex_open(s)
	char		*s;
{
	input = lex_input_file_open(s);
	line_number = 1;
	error_count = 0;
}


void
lex_open_env(s)
	char		*s;
{
	input = lex_input_env_open(s);
	line_number = 1;
	error_count = 0;
}


void
lex_close()
{
	if (error_count)
	{
		fatal
		(
			"%s: found %d fatal errors",
			input->method.name(input),
			error_count
		);
	}
	input->method.destruct(input);
	input = 0;
}


static int lex_getc _((void));

static int
lex_getc()
{
	int		c;

	c = input->method.get(input);
	switch (c)
	{
	case EOF:
		break;

	case '\n':
		++line_number;
		break;

	default:
		if (!isprint(c))
			gram_error("illegal '\\%o' character", c);
		break;

	case '\t':
	case '\f':
		break;
	}
	return c;
}


static void lex_getc_undo _((int));

static void
lex_getc_undo(c)
	int		c;
{
	input->method.unget(input, c);
	if (c == '\n')
		--line_number;
}


int
gram_lex()
{
	int		c;
	char		*cp;

	for (;;)
	{
		c = lex_getc();
		switch (c)
		{
		case ' ':
		case '\t':
		case '\f':
		case '\n':
			break;

		case '0':
			gram_lval.lv_integer = 0;
			c = lex_getc();
			if (c == 'x' || c == 'X')
			{
				int	ndigits;
				int	n;

				ndigits = 0;
				n = 0;
				for (;;)
				{
					++ndigits;
					c = lex_getc();
					switch (c)
					{
					case '0': case '1': case '2': case '3':
					case '4': case '5': case '6': case '7':
					case '8': case '9':
						n = 16 * n + c - '0';
						continue;
	
					case 'A': case 'B': case 'C': case 'D':
					case 'E': case 'F': 
						n = 16 * n + c - 'A' + 10;
						continue;
	
					case 'a': case 'b': case 'c': case 'd':
					case 'e': case 'f': 
						n = 16 * n + c - 'a' + 10;
						continue;
	
					default:
						--ndigits;
						lex_getc_undo(c);
						break;
					}
					break;
				}
				if (!ndigits)
				{
					gram_error("malformed hex constant");
					gram_lval.lv_integer = 0;
					trace(("%s: %d: INTEGER 0\n",
						input->method.name(input),
						line_number));
					return INTEGER;
				}
				gram_lval.lv_integer = n;
				trace(("%s: %d: INTEGER %ld\n",
					input->method.name(input),
					line_number, n));
				return INTEGER;
			}
			for (;;)
			{
				gram_lval.lv_integer =
					8 * gram_lval.lv_integer + c - '0';
				c = lex_getc();
				switch (c)
				{
				case '0': case '1': case '2': case '3':
				case '4': case '5': case '6': case '7':
					continue;

				default:
					lex_getc_undo(c);
					break;
				}
				break;
			}
			trace(("%s: %d: INTEGER %ld\n",
				input->method.name(input),
				line_number, gram_lval.lv_integer));
			return INTEGER;

		case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9': 
			gram_lval.lv_integer = 0;
			for (;;)
			{
				gram_lval.lv_integer =
					10 * gram_lval.lv_integer + c - '0';
				c = lex_getc();
				switch (c)
				{
				case '0': case '1': case '2': case '3':
				case '4': case '5': case '6': case '7':
				case '8': case '9': 
					continue;

				default:
					lex_getc_undo(c);
					break;
				}
				break;
			}
			trace(("%s: %d: INTEGER %ld\n",
				input->method.name(input), line_number,
				gram_lval.lv_integer));
			return INTEGER;

		case '"':
			line_number_start = line_number;
			cp = buffer;
			for (;;)
			{
				c = lex_getc();
				if (c == EOF)
				{
					str_eof:
					line_number = line_number_start;
					gram_error("end-of-file within string");
					break;
				}
				if (c == '\n')
				{
					line_number = line_number_start;
					gram_error("end-of-line within string");
					break;
				}
				if (c == '"')
					break;
				if (c == '\\')
				{
					c = lex_getc();
					switch (c)
					{
					default:
						gram_error("unknown '\\%c' escape", c);
						break;

					case '\n':
						break;

					case EOF:
						goto str_eof;

					case 'b':
						*cp++ = '\b';
						break;

					case 'n':
						*cp++ = '\n';
						break;

					case 'r':
						*cp++ = '\r';
						break;

					case 't':
						*cp++ = '\t';
						break;

					case 'f':
						*cp++ = '\f';
						break;

					case '"':
					case '\\':
						*cp++ = c;
						break;

					case '0': case '1': case '2': case '3':
					case '4': case '5': case '6': case '7':
						{
							int			 n;
							int			 v;

							v = 0;
							for (n = 0; n < 3; ++n)
							{
								v = v * 8 + c - '0';
								c = lex_getc();
								switch (c)
								{
								case '0':
								case '1':
								case '2':
								case '3':
								case '4':
								case '5':
								case '6':
								case '7':
									continue;

								default:
									lex_getc_undo(c);
									break;
								}
								break;
							}
							*cp++ = v;
						}
						break;
					}
				}
				else
					*cp++ = c;
			}
			*cp = 0;
			gram_lval.lv_string = str_from_c(buffer);
			trace(("%s: %d: STRING \"%s\"\n",
				input->method.name(input), line_number,
				buffer));
			return STRING;

		case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
		case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
		case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
		case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
		case 'Y': case 'Z': case '_':
		case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
		case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
		case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
		case 's': case 't': case 'u': case 'v': case 'w': case 'x':
		case 'y': case 'z': 
			cp = buffer;
			for (;;)
			{
				*cp++ = c;
				c = lex_getc();
				switch (c)
				{
				case '0': case '1': case '2': case '3':
				case '4': case '5': case '6': case '7':
				case '8': case '9': 
				case 'A': case 'B': case 'C': case 'D':
				case 'E': case 'F': case 'G': case 'H':
				case 'I': case 'J': case 'K': case 'L':
				case 'M': case 'N': case 'O': case 'P':
				case 'Q': case 'R': case 'S': case 'T':
				case 'U': case 'V': case 'W': case 'X':
				case 'Y': case 'Z': case '_':
				case 'a': case 'b': case 'c': case 'd':
				case 'e': case 'f': case 'g': case 'h':
				case 'i': case 'j': case 'k': case 'l':
				case 'm': case 'n': case 'o': case 'p':
				case 'q': case 'r': case 's': case 't':
				case 'u': case 'v': case 'w': case 'x':
				case 'y': case 'z': 
					continue;

				default:
					lex_getc_undo(c);
					break;
				}
				*cp = 0;
				break;
			}
			gram_lval.lv_string = str_from_c(buffer);
			trace(("%s: %d: NAME \"%s\"\n",
				input->method.name(input), line_number,
				buffer));
			return NAME;

		case '/':
			line_number_start = line_number;
			c = lex_getc();
			if (c != '*')
			{
				lex_getc_undo(c);
				trace(("%s: %d: '/'\n",
					input->method.name(input),
					line_number));
				return '/';
			}
			for (;;)
			{
				for (;;)
				{
					c = lex_getc();
					if (c == EOF)
					{
						bad_comment:
						line_number = line_number_start;
						gram_error("end-of-file within comment");
						quit(1);
					}
					if (c == '*')
						break;
				}
				for (;;)
				{
					c = lex_getc();
					if (c == EOF)
						goto bad_comment;
					if (c != '*')
						break;
				}
				if (c == '/')
					break;
			}
			break;

		case EOF:
			trace(("%s: %d: end of file\n",
				input->method.name(input), line_number));
			return 0;

		default:
			trace(("%s: %d: '%c'\n", input->method.name(input),
				line_number, c));
			return c;
		}
	}
}


void
gram_error(s sva_last)
	char		*s;
	sva_last_decl
{
	va_list         ap;

	sva_init(ap, s);
	vsprintf(buffer, s, ap);
	va_end(ap);
	error("%s: %d: %s", input->method.name(input), line_number, buffer);
	if (++error_count >= 20)
		fatal("%s: too many errors, bye!", input->method.name(input));
}

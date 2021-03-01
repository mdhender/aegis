/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1998 Peter Miller;
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
 * MANIFEST: functions to perform lexical analysis on aegis' data files
 */

#include <ac/ctype.h>
#include <stdio.h>
#include <ac/stdlib.h>
#include <ac/stdarg.h>
#include <errno.h>

#include <error.h>
#include <lex.h>
#include <lex/file.h>
#include <lex/env.h>
#include <mem.h>
#include <str.h>
#include <gram.gen.h>	/* must be after <str.h> */
#include <sub.h>
#include <trace.h>
#include <zero.h>


static	lex_input_ty	*input;
static	int		line_number;
static	int		line_number_start;
static	int		error_count;
extern	gram_STYPE	gram_lval;
static	char		*buffer_xxx;
static	size_t		buffer_pos;
static	size_t		buffer_max;


static void buffer_rewind _((void));

static void
buffer_rewind()
{
	buffer_pos = 0;
}


static void buffer_putc _((int));

static void
buffer_putc(c)
	int	c;
{
	if (buffer_pos >= buffer_max)
	{
		buffer_max = buffer_max * 2 + 32;
		buffer_xxx = mem_change_size(buffer_xxx, buffer_max);
	}
	buffer_xxx[buffer_pos++] = c;
}


static string_ty *buffer_string _((void));

static string_ty *
buffer_string()
{
	return str_n_from_c(buffer_xxx, buffer_pos);
}


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
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "File_Name", "%s", input->method.name(input));
		sub_var_set(scp, "Number", "%d", error_count);
		sub_var_optional(scp, "Number");
		fatal_intl(scp, i18n("$filename: has errors"));
		/* NOTREACHED */
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
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "Name", "\\%o", c);
			lex_error(scp, i18n("illegal '$name' character"));
			sub_context_delete(scp);
		}
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
	sub_context_ty	*scp;
	int		c;

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
					gram_error(i18n("malformed hex constant"));
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
				switch (c)
				{
				case '0': case '1': case '2': case '3':
				case '4': case '5': case '6': case '7':
					gram_lval.lv_integer =
					     8 * gram_lval.lv_integer + c - '0';
					c = lex_getc();
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
			integer_return:
			trace(("%s: %d: INTEGER %ld\n",
				input->method.name(input), line_number,
				gram_lval.lv_integer));
			return INTEGER;

		case '"':
			line_number_start = line_number;
			buffer_rewind();
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
						scp = sub_context_new();
						sub_var_set(scp, "Name", "\\%c", c);
						lex_error(scp, i18n("unknown '$name' escape"));
						sub_context_delete(scp);
						break;

					case '\n':
						break;

					case EOF:
						goto str_eof;

					case 'b':
						buffer_putc('\b');
						break;

					case 'n':
						buffer_putc('\n');
						break;

					case 'r':
						buffer_putc('\r');
						break;

					case 't':
						buffer_putc('\t');
						break;

					case 'f':
						buffer_putc('\f');
						break;

					case '"':
					case '\\':
						buffer_putc(c);
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
							buffer_putc(v);
						}
						break;
					}
				}
				else
					buffer_putc(c);
			}
			gram_lval.lv_string = buffer_string();
			trace(("%s: %d: STRING \"%s\"\n",
				input->method.name(input), line_number,
				gram_lval.lv_string->str_text));
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
			buffer_rewind();
			for (;;)
			{
				buffer_putc(c);
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
				break;
			}
			if (buffer_pos == 4 && !memcmp(buffer_xxx, "ZERO", 4))
			{
				gram_lval.lv_integer = MAGIC_ZERO;
				goto integer_return;
			}
			gram_lval.lv_string = buffer_string();
			trace(("%s: %d: NAME \"%s\"\n",
				input->method.name(input), line_number,
				gram_lval.lv_string->str_text));
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
gram_error(s)
	char	*s;
{
	sub_context_ty	*scp;

	scp = sub_context_new();
	lex_error(scp, s);
	sub_context_delete(scp);
}


void
lex_error(scp, s)
	sub_context_ty	*scp;
	char		*s;
{
	string_ty	*msg;

	msg = subst_intl(scp, s);

	/* re-use substitution context */
	sub_var_set(scp, "Message", "%S", msg);
	sub_var_set(scp, "File_Name", "%s", input->method.name(input));
	sub_var_set(scp, "Number", "%d", line_number);
	error_intl(scp, i18n("$filename: $number: $message"));
	str_free(msg);
	if (++error_count >= 20)
	{
		/* re-use substitution context */
		sub_var_set(scp, "File_Name", "%s", input->method.name(input));
		fatal_intl(scp, i18n("$filename: too many errors"));
	}
}

/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1998, 1999 Peter Miller;
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
#include <ac/stdio.h>
#include <ac/stdlib.h>
#include <ac/stdarg.h>
#include <ac/errno.h>

#include <error.h>
#include <input/crlf.h>
#include <input/env.h>
#include <input/file.h>
#include <input/gunzip.h>
#include <lex.h>
#include <mem.h>
#include <str.h>
#include <stracc.h>
#include <gram.gen.h>	/* must be after <str.h> */
#include <sub.h>
#include <trace.h>
#include <zero.h>


static	input_ty	*input;
static	int		line_number;
static	int		line_number_start;
static	int		error_count;
extern	gram_STYPE	gram_lval;
static	stracc_t	buffer;


void
lex_open(s)
	const char	*s;
{
	input_ty	*fp;

	/*
	 * Open the underlying binary file.
	 */
	fp = input_file_open(s);

	/*
	 * Decompress the input stream.  If it *isn't* compressed, this
	 * incurs NO overhead, because the gunzip code gets itself out
	 * of the way, and returns the original fp.
	 */
	fp = input_gunzip(fp);

	/*
	 * Get rid of CRLF sequences in the input.
	 * This happens, for instance, when the file is created on
	 * windows nt, but used on Unix.
	 */
	fp = input_crlf(fp, 1);

	/*
	 * Now that we've completely messed with your mind, parse the
	 * resulting input stream.
	 */
	lex_open_input(fp);
}


void
lex_open_env(s)
	const char	*s;
{
	lex_open_input(input_env_open(s));
}


void
lex_open_input(ifp)
	input_ty	*ifp;
{
	input = ifp;
	line_number = 1;
}


void
lex_close()
{
	if (error_count)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "File_Name", "%s", input_name(input));
		sub_var_set(scp, "Number", "%d", error_count);
		sub_var_optional(scp, "Number");
		fatal_intl(scp, i18n("$filename: has errors"));
		/* NOTREACHED */
	}
	input_delete(input);
	input = 0;
}


static int lex_getc _((void));

static int
lex_getc()
{
	int		c;

	c = input_getc(input);
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
	input_ungetc(input, c);
	if (c == '\n')
		--line_number;
}


int
gram_lex()
{
	sub_context_ty	*scp;
	int		c;
	int		ndigits;

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
			stracc_open(&buffer);
			stracc_char(&buffer, '0');
			c = lex_getc();
			if (c == 'x' || c == 'X')
			{
				stracc_char(&buffer, c);
				ndigits = 0;
				for (;;)
				{
					c = lex_getc();
					switch (c)
					{
					case '0': case '1': case '2': case '3':
					case '4': case '5': case '6': case '7':
					case '8': case '9':
					case 'A': case 'B': case 'C': case 'D':
					case 'E': case 'F': 
					case 'a': case 'b': case 'c': case 'd':
					case 'e': case 'f': 
						++ndigits;
						stracc_char(&buffer, c);
						continue;
	
					default:
						break;
					}
					break;
				}
				if (!ndigits)
				{
					gram_error(i18n("malformed hex constant"));
					gram_lval.lv_integer = 0;
					goto integer_return;
				}
				lex_getc_undo(c);
				stracc_char(&buffer, ' ');
				gram_lval.lv_integer =
					strtoul(buffer.buffer, (char **)0,  16);
				goto integer_return;
			}
			if (c == '.')
			{
				stracc_char(&buffer, c);
				goto fraction;
			}
			if (c == 'e' || c == 'E')
				goto exponent;
			for (;;)
			{
				switch (c)
				{
				case '0': case '1': case '2': case '3':
				case '4': case '5': case '6': case '7':
					stracc_char(&buffer, c);
					c = lex_getc();
					continue;

				default:
					break;
				}
				break;
			}
			lex_getc_undo(c);
			stracc_char(&buffer, ' ');
			gram_lval.lv_integer =
				strtoul(buffer.buffer, (char **)0, 8);
			goto integer_return;

		case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9': 
			stracc_open(&buffer);
			for (;;)
			{
				stracc_char(&buffer, c);
				c = lex_getc();
				if (!isdigit(c))
					break;
			}
			if (c == '.')
			{
				stracc_char(&buffer, c);
				goto fraction;
			}
			if (c == 'e' || c == 'E')
				goto exponent;
			lex_getc_undo(c);
			stracc_char(&buffer, ' ');
			gram_lval.lv_integer =
				strtoul(buffer.buffer, (char **)0, 10);
			assert(gram_lval.lv_integer >= 0);
			integer_return:
			trace(("%s: %d: INTEGER %ld\n",
				input_name(input), line_number,
				gram_lval.lv_integer));
			return INTEGER;

		case '.':
			c = lex_getc();
			if (!isdigit(c))
			{
				lex_getc_undo(c);
				return '.';
			}
			stracc_open(&buffer);
			stracc_char(&buffer, '0');
			stracc_char(&buffer, '.');
			stracc_char(&buffer, c);
			fraction:
			for (;;)
			{
				c = lex_getc();
				if (!isdigit(c))
					break;
				stracc_char(&buffer, c);
			}
			if (c == 'e' || c == 'E')
			{
				exponent:
				stracc_char(&buffer, c);
				c = lex_getc();
				if (c == '+' || c == '-')
				{
					stracc_char(&buffer, c);
					c = lex_getc();
				}
				ndigits = 0;
				for (;;)
				{
					c = lex_getc();
					if (!isdigit(c))
						break;
					++ndigits;
					stracc_char(&buffer, c);
				}
				if (!ndigits)
				{
					gram_error(i18n("malformed exponent"));
					gram_lval.lv_real = 0;
					trace(("%s: %d: REAL 0\n",
						input_name(input),
						line_number));
					return REAL;
				}
			}
			lex_getc_undo(c);
			stracc_char(&buffer, 0);
			gram_lval.lv_real = atof(buffer.buffer);
			trace(("%s: %d: REAL %g\n",
				input_name(input), line_number,
				gram_lval.lv_real));
			return REAL;

		case '"':
			line_number_start = line_number;
			stracc_open(&buffer);
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
						stracc_char(&buffer, '\b');
						break;

					case 'n':
						stracc_char(&buffer, '\n');
						break;

					case 'r':
						stracc_char(&buffer, '\r');
						break;

					case 't':
						stracc_char(&buffer, '\t');
						break;

					case 'f':
						stracc_char(&buffer, '\f');
						break;

					case '"':
					case '\\':
						stracc_char(&buffer, c);
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
							stracc_char(&buffer, v);
						}
						break;
					}
				}
				else
					stracc_char(&buffer, c);
			}
			gram_lval.lv_string = stracc_close(&buffer);
			trace(("%s: %d: STRING \"%s\"\n",
				input_name(input), line_number,
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
			stracc_open(&buffer);
			for (;;)
			{
				stracc_char(&buffer, c);
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
			if (buffer.length == 4 && !memcmp(buffer.buffer, "ZERO", 4))
			{
				gram_lval.lv_integer = MAGIC_ZERO;
				goto integer_return;
			}
			gram_lval.lv_string = stracc_close(&buffer);
			trace(("%s: %d: NAME \"%s\"\n",
				input_name(input), line_number,
				gram_lval.lv_string->str_text));
			return NAME;

		case '/':
			line_number_start = line_number;
			c = lex_getc();
			if (c != '*')
			{
				lex_getc_undo(c);
				trace(("%s: %d: '/'\n",
					input_name(input),
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
				input_name(input), line_number));
			return 0;

		default:
			trace(("%s: %d: '%c'\n", input_name(input),
				line_number, c));
			return c;
		}
	}
}


void
gram_error(s)
	const char	*s;
{
	sub_context_ty	*scp;

	scp = sub_context_new();
	lex_error(scp, s);
	sub_context_delete(scp);
}


void
lex_error(scp, s)
	sub_context_ty	*scp;
	const char	*s;
{
	string_ty	*msg;

	msg = subst_intl(scp, s);

	/* re-use substitution context */
	sub_var_set(scp, "Message", "%S", msg);
	sub_var_set(scp, "File_Name", "%s", input_name(input));
	sub_var_set(scp, "Number", "%d", line_number);
	error_intl(scp, i18n("$filename: $number: $message"));
	str_free(msg);
	if (++error_count >= 20)
	{
		/* re-use substitution context */
		sub_var_set(scp, "File_Name", "%s", input_name(input));
		fatal_intl(scp, i18n("$filename: too many errors"));
	}
}

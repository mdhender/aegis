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
 * MANIFEST: functions to perform lexical analysis on report scripts
 */

#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <ac/stdlib.h>
#include <ac/stdarg.h>

#include <aer/lex.h>
#include <aer/value/integer.h>
#include <aer/value/real.h>
#include <aer/value/string.h>
#include <error.h>
#include <glue.h>
#include <mem.h>
#include <str.h>
#include <symtab.h>
#include <aer/report.gen.h>	/* must be last */


static FILE	*fp;
static int	line_number;
static int	line_number_start;
static string_ty *file_name;
static int	error_count;
extern report_STYPE report_lval;
static char	*sa_buf;
static size_t	sa_buf_len;
static size_t	sa_buf_max;
static symtab_ty *stp;


typedef struct table_ty table_ty;
struct table_ty
{
	char		*name;
	int		token;
};


static table_ty table[] =
{
	{ "!",		NOT_LOGICAL,	},
	{ "!=",		NE,		},
	{ "!~",		NMATCH,		},
	{ "#",		JUNK,		},
	{ "##",		JOIN,		},
	{ "##=",	ASSIGN_JOIN,	},
	{ "%",		MOD,		},
	{ "%=",		ASSIGN_MOD,	},
	{ "&",		AND_BIT,	},
	{ "&&",		AND_LOGICAL,	},
	{ "&=",		ASSIGN_AND_BIT,	},
	{ "(",		LP,		},
	{ ")",		RP,		},
	{ "*",		MUL,		},
	{ "**",		POWER,		},
	{ "**=",	ASSIGN_POWER,	},
	{ "*=",		ASSIGN_MUL,	},
	{ "+",		PLUS,		},
	{ "++",		INCR,		},
	{ "+=",		ASSIGN_PLUS,	},
	{ ",",		COMMA,		},
	{ "-",		MINUS,		},
	{ "--",		DECR,		},
	{ "-=",		ASSIGN_MINUS,	},
	{ ".",		DOT,		},
	{ "/",		DIV,		},
	{ "/=",		ASSIGN_DIV,	},
	{ ":",		COLON,		},
	{ ";",		SEMICOLON,	},
	{ "<",		LT,		},
	{ "<<",		SHIFT_LEFT,	},
	{ "<<=",	ASSIGN_SHIFT_LEFT,	},
	{ "<=",		LE,		},
	{ "=",		ASSIGN,		},
	{ "==",		EQ,		},
	{ ">",		GT,		},
	{ ">=",		GE,		},
	{ ">>",		SHIFT_RIGHT,	},
	{ ">>=",	ASSIGN_SHIFT_RIGHT,	},
	{ "?",		QUESTION,	},
	{ "[",		LBB,		},
	{ "]",		RBB,		},
	{ "^",		XOR_BIT,	},
	{ "^=",		ASSIGN_XOR_BIT,	},
	{ "auto",	AUTO,		},
	{ "break",	BREAK,		},
	{ "case",	CASE,		},
	{ "continue",	CONTINUE,	},
	{ "default",	DEFAULT,	},
	{ "do",		DO,		},
	{ "else",	ELSE,		},
	{ "for",	FOR,		},
	{ "function",	FUNCTION,	},
	{ "global",	GLOBAL,		},
	{ "goto",	GOTO,		},
	{ "if",		IF,		},
	{ "in",		IN,		},
	{ "return",	RETURN,		},
	{ "switch",	SWITCH,		},
	{ "while",	WHILE,		},
	{ "{",		LB,		},
	{ "|",		OR_BIT,		},
	{ "|=",		ASSIGN_OR_BIT,	},
	{ "||",		OR_LOGICAL,	},
	{ "}",		RB,		},
	{ "~",		NOT_BIT,	},
	{ "~~",		MATCH,		},
};


static void reserved_init _((void));

static void
reserved_init()
{
	table_ty	*tp;
	string_ty	*s;

	if (!stp)
	{
		stp = symtab_alloc(SIZEOF(table));
		for (tp = table; tp < ENDOF(table); ++tp)
		{
			s = str_from_c(tp->name);
			symtab_assign(stp, s, &tp->token);
			str_free(s);
		}
	}
}


static int reserved _((string_ty *));

static int
reserved(name)
	string_ty	*name;
{
	int		*data;

	assert(stp);
	data = symtab_query(stp, name);
	return (data ? *data : 0);
}


void
rpt_lex_open(s)
	string_ty	*s;
{
	reserved_init();
	fp = glue_fopen(s->str_text, "r");
	if (!fp)
		nfatal("open \"%S\"", s);
	file_name = str_copy(s);
	line_number = 1;
	error_count = 0;
}


void
rpt_lex_close()
{
	glue_fclose(fp);
	fp = 0;
	if (error_count)
	{
		fatal
		(
			"%S: found %d fatal error%s",
			file_name,
			error_count,
			(error_count == 1 ? "" : "s")
		);
	}
	str_free(file_name);
	file_name = 0;
}


static int lex_getc2 _((void));

static int
lex_getc2()
{
	int		c;

	c = glue_fgetc(fp);
	switch (c)
	{
	case EOF:
		if (glue_ferror(fp))
		{
			nerror("%s", file_name);
			++error_count;
		}
		break;

	case '\n':
		++line_number;
		break;

	default:
		if (!isprint(c))
			report_error("illegal '\\%o' character", c);
		break;

	case ' ':
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
	switch (c)
	{
	case EOF:
		break;

	case '\n':
		--line_number;
		/* fall through... */

	default:
		glue_ungetc(c, fp);
		break;
	}
}


static int lex_getc _((void));

static int
lex_getc()
{
	int		c;

	for (;;)
	{
		c = lex_getc2();
		if (c == '\\')
		{
			c = lex_getc2();
			if (c == '\n')
				continue;
			lex_getc_undo(c);
			c = '\\';
		}
		break;
	}
	return c;
}


static void sa_open _((void));

static void
sa_open()
{
	sa_buf_len = 0;
}


static string_ty *sa_close _((void));

static string_ty *
sa_close()
{
	return str_n_from_c(sa_buf, sa_buf_len);
}


static void sa_char _((int));

static void
sa_char(c)
	int		c;
{
	if (sa_buf_len >= sa_buf_max)
	{
		sa_buf_max = sa_buf_max * 2 + 16;
		sa_buf = mem_change_size(sa_buf, sa_buf_max);
	}
	sa_buf[sa_buf_len++] = c;
}


int
report_lex()
{
	int		c;
	long		n;
	string_ty	*s;
	int		term;
	int		tok;

	for (;;)
	{
		c = lex_getc();
		switch (c)
		{
		case EOF:
			return 0;

		case ' ':
		case '\t':
		case '\f':
		case '\n':
			break;

		case '0':
			n = 0;
			c = lex_getc();
			if (c == '.')
			{
				sa_open();
				sa_char('0');
				goto fraction_part;
			}
			if (c == 'x' || c == 'X')
			{
				int	ndigits;

				ndigits = 0;
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
					report_error("malformed hex constant");
					n = 0;
				}
				report_lval.lv_value = rpt_value_integer(n);
				return CONSTANT;
			}
			for (;;)
			{
				switch (c)
				{
				case '0': case '1': case '2': case '3':
				case '4': case '5': case '6': case '7':
					n = 8 * n + c - '0';
					c = lex_getc();
					continue;

				default:
					break;
				}
				break;
			}
			lex_getc_undo(c);
			report_lval.lv_value = rpt_value_integer(n);
			return CONSTANT;

		case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9': 
			sa_open();
			for (;;)
			{
				sa_char(c);
				c = lex_getc();
				if (!isdigit(c))
					break;
			}
			if (c != '.' && c != 'e' && c != 'E')
			{
				lex_getc_undo(c);
				s = sa_close();
				report_lval.lv_value =
					rpt_value_integer(atol(s->str_text));
				str_free(s);
				return CONSTANT;
			}
			if (c == '.')
			{
				fraction_part:
				for (;;)
				{
					sa_char(c);
					c = lex_getc();
					if (!isdigit(c))
						break;
				}
			}
			if (c == 'e' || c == 'E')
			{
				sa_char(c);
				c = lex_getc();
				if (c == '+' || c == '-')
				{
					sa_char(c);
					c = lex_getc();
				}
				if (!isdigit(c))
				{
					sa_char('0');
					report_error("malformed exponent");
				}
				else
				{
					for (;;)
					{
						sa_char(c);
						c = lex_getc();
						if (!isdigit(c))
							break;
					}
				}
			}
			lex_getc_undo(c);
			s = sa_close();
			report_lval.lv_value =
				rpt_value_real(atof(s->str_text));
			str_free(s);
			return CONSTANT;

		case '"':
		case '\'':
			term = c;
			line_number_start = line_number;
			sa_open();
			for (;;)
			{
				c = lex_getc();
				if (c == EOF)
				{
					str_eof:
					line_number = line_number_start;
					report_error("end-of-file within string");
					break;
				}
				if (c == '\n')
				{
					line_number = line_number_start;
					report_error("end-of-line within string");
					break;
				}
				if (c == term)
					break;
				if (c == '\\')
				{
					c = lex_getc();
					switch (c)
					{
					default:
						report_error("unknown '\\%c' escape", c);
						break;

					case EOF:
						goto str_eof;

					case 'b':
						sa_char('\b');
						break;

					case 'n':
						sa_char('\n');
						break;

					case 'r':
						sa_char('\r');
						break;

					case 't':
						sa_char('\t');
						break;

					case 'f':
						sa_char('\f');
						break;

					case '"':
					case '\'':
					case '\\':
						sa_char(c);
						break;

					case '0': case '1': case '2': case '3':
					case '4': case '5': case '6': case '7':
						{
							int		 n;
							int		 v;

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
							sa_char(v);
						}
						break;
					}
				}
				else
					sa_char(c);
			}
			s = sa_close();
			report_lval.lv_value = rpt_value_string(s);
			str_free(s);
			return CONSTANT;

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
			sa_open();
			for (;;)
			{
				sa_char(c);
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
			s = sa_close();
			tok = reserved(s);
			if (tok)
			{
				str_free(s);
				return tok;
			}
			report_lval.lv_string = s;
			return NAME;

		case '/':
			line_number_start = line_number;
			c = lex_getc();
			if (c == '/')
			{
				eoln_comment:
				for (;;)
				{
					c = lex_getc();
					if (c == EOF)
						goto bad_comment;
					if (c == '\n')
						break;
				}
				break;
			}
			if (c != '*')
			{
				lex_getc_undo(c);
				c = '/';
				goto normal;
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
						report_error("end-of-file within comment");
						return 0;
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

		case '#':
			c = lex_getc();
			if (c == '!')
				goto eoln_comment;
			lex_getc_undo(c);
			c = '#';
			goto normal;

		case '.':
			c = lex_getc();
			lex_getc_undo(c);
			if (!isdigit(c))
			{
				c = '.';
				goto normal;
			}
			c = '.';
			sa_open();
			sa_char('0');
			goto fraction_part;

		default:
			normal:
			sa_open();
			for (;;)
			{
				sa_char(c);
				s = str_n_from_c(sa_buf, sa_buf_len);
				if (!symtab_query(stp, s))
				{
					if (sa_buf_len > 1)
					{
						sa_buf_len--;
						lex_getc_undo(c);
					}
					str_free(s);
					break;
				}
				str_free(s);
				c = lex_getc();
			}
			s = sa_close();
			tok = reserved(s);
			str_free(s);
			return (tok ? tok : JUNK);
		}
	}
}


void
report_error(fmt sva_last)
	char		*fmt;
	sva_last_decl
{
	va_list         ap;
	string_ty	*buffer;

	sva_init(ap, fmt);
	buffer = str_vformat(fmt, ap);
	va_end(ap);
	error("%S: %ld: %S", file_name, line_number, buffer);
	str_free(buffer);
	if (++error_count >= 20)
		fatal("%S: too many fatal errors", file_name);
}


rpt_lex_pos_ty *
rpt_lex_pos_get()
{
	rpt_lex_pos_ty	*result;

	result = mem_alloc(sizeof(rpt_lex_pos_ty));
	result->reference_count = 1;
	result->file_name = str_copy(file_name);
	result->line_number = line_number;
	return result;
}


void
rpt_lex_pos_free(p)
	rpt_lex_pos_ty	*p;
{
	p->reference_count--;
	if (p->reference_count <= 0)
	{
		str_free(p->file_name);
		mem_free(p);
	}
}


void
rpt_lex_error(p, fmt sva_last)
	rpt_lex_pos_ty	*p;
	char		*fmt;
	sva_last_decl
{
	va_list		ap;
	string_ty	*s;

	sva_init(ap, fmt);
	s = str_vformat(fmt, ap);
	va_end(ap);

	assert(p);
	if (!p)
		p = rpt_lex_pos_get();
	error("%S: %ld: %S", p->file_name, p->line_number, s);
	str_free(s);

	if (!fp)
		fatal("report aborted");

	++error_count;
	if (error_count >= 20)
		fatal("%S: too many fatal errors", file_name);
}


rpt_lex_pos_ty *
rpt_lex_pos_union(p1, p2)
	rpt_lex_pos_ty	*p1;
	rpt_lex_pos_ty	*p2;
{
	rpt_lex_pos_ty	*result;

	assert(p1);
	if (!p1)
		p1 = rpt_lex_pos_get();
	assert(p2);
	if (!p2)
		p2 = rpt_lex_pos_get();
	if (p1->line_number < p2->line_number)
		result = rpt_lex_pos_copy(p1);
	else
		result = rpt_lex_pos_copy(p2);
	return result;
}


rpt_lex_pos_ty *
rpt_lex_pos_copy(p)
	rpt_lex_pos_ty	*p;
{
	p->reference_count++;
	return p;
}

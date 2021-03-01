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
 * MANIFEST: functions to perform lexical analysis on report scripts
 */

#include <ac/ctype.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <ac/stdlib.h>

#include <aer/lex.h>
#include <aer/pos.h>
#include <aer/value/integer.h>
#include <aer/value/real.h>
#include <aer/value/string.h>
#include <error.h>
#include <glue.h>
#include <mem.h>
#include <sub.h>
#include <symtab.h>
#include <aer/report.gen.h>	/* must be last */


static FILE	*fp;
static int	line_number;
static int	line_number_start;
static string_ty *file_name;
static int	error_count;
extern aer_report_STYPE aer_report_lval;
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
	{ "catch",	CATCH,		},
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
	{ "throw",	THROW,		},
	{ "try",	TRY,		},
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
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name", "%S", s);
		fatal_intl(scp, i18n("open $filename: $errno"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}
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
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "File_Name", "%S", file_name);
		sub_var_set(scp, "Number", "%d", error_count);
		sub_var_optional(scp, "Number");
		fatal_intl(scp, i18n("$filename: has errors"));
		/* NOTREACHED */
		sub_context_delete(scp);
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
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_errno_set(scp);
			sub_var_set(scp, "File_Name", "%S", file_name);
			error_intl(scp, i18n("read $filename: $errno"));
			sub_context_delete(scp);
			++error_count;
		}
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
			aer_lex_error(scp, 0, i18n("illegal '$name' character"));
			sub_context_delete(scp);
		}
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
aer_report_lex()
{
	sub_context_ty	*scp;
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
					aer_report_error(i18n("malformed hex constant"));
					n = 0;
				}
				aer_report_lval.lv_value = rpt_value_integer(n);
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
			aer_report_lval.lv_value = rpt_value_integer(n);
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
				aer_report_lval.lv_value =
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
					aer_report_error(i18n("malformed exponent"));
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
			aer_report_lval.lv_value =
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
					aer_report_error(i18n("end-of-file within string"));
					break;
				}
				if (c == '\n')
				{
					line_number = line_number_start;
					aer_report_error(i18n("end-of-line within string"));
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
						scp = sub_context_new();
						sub_var_set(scp, "Name", "\\%c", c);
						aer_lex_error(scp, 0, i18n("unknown '$name' escape"));
						sub_context_delete(scp);
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
							int		 nc;
							int		 v;

							v = 0;
							for (nc = 0; nc < 3; ++nc)
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
			aer_report_lval.lv_value = rpt_value_string(s);
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
			aer_report_lval.lv_string = s;
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
						aer_report_error(i18n("end-of-file within comment"));
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
aer_report_error(fmt)
	char		*fmt;
{
	aer_lex_error(0, 0, fmt);
}


rpt_pos_ty *
rpt_lex_pos_get()
{
	static rpt_pos_ty *curpos;

	if (curpos)
	{
		if
		(
			str_equal(curpos->file_name, file_name)
		&&
			curpos->line_number1 == line_number
		)
			return rpt_pos_copy(curpos);
		rpt_pos_free(curpos);
		curpos = 0;
	}
	curpos = rpt_pos_alloc(file_name, line_number);
	return rpt_pos_copy(curpos);
}


void
aer_lex_error(scp, p, fmt)
	sub_context_ty	*scp;
	rpt_pos_ty	*p;
	char		*fmt;
{
	int		need_to_delete;

	if (!scp)
	{
		scp = sub_context_new();
		need_to_delete = 1;
	}
	else
		need_to_delete = 0;

	if (!p)
		p = rpt_lex_pos_get();
	rpt_pos_error(scp, p, fmt);

	if (!fp)
		fatal_intl(0, i18n("report aborted"));

	++error_count;
	if (error_count >= 20)
	{
		assert(file_name);
		sub_var_set(scp, "File_Name", "%S", file_name);
		fatal_intl(scp, i18n("$filename: too many errors"));
		/* NOTREACHED */
	}

	if (need_to_delete)
		sub_context_delete(scp);
}


void
rpt_lex_error(p, fmt)
	rpt_pos_ty	*p;
	char		*fmt;
{
	rpt_pos_error(0, 0, fmt);

	if (!fp)
		fatal_intl(0, i18n("report aborted"));

	++error_count;
	if (error_count >= 20)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		assert(file_name);
		sub_var_set(scp, "File_Name", "%S", file_name);
		fatal_intl(scp, i18n("$filename: too many errors"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}
}


/*
 * These are faked, so that the messages in the generated files
 * will also be translated correctly.
 */
#if 0
static void
fake()
{
	i18n("parse error"); /* bison */
	i18n("parse error; also virtual memory exceeded"); /* bison */
	i18n("parser stack overflow"); /* bison */
	i18n("syntax error"); /* yacc */
	i18n("yacc stack overflow"); /* yacc */
}
#endif

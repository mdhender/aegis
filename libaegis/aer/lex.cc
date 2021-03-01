//
//	aegis - project change supervisor
//	Copyright (C) 1991-1996, 1998, 1999, 2001-2004 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to perform lexical analysis on report scripts
//

#include <ac/ctype.h>
#include <ac/errno.h>
#include <ac/math.h>
#include <ac/stdlib.h>

#include <aer/lex.h>
#include <aer/pos.h>
#include <aer/value/integer.h>
#include <aer/value/real.h>
#include <aer/value/string.h>
#include <error.h>
#include <input/file_text.h>
#include <mem.h>
#include <stracc.h>
#include <sub.h>
#include <symtab.h>
#include <aer/report.gen.h> // must be last


static input_ty *ip;
static int	error_count;
extern aer_report_STYPE aer_report_lval;
static stracc_t buffer;
static symtab_ty *stp;


typedef struct table_ty table_ty;
struct table_ty
{
    const char      *name;
    int		    token;
};


static table_ty table[] =
{
    {"!", NOT_LOGICAL, },
    {"!=", NE, },
    {"!~", NMATCH, },
    {"#", JUNK, },
    {"##", JOIN, },
    {"##=", ASSIGN_JOIN, },
    {"%", MOD, },
    {"%=", ASSIGN_MOD, },
    {"&", AND_BIT, },
    {"&&", AND_LOGICAL, },
    {"&=", ASSIGN_AND_BIT, },
    {"(", LP, },
    {")", RP, },
    {"*", MUL, },
    {"**", POWER, },
    {"**=", ASSIGN_POWER, },
    {"*=", ASSIGN_MUL, },
    {"+", PLUS, },
    {"++", INCR, },
    {"+=", ASSIGN_PLUS, },
    {",", COMMA, },
    {"-", MINUS, },
    {"--", DECR, },
    {"-=", ASSIGN_MINUS, },
    {".", DOT, },
    {"/", DIV, },
    {"/=", ASSIGN_DIV, },
    {":", COLON, },
    {";", SEMICOLON, },
    {"<", LT, },
    {"<<", SHIFT_LEFT, },
    {"<<=", ASSIGN_SHIFT_LEFT, },
    {"<=", LE, },
    {"=", ASSIGN, },
    {"==", EQ, },
    {">", GT, },
    {">=", GE, },
    {">>", SHIFT_RIGHT, },
    {">>=", ASSIGN_SHIFT_RIGHT, },
    {"?", QUESTION, },
    {"[", LBB, },
    {"]", RBB, },
    {"^", XOR_BIT, },
    {"^=", ASSIGN_XOR_BIT, },
    {"auto", AUTO, },
    {"break", BREAK, },
    {"case", CASE, },
    {"catch", CATCH, },
    {"continue", CONTINUE, },
    {"default", DEFAULT, },
    {"do", DO, },
    {"else", ELSE, },
    {"for", FOR, },
    {"function", FUNCTION, },
    {"global", GLOBAL, },
    {"goto", GOTO, },
    {"if", IF, },
    {"in", IN, },
    {"return", RETURN, },
    {"switch", SWITCH, },
    {"throw", THROW, },
    {"try", TRY, },
    {"while", WHILE, },
    {"{", LB, },
    {"|", OR_BIT, },
    {"|=", ASSIGN_OR_BIT, },
    {"||", OR_LOGICAL, },
    {"}", RB, },
    {"~", NOT_BIT, },
    {"~~", MATCH, },
};


static void
reserved_init(void)
{
    table_ty        *tp;
    string_ty	    *s;

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


static int
reserved(string_ty *name)
{
    const int       *data;

    assert(stp);
    data = (int *)symtab_query(stp, name);
    return (data ? *data : 0);
}


void
rpt_lex_open(string_ty *s)
{
    reserved_init();
    ip = input_file_text_open(s);
    input_file_text_escaped_newline(ip);
    error_count = 0;
}


void
rpt_lex_close(void)
{
    if (error_count)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_string(scp, "File_Name", input_name(ip));
	sub_var_set_long(scp, "Number", error_count);
	sub_var_optional(scp, "Number");
	fatal_intl(scp, i18n("$filename: has errors"));
	// NOTREACHED
	sub_context_delete(scp);
    }
    input_delete(ip);
    ip = 0;
}


#define lex_getc_undo(c) \
    ((c) >= 0 ? input_ungetc(ip, (c)) : (void)0)


int
aer_report_lex(void)
{
    sub_context_ty  *scp;
    int		    c;
    long	    n;
    string_ty	    *s;
    int		    term;
    int		    tok;

    for (;;)
    {
	c = input_getc(ip);
	switch (c)
	{
	case -1:
	    return 0;

	case ' ':
	case '\t':
	case '\f':
	case '\n':
	    break;

	case '0':
	    n = 0;
	    c = input_getc(ip);
	    if (c == '.')
	    {
		stracc_open(&buffer);
		stracc_char(&buffer, '0');
		goto fraction_part;
	    }
	    if (c == 'x' || c == 'X')
	    {
		int		ndigits;

		ndigits = 0;
		for (;;)
		{
		    ++ndigits;
		    c = input_getc(ip);
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
		    case '8':
		    case '9':
			n = 16 * n + c - '0';
			continue;

		    case 'A':
		    case 'B':
		    case 'C':
		    case 'D':
		    case 'E':
		    case 'F':
			n = 16 * n + c - 'A' + 10;
			continue;

		    case 'a':
		    case 'b':
		    case 'c':
		    case 'd':
		    case 'e':
		    case 'f':
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
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		    n = 8 * n + c - '0';
		    c = input_getc(ip);
		    continue;

		default:
		    break;
		}
		break;
	    }
	    lex_getc_undo(c);
	    aer_report_lval.lv_value = rpt_value_integer(n);
	    return CONSTANT;

	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	    stracc_open(&buffer);
	    for (;;)
	    {
		stracc_char(&buffer, c);
		c = input_getc(ip);
		if (c < 0)
		    break;
		if (!isdigit((unsigned char)c))
		    break;
	    }
	    if (c != '.' && c != 'e' && c != 'E')
	    {
		lex_getc_undo(c);
		s = stracc_close(&buffer);
		aer_report_lval.lv_value = rpt_value_integer(atol(s->str_text));
		str_free(s);
		return CONSTANT;
	    }
	    if (c == '.')
	    {
		fraction_part:
		for (;;)
		{
		    stracc_char(&buffer, c);
		    c = input_getc(ip);
		    if (c < 0)
			break;
		    if (!isdigit((unsigned char)c))
			break;
		}
	    }
	    if (c == 'e' || c == 'E')
	    {
		stracc_char(&buffer, c);
		c = input_getc(ip);
		if (c == '+' || c == '-')
		{
		    stracc_char(&buffer, c);
		    c = input_getc(ip);
		}
		if (c < 0 || !isdigit((unsigned char)c))
		{
		    stracc_char(&buffer, '0');
		    aer_report_error(i18n("malformed exponent"));
		}
		else
		{
		    for (;;)
		    {
			stracc_char(&buffer, c);
			c = input_getc(ip);
			if (c < 0)
			    break;
			if (!isdigit((unsigned char)c))
			    break;
		    }
		}
	    }
	    lex_getc_undo(c);
	    s = stracc_close(&buffer);
	    aer_report_lval.lv_value = rpt_value_real(atof(s->str_text));
	    str_free(s);
	    return CONSTANT;

	case '"':
	case '\'':
	    term = c;
	    stracc_open(&buffer);
	    for (;;)
	    {
		c = input_getc(ip);
		if (c < 0)
		{
		    str_eof:
		    aer_report_error(i18n("end-of-file within string"));
		    break;
		}
		if (c == '\n')
		{
		    aer_report_error(i18n("end-of-line within string"));
		    break;
		}
		if (c == term)
		    break;
		if (c == '\\')
		{
		    c = input_getc(ip);
		    switch (c)
		    {
		    default:
			scp = sub_context_new();
			sub_var_set_format(scp, "Name", "\\%c", c);
			aer_lex_error(scp, 0, i18n("unknown '$name' escape"));
			sub_context_delete(scp);
			break;

		    case -1:
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
		    case '\'':
		    case '\\':
			stracc_char(&buffer, c);
			break;

		    case '0':
		    case '1':
		    case '2':
		    case '3':
		    case '4':
		    case '5':
		    case '6':
		    case '7':
			{
			    int		    nc;
			    int		    v;

			    v = 0;
			    for (nc = 0; nc < 3; ++nc)
			    {
				v = v * 8 + c - '0';
				c = input_getc(ip);
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
	    s = stracc_close(&buffer);
	    aer_report_lval.lv_value = rpt_value_string(s);
	    str_free(s);
	    return CONSTANT;

	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case '_':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':
	    stracc_open(&buffer);
	    for (;;)
	    {
		stracc_char(&buffer, c);
		c = input_getc(ip);
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
		case '8':
		case '9':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
		case '_':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z':
		    continue;

		default:
		    lex_getc_undo(c);
		    break;
		}
		break;
	    }
	    s = stracc_close(&buffer);
	    tok = reserved(s);
	    if (tok)
	    {
		str_free(s);
		return tok;
	    }
	    aer_report_lval.lv_string = s;
	    return NAME;

	case '/':
	    c = input_getc(ip);
	    if (c == '/')
	    {
		eoln_comment:
		for (;;)
		{
		    c = input_getc(ip);
		    if (c < 0)
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
		    c = input_getc(ip);
		    if (c < 0)
		    {
			bad_comment:
			aer_report_error(i18n("end-of-file within comment"));
			return 0;
		    }
		    if (c == '*')
			break;
		}
		for (;;)
		{
		    c = input_getc(ip);
		    if (c < 0)
			goto bad_comment;
		    if (c != '*')
			break;
		}
		if (c == '/')
		    break;
	    }
	    break;

	case '#':
	    c = input_getc(ip);
	    if (c == '!')
		goto eoln_comment;
	    lex_getc_undo(c);
	    c = '#';
	    goto normal;

	case '.':
	    c = input_getc(ip);
	    lex_getc_undo(c);
	    if (c < 0 || !isdigit((unsigned char)c))
	    {
		c = '.';
		goto normal;
	    }
	    c = '.';
	    stracc_open(&buffer);
	    stracc_char(&buffer, '0');
	    goto fraction_part;

	default:
	    normal:
	    stracc_open(&buffer);
	    for (;;)
	    {
		stracc_char(&buffer, c);
		s = stracc_close(&buffer);
		if (!symtab_query(stp, s))
		{
		    if (buffer.length > 1)
		    {
			buffer.length--;
			lex_getc_undo(c);
		    }
		    str_free(s);
		    break;
		}
		str_free(s);
		c = input_getc(ip);
	    }
	    s = stracc_close(&buffer);
	    tok = reserved(s);
	    str_free(s);
	    return (tok ? tok : JUNK);
	}
    }
}


void
aer_report_error(const char *fmt)
{
    aer_lex_error(0, 0, fmt);
}


rpt_pos_ty *
rpt_lex_pos_get(void)
{
    static rpt_pos_ty *curpos;
    string_ty	    *s;

    s = input_name(ip);
    if (curpos)
    {
	//
	// The file name includes the line number.
	//
	if (str_equal(curpos->file_name, s))
	    return rpt_pos_copy(curpos);
	rpt_pos_free(curpos);
	curpos = 0;
    }
    curpos = rpt_pos_alloc(s, 0);
    return rpt_pos_copy(curpos);
}


void
aer_lex_error(sub_context_ty *scp, rpt_pos_ty *p, const char *fmt)
{
    int		    need_to_delete;

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

    if (!ip)
	fatal_intl(0, i18n("report aborted"));

    ++error_count;
    if (error_count >= 20)
    {
	sub_var_set_string(scp, "File_Name", input_name(ip));
	fatal_intl(scp, i18n("$filename: too many errors"));
	// NOTREACHED
    }

    if (need_to_delete)
	sub_context_delete(scp);
}


void
rpt_lex_error(rpt_pos_ty *p, const char *fmt)
{
    rpt_pos_error(0, 0, fmt);

    if (!ip)
	fatal_intl(0, i18n("report aborted"));

    ++error_count;
    if (error_count >= 20)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_string(scp, "File_Name", input_name(ip));
	fatal_intl(scp, i18n("$filename: too many errors"));
	// NOTREACHED
	sub_context_delete(scp);
    }
}


//
// These are faked, so that the messages in the generated files
// will also be translated correctly.
//
#if 0
static void
fake()
{
    i18n("parse error"); // bison
    i18n("parse error; also virtual memory exceeded"); // bison
    i18n("parse error; also virtual memory exhausted"); // bison
    i18n("parser stack overflow"); // bison
    i18n("syntax error"); // yacc
    i18n("yacc stack overflow"); // yacc
}
#endif
//
//	aegis - project change supervisor
//	Copyright (C) 1991-1996, 1998, 1999, 2001-2008 Peter Miller
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

#include <common/ac/ctype.h>
#include <common/ac/errno.h>
#include <common/ac/math.h>
#include <common/ac/stdlib.h>

#include <common/error.h>
#include <common/mem.h>
#include <common/nstring/accumulator.h>
#include <common/symtab.h>
#include <libaegis/aer/expr.h>
#include <libaegis/aer/lex.h>
#include <libaegis/aer/pos.h>
#include <libaegis/aer/stmt.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/aer/value/real.h>
#include <libaegis/aer/value/string.h>
#include <libaegis/input/file_text.h>
#include <libaegis/sub.h>
#include <libaegis/aer/report.gen.h> // must be last


static input ip;
static int	error_count;
extern aer_report_STYPE aer_report_lval;
static nstring_accumulator buffer;
static symtab_ty *stp;


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
    if (!stp)
    {
	stp = symtab_alloc(SIZEOF(table));
	for (table_ty *tp = table; tp < ENDOF(table); ++tp)
	{
	    nstring s(tp->name);
	    stp->assign(s, &tp->token);
	}
    }
}


static int
reserved(const nstring &name)
{
    assert(stp);
    const int *data = (int *)stp->query(name);
    return (data ? *data : 0);
}


void
rpt_lex_open(string_ty *s)
{
    reserved_init();
    ip = input_file_text_open(s, true);
    error_count = 0;
}


void
rpt_lex_close(void)
{
    if (error_count)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_string(scp, "File_Name", ip->name());
	sub_var_set_long(scp, "Number", error_count);
	sub_var_optional(scp, "Number");
	fatal_intl(scp, i18n("$filename: has errors"));
	// NOTREACHED
	sub_context_delete(scp);
    }
    ip.close();
}


static inline void
lex_getc_undo(int c)
{
    if (c >= 0)
	ip->ungetc(c);
}


int
aer_report_lex(void)
{
    int		    c;
    long	    n;
    nstring s;
    int		    term;
    int		    tok;

    for (;;)
    {
	c = ip->getch();
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
	    c = ip->getch();
	    if (c == '.')
	    {
		buffer.clear();
		buffer.push_back('0');
		goto fraction_part;
	    }
	    if (c == 'x' || c == 'X')
	    {
		int		ndigits;

		ndigits = 0;
		for (;;)
		{
		    ++ndigits;
		    c = ip->getch();
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
		aer_report_lval.lv_value =
                    new rpt_value::pointer(rpt_value_integer::create(n));
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
		    c = ip->getch();
		    continue;

		default:
		    break;
		}
		break;
	    }
	    lex_getc_undo(c);
	    aer_report_lval.lv_value =
                new rpt_value::pointer(rpt_value_integer::create(n));
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
	    buffer.clear();
	    for (;;)
	    {
		buffer.push_back(c);
		c = ip->getch();
		if (c < 0)
		    break;
		if (!isdigit((unsigned char)c))
		    break;
	    }
	    if (c != '.' && c != 'e' && c != 'E')
	    {
		lex_getc_undo(c);
		s = buffer.mkstr();
                rpt_value::pointer vp =
                    rpt_value_integer::create(atol(s.c_str()));
		aer_report_lval.lv_value = new rpt_value::pointer(vp);
		return CONSTANT;
	    }
	    if (c == '.')
	    {
		fraction_part:
		for (;;)
		{
		    buffer.push_back(c);
		    c = ip->getch();
		    if (c < 0)
			break;
		    if (!isdigit((unsigned char)c))
			break;
		}
	    }
	    if (c == 'e' || c == 'E')
	    {
		buffer.push_back(c);
		c = ip->getch();
		if (c == '+' || c == '-')
		{
		    buffer.push_back(c);
		    c = ip->getch();
		}
		if (c < 0 || !isdigit((unsigned char)c))
		{
		    buffer.push_back('0');
		    aer_report_error(i18n("malformed exponent"));
		}
		else
		{
		    for (;;)
		    {
			buffer.push_back(c);
			c = ip->getch();
			if (c < 0)
			    break;
			if (!isdigit((unsigned char)c))
			    break;
		    }
		}
	    }
	    lex_getc_undo(c);
	    s = buffer.mkstr();
	    aer_report_lval.lv_value =
                new rpt_value::pointer
                (
                    rpt_value_real::create(atof(s.c_str()))
                );
	    return CONSTANT;

	case '"':
	case '\'':
	    term = c;
	    buffer.clear();
	    for (;;)
	    {
		c = ip->getch();
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
		    c = ip->getch();
		    switch (c)
		    {
		    default:
                        {
                            sub_context_ty sc;
                            sc.var_set_format("Name", "\\%c", c);
                            aer_lex_error(sc, i18n("unknown '$name' escape"));
                        }
			break;

		    case -1:
			goto str_eof;

		    case 'b':
			buffer.push_back('\b');
			break;

		    case 'n':
			buffer.push_back('\n');
			break;

		    case 'r':
			buffer.push_back('\r');
			break;

		    case 't':
			buffer.push_back('\t');
			break;

		    case 'f':
			buffer.push_back('\f');
			break;

		    case '"':
		    case '\'':
		    case '\\':
			buffer.push_back(c);
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
				c = ip->getch();
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
			    buffer.push_back(v);
			}
			break;
		    }
		}
		else
		    buffer.push_back(c);
	    }
	    s = buffer.mkstr();
	    aer_report_lval.lv_value =
                new rpt_value::pointer(rpt_value_string::create(s));
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
	    buffer.clear();
	    for (;;)
	    {
		buffer.push_back(c);
		c = ip->getch();
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
	    s = buffer.mkstr();
	    tok = reserved(s);
	    if (tok)
		return tok;
	    aer_report_lval.lv_string = new nstring(s);
	    return NAME;

	case '/':
	    c = ip->getch();
	    if (c == '/')
	    {
		eoln_comment:
		for (;;)
		{
		    c = ip->getch();
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
		    c = ip->getch();
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
		    c = ip->getch();
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
	    c = ip->getch();
	    if (c == '!')
		goto eoln_comment;
	    lex_getc_undo(c);
	    c = '#';
	    goto normal;

	case '.':
	    c = ip->getch();
	    lex_getc_undo(c);
	    if (c < 0 || !isdigit((unsigned char)c))
	    {
		c = '.';
		goto normal;
	    }
	    c = '.';
	    buffer.clear();
	    buffer.push_back('0');
	    goto fraction_part;

	default:
	    normal:
	    buffer.clear();
	    for (;;)
	    {
		buffer.push_back(c);
		s = buffer.mkstr();
		if (!stp->query(s))
		{
		    if (buffer.size() > 1)
		    {
			buffer.pop_back();
			lex_getc_undo(c);
		    }
		    break;
		}
		c = ip->getch();
	    }
	    s = buffer.mkstr();
	    tok = reserved(s);
	    return (tok ? tok : JUNK);
	}
    }
}


void
aer_report_error(const char *fmt)
{
    sub_context_ty sc;
    aer_lex_error(sc, fmt);
}


void
aer_report_error(sub_context_ty &sc, const char *fmt)
{
    aer_lex_error(sc, rpt_lex_pos_get(), fmt);
}


rpt_position::pointer
rpt_lex_pos_get(void)
{
    static rpt_position::pointer curpos;
    nstring s = ip->name();
    if (curpos)
    {
	//
	// The file name includes the line number.
	//
	if (curpos->get_file_name() == s)
	    return curpos;
        curpos.reset();
    }
    curpos = rpt_position::create(s, 0);
    return curpos;
}


void
aer_lex_error(sub_context_ty &sc, const char *fmt)
{
    aer_lex_error(sc, rpt_lex_pos_get(), fmt);
}


void
aer_lex_error(rpt_position::pointer &pp, const char *fmt)
{
    sub_context_ty sc;
    aer_lex_error(sc, pp, fmt);
}


void
aer_lex_error(sub_context_ty &sc, const rpt_position::pointer &pp,
    const char *fmt)
{
    assert(pp);
    pp->print_error(sc, fmt);

    if (!ip.is_open())
	sc.fatal_intl(i18n("report aborted"));

    ++error_count;
    if (error_count >= 20)
    {
	sc.var_set_string("File_Name", ip->name());
	sc.fatal_intl(i18n("$filename: too many errors"));
	// NOTREACHED
    }
}


void
rpt_lex_error(const rpt_position::pointer &pp, const char *fmt)
{
    sub_context_ty sc;
    assert(pp);
    pp->print_error(sc, fmt);

    if (!ip.is_open())
	fatal_intl(&sc, i18n("report aborted"));

    ++error_count;
    if (error_count >= 20)
    {
	sc.var_set_string("File_Name", ip->name());
	sc.fatal_intl(i18n("$filename: too many errors"));
	// NOTREACHED
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

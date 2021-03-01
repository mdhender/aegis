//
//	aegis - project change supervisor
//	Copyright (C) 2002, 2004, 2005 Peter Miller;
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
// MANIFEST: functions to manipulate plural_lexs
//

#include <common/ac/string.h>

#include <common/str.h>
#include <common/stracc.h>
#include <libaegis/sub/plural_lex.h>
#include <libaegis/sub/plural_gram.gen.h>


static string_ty *text;
static size_t   pos;


void
sub_plural_lex_open(string_ty *s)
{
    text = s;
    pos = 0;
}


void
sub_plural_lex_close(void)
{
    text = 0;
    pos = 0;
}


static int
lex_getc(void)
{
    int		    c;

    if (!text || pos >= text->str_length)
	c = 0;
    else
	c = (unsigned char)text->str_text[pos];
    ++pos;
    return c;
}


static void
lex_getc_undo(int c)
{
    if (pos > 0)
	--pos;
}


int
sub_plural_gram_lex(void)
{
    int		    c;
    long	    n;

    for (;;)
    {
	c = lex_getc();
	switch (c)
	{
	case 0:
	    return 0;

	case ' ':
	case '\t':
	case '\n':
	case '\f':
	    break;

	case '!':
	    c = lex_getc();
	    if (c == '=')
		return NE;
	    lex_getc_undo(c);
	    return NOT;

	case '%':
	    return MOD;

	case '&':
	    c = lex_getc();
	    if (c == '&')
		return ANDAND;
	    lex_getc_undo(c);
	    return BITAND;

	case '(':
	    return LP;

	case ')':
	    return RP;

	case '*':
	    return MUL;

	case '+':
	    return PLUS;

	case '-':
	    return MINUS;

	case '/':
	    return DIV;

	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
	    n = 0;
	    for (;;)
	    {
		n = n * 10 + c - '0';
		c = lex_getc();
		switch (c)
		{
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
		    continue;

		default:
		    break;
		}
		lex_getc_undo(c);
		break;
	    }
	    sub_plural_gram_lval.lv_number = n;
	    return INTEGER;

	case ':':
	    return COLON;

	case ';':
	    return SEMI;

	case '<':
	    c = lex_getc();
	    if (c == '=')
		return LE;
	    if (c == '>')
		return NE;
	    lex_getc_undo(c);
	    return LT;

	case '=':
	    c = lex_getc();
	    if (c == '=')
		return EQ;
	    lex_getc_undo(c);
	    return EQU;

	case '>':
	    c = lex_getc();
	    if (c == '=')
		return GE;
	    lex_getc_undo(c);
	    return GT;

	case '?':
	    return QUEST;
	case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
	case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
	case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
	case 'V': case 'W': case 'X': case 'Y': case 'Z':
	case '_':
	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
	case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
	case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
	case 'v': case 'w': case 'x': case 'y': case 'z':
	    {
		static stracc_t sa;
		string_ty	*s;

		sa.clear();
		for (;;)
		{
		    sa.push_back(c);
		    c = lex_getc();
		    switch (c)
		    {
		    case 'A': case 'B': case 'C': case 'D': case 'E':
		    case 'F': case 'G': case 'H': case 'I': case 'J':
		    case 'K': case 'L': case 'M': case 'N': case 'O':
		    case 'P': case 'Q': case 'R': case 'S': case 'T':
		    case 'U': case 'V': case 'W': case 'X': case 'Y':
		    case 'Z': case '_':
		    case 'a': case 'b': case 'c': case 'd': case 'e':
		    case 'f': case 'g': case 'h': case 'i': case 'j':
		    case 'k': case 'l': case 'm': case 'n': case 'o':
		    case 'p': case 'q': case 'r': case 's': case 't':
		    case 'u': case 'v': case 'w': case 'x': case 'y':
		    case 'z':
		    case '0': case '1': case '2': case '3': case '4':
		    case '5': case '6': case '7': case '8': case '9':
			continue;

		    case 0:
			break;

		    default:
			lex_getc_undo(c);
			break;
		    }
		    break;
		}
		s = sa.mkstr();
		if (0 == strcasecmp(s->str_text, "n"))
		{
		    str_free(s);
		    return NUMBER;
		}
		if (0 == strcasecmp(s->str_text, "nplurals"))
		{
		    str_free(s);
		    return NPLURALS;
		}
		if (0 == strcasecmp(s->str_text, "plural"))
		{
		    str_free(s);
		    return PLURAL;
		}
		str_free(s);
	    }
	    return JUNK;

	case '^':
	    return BITXOR;

	case '|':
	    c = lex_getc();
	    if (c == '|')
		return OROR;
	    lex_getc_undo(c);
	    return BITOR;

	case '~':
	    return BITNOT;

	default:
	    return JUNK;
	}
    }
}

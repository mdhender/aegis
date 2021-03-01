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
// MANIFEST: functions to perform lexical analysis on aegis' data files
//

#include <ac/ctype.h>
#include <ac/errno.h>
#include <ac/stdarg.h>
#include <ac/stdio.h>
#include <ac/stdlib.h>
#include <ac/string.h>

#include <error.h> // for assert
#include <input/crlf.h>
#include <input/env.h>
#include <input/file.h>
#include <input/gunzip.h>
#include <meta_lex.h>
#include <mem.h>
#include <quit.h>
#include <str.h>
#include <stracc.h>
#include <gram.gen.h> // must be after <str.h>
#include <sub.h>
#include <trace.h>
#include <zero.h>


static input_ty *input;
static int	error_count;
extern gram_STYPE gram_lval;
static stracc_t buffer;


void
lex_open(string_ty *s)
{
    input_ty	    *fp;

    //
    // Open the underlying binary file.
    //
    fp = input_file_open(s);

    //
    // Decompress the input stream.  If it *isn't* compressed, this
    // incurs NO overhead, because the gunzip code gets itself out
    // of the way, and returns the original fp.
    //
    fp = input_gunzip(fp);

    //
    // Get rid of CRLF sequences in the input.
    // This happens, for instance, when the file is created on
    // windows nt, but used on Unix.
    //
    fp = input_crlf(fp, 1);

    //
    // Now that we've completely messed with your mind, parse the
    // resulting input stream.
    //
    lex_open_input(fp);
}


void
lex_open_env(const char *s)
{
    lex_open_input(input_env_open(s));
}


void
lex_open_input(input_ty *ifp)
{
    input = ifp;
}


void
lex_close(void)
{
    if (error_count)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_string(scp, "File_Name", input_name(input));
	sub_var_set_long(scp, "Number", error_count);
	sub_var_optional(scp, "Number");
	fatal_intl(scp, i18n("$filename: has errors"));
	// NOTREACHED
    }
    input_delete(input);
    input = 0;
}


#define lex_getc_undo(c) ((c) >= 0 ? input_ungetc(input, (c)) : (void)0)


int
gram_lex(void)
{
    sub_context_ty  *scp;
    int		    c;
    int		    ndigits;

    for (;;)
    {
	c = input_getc(input);
	switch (c)
	{
	case ' ':
	case '\t':
	case '\f':
	case '\n':
	    break;

	case '0':
	    buffer.clear();
	    buffer.push_back('0');
	    c = input_getc(input);
	    if (c == 'x' || c == 'X')
	    {
		buffer.push_back(c);
		ndigits = 0;
		for (;;)
		{
		    c = input_getc(input);
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
		    case 'a':
		    case 'b':
		    case 'c':
		    case 'd':
		    case 'e':
		    case 'f':
			++ndigits;
			buffer.push_back(c);
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
		buffer.push_back(' ');
		gram_lval.lv_integer =
		    strtoul(buffer.get_data(), (char **)0, 16);
		goto integer_return;
	    }
	    if (c == '.')
	    {
		buffer.push_back(c);
		goto fraction;
	    }
	    if (c == 'e' || c == 'E')
		goto exponent;
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
		    buffer.push_back(c);
		    c = input_getc(input);
		    continue;

		default:
		    break;
		}
		break;
	    }
	    lex_getc_undo(c);
	    buffer.push_back(' ');
	    gram_lval.lv_integer = strtoul(buffer.get_data(), (char **)0, 8);
	    goto integer_return;

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
		c = input_getc(input);
		if (c < 0)
		    break;
		if (!isdigit((unsigned char)c))
		    break;
	    }
	    if (c == '.')
	    {
		buffer.push_back(c);
		goto fraction;
	    }
	    if (c == 'e' || c == 'E')
		goto exponent;
	    lex_getc_undo(c);
	    buffer.push_back(' ');
	    gram_lval.lv_integer = strtoul(buffer.get_data(), (char **)0, 10);
	    assert(gram_lval.lv_integer >= 0);
	    integer_return:
	    trace(("%s: INTEGER %ld\n", input_name(input)->str_text,
		gram_lval.lv_integer));
	    return INTEGER;

	case '.':
	    c = input_getc(input);
	    if (c < 0 || !isdigit((unsigned char)c))
	    {
		lex_getc_undo(c);
		return '.';
	    }
	    buffer.clear();
	    buffer.push_back('0');
	    buffer.push_back('.');
	    buffer.push_back(c);
	    fraction:
	    for (;;)
	    {
		c = input_getc(input);
		if (c < 0 || !isdigit((unsigned char)c))
		    break;
		buffer.push_back(c);
	    }
	    if (c == 'e' || c == 'E')
	    {
		exponent:
		buffer.push_back(c);
		c = input_getc(input);
		if (c == '+' || c == '-')
		{
		    buffer.push_back(c);
		    c = input_getc(input);
		}
		ndigits = 0;
		for (;;)
		{
		    c = input_getc(input);
		    if (c < 0 || !isdigit((unsigned char)c))
			break;
		    ++ndigits;
		    buffer.push_back(c);
		}
		if (!ndigits)
		{
		    gram_error(i18n("malformed exponent"));
		    gram_lval.lv_real = 0;
		    trace(("%s: REAL 0\n", input_name(input)->str_text));
		    return REAL;
		}
	    }
	    lex_getc_undo(c);
	    buffer.push_back('\0');
	    gram_lval.lv_real = atof(buffer.get_data());
	    trace(("%s: REAL %g\n", input_name(input)->str_text,
		gram_lval.lv_real));
	    return REAL;

	case '"':
	    buffer.clear();
	    for (;;)
	    {
		c = input_getc(input);
		if (c == EOF)
		{
		    str_eof:
		    gram_error("end-of-file within string");
		    break;
		}
		if (c == '\n')
		{
		    gram_error("end-of-line within string");
		    break;
		}
		if (c == '"')
		    break;
		if (c == '\\')
		{
		    c = input_getc(input);
		    switch (c)
		    {
		    default:
			scp = sub_context_new();
			sub_var_set_format(scp, "Name", "\\%c", c);
			lex_error(scp, i18n("unknown '$name' escape"));
			sub_context_delete(scp);
			break;

		    case '\n':
			break;

		    case EOF:
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
			    int		    n;
			    int		    v;

			    v = 0;
			    for (n = 0; n < 3; ++n)
			    {
				v = v * 8 + c - '0';
				c = input_getc(input);
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
	    gram_lval.lv_string = buffer.mkstr();
	    trace(("%s: STRING \"%s\"\n", input_name(input)->str_text,
		gram_lval.lv_string->str_text));
	    return STRING;

	case '@':
	    buffer.clear();
	    for (;;)
	    {
		c = input_getc(input);
		switch (c)
		{
		case EOF:
		    goto str_eof;

		case '@':
		    c = input_getc(input);
		    if (c == EOF)
			break;
		    if (c != '@')
		    {
			input_ungetc(input, c);
			break;
		    }
		    // fall through...

		default:
		    buffer.push_back(c);
		    continue;
		}
		break;
	    }
	    gram_lval.lv_string = buffer.mkstr();
	    trace(("%s: STRING \"%s\"\n", input_name(input)->str_text,
		gram_lval.lv_string->str_text));
	    return STRING;

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
		c = input_getc(input);
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
	    if (buffer.size() == 4 && !memcmp(buffer.get_data(), "ZERO", 4))
	    {
		gram_lval.lv_integer = MAGIC_ZERO;
		goto integer_return;
	    }
	    gram_lval.lv_string = buffer.mkstr();
	    trace(("%s: NAME \"%s\"\n", input_name(input)->str_text,
		gram_lval.lv_string->str_text));
	    return NAME;

	case '#':
	    //
	    // Shell style single line comment
	    //
	    single_line_comment:
	    for (;;)
	    {
		c = input_getc(input);
		if (c == EOF || c == '\n')
		    break;
	    }
	    break;

	case '/':
	    //
	    // C and C++ style comments
	    //
	    c = input_getc(input);
	    if (c == '/')
	    {
		//
		// C++ style single line comment.
		//
		goto single_line_comment;
	    }
	    if (c != '*')
	    {
		//
		// Not a C style block comment,
		// just return the slash.
		//
		lex_getc_undo(c);
		trace(("%s: '/'\n", input_name(input)->str_text));
		return '/';
	    }
	    for (;;)
	    {
		for (;;)
		{
		    c = input_getc(input);
		    if (c == EOF)
		    {
			bad_comment:
			gram_error("end-of-file within comment");
			quit(1);
		    }
		    if (c == '*')
			break;
		}
		for (;;)
		{
		    c = input_getc(input);
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
	    trace(("%s: end of file\n", input_name(input)->str_text));
	    return 0;

	default:
	    trace(("%s: '%c'\n", input_name(input)->str_text, c));
	    return c;
	}
    }
}


void
gram_error(const char *s)
{
    sub_context_ty  *scp;

    scp = sub_context_new();
    lex_error(scp, s);
    sub_context_delete(scp);
}


void
lex_error(sub_context_ty *scp, const char *s)
{
    string_ty	    *msg;

    msg = subst_intl(scp, s);

    // re-use substitution context
    sub_var_set_string(scp, "Message", msg);
    sub_var_set_string(scp, "File_Name", input_name(input));
    error_intl(scp, i18n("$filename: $message"));
    str_free(msg);
    if (++error_count >= 20)
    {
	// re-use substitution context
	sub_var_set_string(scp, "File_Name", input_name(input));
	fatal_intl(scp, i18n("$filename: too many errors"));
    }

#if 0
    //
    // This stuff is here to insulate against error messages that various
    // versions if GNU Bison may or may not issue.  If there are similar
    // issues with byacc, put them here, too.
    //
    i18n("syntax error: cannot back up")
    i18n("syntax error; also virtual memory exhausted");
#endif
}


string_ty *
lex_position(void)
{
    static string_ty *s;

    if (input)
	return input_name(input);
    if (!s)
	s = str_from_c("end-of-input");
    return s;
}

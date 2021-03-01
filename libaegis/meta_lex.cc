//
// aegis - project change supervisor
// Copyright (C) 1991-1996, 1998, 1999, 2001-2008, 2010, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>
#include <common/ac/ctype.h>
#include <common/ac/errno.h>
#include <common/ac/stdarg.h>
#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>

#include <common/mem.h>
#include <common/nstring/accumulator.h>
#include <common/quit.h>
#include <common/trace.h>
#include <libaegis/gram.yacc.h> // must be after <common/nstring.h>
#include <libaegis/input/bunzip2.h>
#include <libaegis/input/crlf.h>
#include <libaegis/input/env.h>
#include <libaegis/input/file.h>
#include <libaegis/input/gunzip.h>
#include <libaegis/meta_lex.h>
#include <libaegis/sub.h>
#include <libaegis/zero.h>


/**
  * The source global variable is used to remember the managed input
  * stream if the currently being parsed meta-data file.
  */
static input source;

static int error_count;
extern gram_STYPE gram_lval;
static nstring_accumulator buffer;


input
lex_iopen_file(const nstring &filename)
{
    //
    // Open the underlying binary file.
    //
    input fp = input_file_open(filename);

    //
    // Decompress the input stream.  If it *isn't* compressed, this
    // incurs NO overhead, because the gunzip code gets itself out
    // of the way, and returns the original fp.
    //
    fp = input_gunzip_open(fp);
    fp = input_bunzip2_open(fp);

    //
    // Get rid of CRLF sequences in the input.
    // This happens, for instance, when the file is created on
    // windows nt, but used on Unix.
    //
    fp = input_crlf::create(fp);

    return fp;
}


input
lex_iopen_env(const nstring &name)
{
    return input_env_open(name);
}


void
lex_open_input(input &ifp)
{
    assert(!source.is_open());
    source = ifp;
}


void
lex_close(void)
{
    assert(source.is_open());
    if (error_count)
    {
        sub_context_ty  *scp;

        scp = sub_context_new();
        sub_var_set_string(scp, "File_Name", source->name());
        sub_var_set_long(scp, "Number", error_count);
        sub_var_optional(scp, "Number");
        fatal_intl(scp, i18n("$filename: has errors"));
        // NOTREACHED
    }
    source.close();
}


static inline void
lex_getc_undo(int c)
{
    if (c >= 0)
        source->ungetc(c);
}


int
gram_lex(void)
{
    sub_context_ty  *scp;
    int             c;
    int             ndigits;

    for (;;)
    {
        c = source->getch();
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
            c = source->getch();
            if (c == 'x' || c == 'X')
            {
                buffer.push_back(c);
                ndigits = 0;
                for (;;)
                {
                    c = source->getch();
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
                    c = source->getch();
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
                c = source->getch();
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
            trace(("%s: INTEGER %ld\n", source->name().c_str(),
                gram_lval.lv_integer));
            return INTEGER;

        case '.':
            c = source->getch();
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
                c = source->getch();
                if (c < 0 || !isdigit((unsigned char)c))
                    break;
                buffer.push_back(c);
            }
            if (c == 'e' || c == 'E')
            {
                exponent:
                buffer.push_back(c);
                c = source->getch();
                if (c == '+' || c == '-')
                {
                    buffer.push_back(c);
                    c = source->getch();
                }
                ndigits = 0;
                for (;;)
                {
                    c = source->getch();
                    if (c < 0 || !isdigit((unsigned char)c))
                        break;
                    ++ndigits;
                    buffer.push_back(c);
                }
                if (!ndigits)
                {
                    gram_error(i18n("malformed exponent"));
                    gram_lval.lv_real = 0;
                    trace(("%s: REAL 0\n", source->name().c_str()));
                    return REAL;
                }
            }
            lex_getc_undo(c);
            buffer.push_back('\0');
            gram_lval.lv_real = atof(buffer.get_data());
            trace(("%s: REAL %g\n", source->name().c_str(),
                gram_lval.lv_real));
            return REAL;

        case '"':
            buffer.clear();
            for (;;)
            {
                c = source->getch();
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
                    c = source->getch();
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
                            int             n;
                            int             v;

                            v = 0;
                            for (n = 0; n < 3; ++n)
                            {
                                v = v * 8 + c - '0';
                                c = source->getch();
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
            gram_lval.lv_string = new nstring(buffer.mkstr());
            trace(("%s: STRING %s\n", source->name().c_str(),
                gram_lval.lv_string->quote_c().c_str()));
            return STRING;

        case '@':
            buffer.clear();
            for (;;)
            {
                c = source->getch();
                switch (c)
                {
                case EOF:
                    goto str_eof;

                case '@':
                    c = source->getch();
                    if (c == EOF)
                        break;
                    if (c != '@')
                    {
                        source->ungetc(c);
                        break;
                    }
                    // fall through...

                default:
                    buffer.push_back(c);
                    continue;
                }
                break;
            }
            gram_lval.lv_string = new nstring(buffer.mkstr());
            trace(("%s: STRING %s\n", source->name().c_str(),
                gram_lval.lv_string->quote_c().c_str()));
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
                c = source->getch();
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
            gram_lval.lv_string = new nstring(buffer.mkstr());
            trace(("%s: NAME %s\n", source->name().c_str(),
                gram_lval.lv_string->quote_c().c_str()));
            return NAME;

        case '#':
            //
            // Shell style single line comment
            //
            single_line_comment:
            for (;;)
            {
                c = source->getch();
                if (c == EOF || c == '\n')
                    break;
            }
            break;

        case '/':
            //
            // C and C++ style comments
            //
            c = source->getch();
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
                trace(("%s: '/'\n", source->name().c_str()));
                return '/';
            }
            for (;;)
            {
                for (;;)
                {
                    c = source->getch();
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
                    c = source->getch();
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
            trace(("%s: end of file\n", source->name().c_str()));
            return 0;

        default:
            trace(("%s: '%c'\n", source->name().c_str(), c));
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
    string_ty       *msg;

    msg = subst_intl(scp, s);

    // re-use substitution context
    sub_var_set_string(scp, "MeSsaGe", msg);
    sub_var_set_string(scp, "File_Name", source->name());
    error_intl(scp, i18n("$filename: $message"));
    str_free(msg);
    if (++error_count >= 20)
    {
        // re-use substitution context
        sub_var_set_string(scp, "File_Name", source->name());
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

    if (source.is_open())
        return source->name().get_ref();
    if (!s)
        s = str_from_c("end-of-input");
    return s;
}


// vim: set ts=8 sw=4 et :

//
// aegis - project change supervisor
// Copyright (C) 1991-1995, 1997, 1999, 2002-2008, 2012 Peter Miller
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

#include <common/error.h>
#include <common/nstring/accumulator.h>
#include <common/nstring/list.h>
#include <common/sizeof.h>
#include <common/symtab/template.h>
#include <common/trace.h>

#include <fmtgen/lex.h>
#include <fmtgen/type.h>
#include <fmtgen/parse.yacc.h> // must be last


struct file_ty
{
    ~file_ty() { if (fp) fclose(fp); fp = 0; line_number = 0; next = 0; }
    file_ty() : fp(0), line_number(1), next(0) { }
    FILE *fp;
    int line_number;
    nstring file_name;
    file_ty *next;
};

static file_ty *file;
static int error_count;
extern parse_STYPE parse_lval;
static nstring_list ifiles;
static nstring_list include_path;
static symtab<int> keyword;
static nstring most_recent_comment;


/**
  * The lex_initialize function adds all the keywords to the symbol table.
  * The keywords are intentionally case sensitive.
  */

static void
lex_initialize(void)
{
    struct keyword_ty
    {
        const char *k_name;
        int k_token;
    };

    static const keyword_ty table[] =
    {
        { "boolean", BOOLEAN, },
        { "false", BOOLEAN_CONSTANT, },
        { "hide_if_default", HIDE_IF_DEFAULT, },
        { "include", INCLUDE, },
        { "integer", INTEGER, },
        { "real", REAL, },
        { "redefinition_ok", REDEFINITION_OK, },
        { "show_if_default", SHOW_IF_DEFAULT, },
        { "string", STRING, },
        { "time", TIME, },
        { "true", BOOLEAN_CONSTANT, },
        { "type", TYPE, },
    };

    if (!keyword.empty())
        return;
    for (const keyword_ty *kp = table; kp < ENDOF(table); ++kp)
    {
        keyword.assign(kp->k_name, kp->k_token);
    }
}


void
lex_open(const nstring &s)
{
    file_ty *f = new file_ty;
    if (!file)
        lex_initialize();

    if (s[0] != '/')
    {
        for (size_t j = 0; j < include_path.size(); ++j)
        {
            char buffer[2000];
            snprintf
            (
                buffer,
                sizeof(buffer),
                "%s/%s",
                include_path[j].c_str(),
                s.c_str()
            );
            f->fp = fopen(buffer, "r");
            if (f->fp)
            {
                f->file_name = nstring(buffer);
                break;
            }
            if (errno != ENOENT)
                nfatal("open %s", buffer);
        }
    }
    if (!f->fp)
    {
        f->fp = fopen(s.c_str(), "r");
        if (!f->fp)
            nfatal("open %s", s.quote_c().c_str());
        f->file_name = s;
    }
    f->next = file;
    ifiles.push_back_unique(s);
    file = f;
}


void
lex_close(void)
{
    if (error_count)
        exit(1);
    delete file;
}


static int
lex_getc(void)
{
    int c = 0;
    for (;;)
    {
        c = fgetc(file->fp);
        if (c != EOF)
            break;
        if (ferror(file->fp))
            nfatal("read %s", file->file_name.quote_c().c_str());
        if (!file->next)
            break;
        file_ty *old = file;
        file = old->next;
        delete old;
    }
    if (c == '\n')
        file->line_number++;
    return c;
}


static void
lex_getc_undo(int c)
{
    switch (c)
    {
    case EOF:
        break;

    case '\n':
        file->line_number--;
        // fall through...

    default:
        ungetc(c, file->fp);
        break;
    }
}


int
parse_lex(void)
{
    static nstring_accumulator buffer;
    for (;;)
    {
        buffer.clear();
        int line_number_start = file->line_number;
        int c = lex_getc();
        switch (c)
        {
        case ' ':
        case '\t':
        case '\f':
        case '\n':
            break;

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
            parse_lval.lv_integer = 0;
            for (;;)
            {
                parse_lval.lv_integer = 10 * parse_lval.lv_integer + c - '0';
                c = lex_getc();
                if (c < '0' || c > '9')
                {
                    lex_getc_undo(c);
                    break;
                }
            }
            trace(("INTEGER_CONSTANT\n"));
            return INTEGER_CONSTANT;

        case 'A': case 'B': case 'C': case 'D': case 'E':
        case 'F': case 'G': case 'H': case 'I': case 'J':
        case 'K': case 'L': case 'M': case 'N': case 'O':
        case 'P': case 'Q': case 'R': case 'S': case 'T':
        case 'U': case 'V': case 'W': case 'X': case 'Y':
        case 'Z': case '_': case 'a': case 'b': case 'c':
        case 'd': case 'e': case 'f': case 'g': case 'h':
        case 'i': case 'j': case 'k': case 'l': case 'm':
        case 'n': case 'o': case 'p': case 'q': case 'r':
        case 's': case 't': case 'u': case 'v': case 'w':
        case 'x': case 'y': case 'z':
            {
                for (;;)
                {
                    buffer.push_back((char)c);
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
                    case 'Y': case 'Z': case '_': case 'a':
                    case 'b': case 'c': case 'd': case 'e':
                    case 'f': case 'g': case 'h': case 'i':
                    case 'j': case 'k': case 'l': case 'm':
                    case 'n': case 'o': case 'p': case 'q':
                    case 'r': case 's': case 't': case 'u':
                    case 'v': case 'w': case 'x': case 'y':
                    case 'z':
                        continue;
                    }
                    lex_getc_undo(c);
                    break;
                }
                nstring s = buffer.mkstr();
                int *data = keyword.query(s);
                if (data)
                {
                    trace(("keyword:%s\n", s.c_str()));
                    return *data;
                }
                parse_lval.lv_string = new nstring(s);
                trace(("NAME\n"));
                return NAME;
            }

        case '/':
            c = lex_getc();
            if (c != '*')
            {
                lex_getc_undo(c);
                trace(("character:'/'\n"));
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
                        parse_error("end-of-file inside comment");
                        exit(1);
                    }
                    if (!buffer.empty() || !isspace((unsigned char)c))
                        buffer.push_back((char)c);
                    if (c == '*')
                        break;
                }
                for (;;)
                {
                    c = lex_getc();
                    if (c == EOF)
                        goto bad_comment;
                    buffer.push_back((char)c);
                    if (c != '*')
                        break;
                }
                if (c == '/')
                {
                    assert(buffer.size() >= 2);
                    buffer.pop_back();
                    buffer.pop_back();
                    while
                    (
                        !buffer.empty()
                    &&
                        isspace((unsigned char)buffer.back())
                    )
                        buffer.pop_back();
                    most_recent_comment = buffer.mkstr();
                    break;
                }
            }
            break;

        case '<':
            for (;;)
            {
                c = lex_getc();
                if (c == EOF)
                    goto str_eof;
                if (c == '\n')
                    goto str_eoln;
                if (c == '>')
                    break;
                buffer.push_back((char)c);
            }
            parse_lval.lv_string = new nstring(buffer.mkstr());
            trace(("STRING_CONSTANT\n"));
            return STRING_CONSTANT;

        case '"':
            for (;;)
            {
                c = lex_getc();
                if (c == EOF)
                {
                    str_eof:
                    file->line_number = line_number_start;
                    parse_error("end-of-file within string");
                    break;
                }
                if (c == '\n')
                {
                    str_eoln:
                    file->line_number = line_number_start;
                    parse_error("end-of-line within string");
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
                        parse_error("unknown '\\%c' escape", c);
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
                        buffer.push_back((char)c);
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
                            int v = 0;
                            for (int n = 0; n < 3; ++n)
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
                            buffer.push_back((char)v);
                        }
                        break;
                    }
                }
                else
                    buffer.push_back((char)c);
            }
            parse_lval.lv_string = new nstring(buffer.mkstr());
            trace(("STRING_CONSTANT\n"));
            return STRING_CONSTANT;

        default:
            trace(("character:%s\n", unctrl(c)));
            return c;
        }
    }
}


void
parse_error(const char *s, ...)
{
    va_list         ap;
    char            buffer[1000];

    va_start(ap, s);
    vsnprintf(buffer, sizeof(buffer), s, ap);
    va_end(ap);
    error_raw("%s: %d: %s", file->file_name.c_str(), file->line_number, buffer);
    if (++error_count >= 20)
        error_raw("%s: too many errors, bye!", file->file_name.c_str());
}


void
lex_message(const char *s, ...)
{
    va_list ap;
    va_start(ap, s);
    char buffer[1000];
    vsnprintf(buffer, sizeof(buffer), s, ap);
    va_end(ap);
    error_raw("%s: %d: %s", file->file_name.c_str(), file->line_number, buffer);
}


bool
lex_in_include_file(void)
{
    trace(("lex_in_include_file() => %d\n", (file && file->next)));
    return (file && file->next);
}


void
lex_include_path(const nstring &dir)
{
    include_path.push_back_unique(dir);
}


nstring
lex_comment_get()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    nstring s = most_recent_comment;
    most_recent_comment.clear();
    return s;
}


nstring
lex_position_get()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    if (!file)
        return "";
    return
        nstring::format("%s: %d", file->file_name.c_str(), file->line_number);
}


#ifdef DEBUG

static char lex_debug_buf[3000];
static char *lex_debug_buf_p = lex_debug_buf;


void
lex_debug_vprintf(const char *s, va_list ap)
{
    char buffer[1000];
    vsnprintf(buffer, sizeof(buffer), s, ap);

    char *start = lex_debug_buf_p;
    lex_debug_buf_p =
        strendcpy
        (
            lex_debug_buf_p,
            buffer,
            lex_debug_buf + sizeof(lex_debug_buf)
        );
    const char *ep = strchr(start, '\n');
    if (ep)
    {
        error_raw
        (
            "%s: %d: %.*s",
            file->file_name.c_str(),
            file->line_number,
            (ep - lex_debug_buf),
            lex_debug_buf
        );
        size_t nbytes = lex_debug_buf_p - (ep + 1);
        memmove
        (
            lex_debug_buf,
            ep + 1,
            nbytes + 1
        );
        lex_debug_buf_p = lex_debug_buf + nbytes;
    }
}


void
lex_debug_printf(const char *s, ...)
{
    va_list ap;
    va_start(ap, s);
    lex_debug_vprintf(s, ap);
    va_end(ap);
}


void
lex_debug_fprintf(void *, const char *s, ...)
{
    va_list ap;
    va_start(ap, s);
    lex_debug_vprintf(s, ap);
    va_end(ap);
}

#endif // DEBUG


// vim: set ts=8 sw=4 et :

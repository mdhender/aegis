//
//      aegis - project change supervisor
//      Copyright (C) 1991-1995, 1997, 1999, 2002-2006, 2008, 2012 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <common/ac/ctype.h>
#include <common/ac/stdio.h>
#include <common/ac/string.h>
#include <common/ac/stddef.h>
#include <common/ac/stdarg.h>
#include <common/ac/time.h>

#include <common/error.h>
#include <common/mem.h>
#include <common/progname.h>
#include <common/str.h>
#include <common/nstring.h>
#include <common/trace.h>


#define INDENT 2

struct known_ty
{
    string_ty       *filename;
    int             flag;
    int             *flag_p;
    known_ty        *next;
};

static string_ty *file_name;
static int      line_number;
static int      page_width;
static known_ty *known;
static int      depth;


static string_ty *
base_name(const char *file)
{
    const char      *cp1;
    const char      *cp2;

    cp1 = strrchr(file, '/');
    if (cp1)
        ++cp1;
    else
        cp1 = file;
    cp2 = strrchr(cp1, '.');
    if (!cp2)
        cp2 = cp1 + strlen(cp1);
    if (cp2 > cp1 + 6)
        return str_n_from_c(cp1, 6);
    return str_n_from_c(cp1, cp2 - cp1);
}


int
trace_pretest(const char *file, int *result)
{
    string_ty       *s;
    known_ty        *kp;

    s = base_name(file);
    for (kp = known; kp; kp = kp->next)
    {
        if (str_equal(s, kp->filename))
        {
            str_free(s);
            break;
        }
    }
    if (!kp)
    {
        kp = (known_ty *)mem_alloc(sizeof(known_ty));
        kp->filename = s;
        kp->next = known;
        kp->flag = 2; // disabled
        known = kp;
    }
    kp->flag_p = result;
    *result = kp->flag;
    return *result;
}


void
trace_where(const char *file, int line)
{
    string_ty       *s;

    //
    // take new name fist, because will probably be same as last
    // thus saving a free and a malloc (which are slow)
    //
    s = base_name(file);
    if (file_name)
        str_free(file_name);
    file_name = s;
    line_number = line;
}


static void
trace_putchar(int c)
{
    static char     buffer[200];
    static char     *cp;
    static int      in_col;
    static int      out_col;

    if (!page_width)
    {
        // don't use last column, many terminals are dumb
        page_width = 79;
        // allow for progname, filename and line number (8 each)
        page_width -= 24;
        if (page_width < 16)
            page_width = 16;
    }
    if (!cp)
    {
        cp = strendcpy(buffer, progname_get(), buffer + sizeof(buffer));
        if (cp > buffer + 6)
            cp = buffer + 6;
        *cp++ = ':';
        *cp++ = '\t';
        cp = strendcpy(cp, file_name->str_text, buffer + sizeof(buffer));
        *cp++ = ':';
        *cp++ = '\t';
        snprintf(cp, buffer + sizeof(buffer) - cp, "%d:\t", line_number);
        cp += strlen(cp);
        in_col = 0;
        out_col = 0;
    }
    switch (c)
    {
    case '\n':
        *cp++ = '\n';
        *cp = 0;
        fflush(stdout);
        fputs(buffer, stderr);
        fflush(stderr);
        if (ferror(stderr))
            nfatal("(stderr)");
        cp = 0;
        break;

    case ' ':
        if (out_col)
            ++in_col;
        break;

    case '\t':
        if (out_col)
            in_col = (in_col/INDENT + 1) * INDENT;
        break;

    case '}':
    case ')':
    case ']':
        if (depth > 0)
            --depth;
        // fall through

    default:
        if (!out_col)
        {
            if (c != '#')
                // modulo so never too long
                in_col = (INDENT * depth) % page_width;
            else
                in_col = 0;
        }
        if (in_col >= page_width)
        {
            trace_putchar('\n');
            trace_putchar(c);
            return;
        }
        while (((out_col + 8) & -8) <= in_col && out_col + 1 < in_col)
        {
            *cp++ = '\t';
            out_col = (out_col + 8) & -8;
        }
        while (out_col < in_col)
        {
            *cp++ = ' ';
            ++out_col;
        }
        if (c == '{' || c == '(' || c == '[')
            ++depth;
        *cp++ = c;
        in_col++;
        out_col++;
        break;
    }
}


void
trace_printf(const char *s, ...)
{
    va_list         ap;
    char            buffer[3000];

    va_start(ap, s);
    vsnprintf(buffer, sizeof(buffer), s, ap);
    va_end(ap);
    for (s = buffer; *s; ++s)
        trace_putchar(*s);
}


const char*
trace_args()
{
    static nstring args;
    if (args.empty())
    {
        for (known_ty *kp = known; kp; kp = kp->next)
        {
            if (kp->flag != 3)
                continue;
            if (args.empty())
                args += " -Trace";
            args += " ";
            args += kp->filename->str_text;
        }
    }
    return args.c_str();
}


void
trace_enable(const char *file)
{
    string_ty       *s;
    known_ty        *kp;

    s = base_name(file);
    for (kp = known; kp; kp = kp->next)
    {
        if (str_equal(s, kp->filename))
        {
            str_free(s);
            break;
        }
    }
    if (!kp)
    {
        kp = (known_ty *)mem_alloc(sizeof(known_ty));
        kp->filename = s;
        kp->flag_p = 0;
        kp->next = known;
        known = kp;
    }
    kp->flag = 3; // enabled
    if (kp->flag_p)
        *kp->flag_p = kp->flag;

    //
    // this silences a warning...
    //
#ifdef DEBUG
    trace_pretest_result = 1;
#endif
}


void
trace_bool_real(const char *name, const bool &value)
{
    if (name && *name)
        trace_printf("%s = ", name);
    trace_printf("%s", (value ? "true" : "false"));
    trace_printf(name && *name ? ";\n" : ",\n");
}


void
trace_char_real(const char *name, const char *vp)
{
    trace_printf("%s = '", name);
    if (!isprint((unsigned char)*vp) || strchr("(){}[]", *vp))
    {
        const char *s = strchr("\bb\nn\tt\rr\ff", *vp);
        if (s)
        {
            trace_putchar('\\');
            trace_putchar(s[1]);
        }
        else
            trace_printf("\\%03o", (unsigned char)*vp);
    }
    else
    {
            if (strchr("'\\", *vp))
                    trace_putchar('\\');
            trace_putchar(*vp);
    }
    trace_printf("'; /* 0x%02X, %d */\n", (unsigned char)*vp, *vp);
}


void
trace_char_unsigned_real(const char *name, const unsigned char *vp)
{
    trace_printf("%s = '", name);
    if (!isprint((unsigned char)*vp) || strchr("(){}[]", *vp))
    {
        const char *s = strchr("\bb\nn\tt\rr\ff", *vp);
        if (s)
        {
            trace_putchar('\\');
            trace_putchar(s[1]);
        }
        else
            trace_printf("\\%03o", *vp);
    }
    else
    {
        if (strchr("'\\", *vp))
            trace_putchar('\\');
        trace_putchar(*vp);
    }
    trace_printf("'; /* 0x%02X, %d */\n", *vp, *vp);
}


void
trace_int_real(const char *name, const int *vp)
{
    trace_printf("%s = %d;\n", name, *vp);
}


void
trace_int_unsigned_real(const char *name, const unsigned int *vp)
{
    trace_printf("%s = %u;\n", name, *vp);
}


void
trace_long_real(const char *name, const long *vp)
{
    if (name && *name)
        trace_printf("%s = ", name);
    trace_printf("%ld", *vp);
    trace_printf(name && *name ? ";\n" : ",\n");
}


void
trace_long_unsigned_real(const char *name, const unsigned long *vp)
{
    trace_printf("%s = %lu;\n", name, *vp);
}


void
trace_pointer_real(const char *name, const void *vptrptr)
{
    const void *ptr;
    const void *const *ptr_ptr = (const void *const *)vptrptr;

    ptr = *ptr_ptr;
    if (!ptr)
        trace_printf("%s = NULL;\n", name);
    else
        trace_printf("%s = 0x%p;\n", name, ptr);
}


void
trace_short_real(const char *name, const short *vp)
{
    trace_printf("%s = %hd;\n", name, *vp);
}


void
trace_short_unsigned_real(const char *name, const unsigned short *vp)
{
    trace_printf("%s = %hu;\n", name, *vp);
}


void
trace_string_real(const char *name, const nstring &value)
{
    trace_string_real(name, value.c_str());
}


void
trace_string_real(const char *name, const string_ty *value)
{
    trace_string_real(name, (value ? value->str_text : 0));
}


void
trace_string_real(const char *name, const char *vp)
{
    const char      *s;
    long            count;

    if (name && *name)
        trace_printf("%s = ", name);
    if (!vp)
    {
        trace_printf("NULL");
    }
    else
    {
        trace_printf("\"");
        count = 0;
        for (s = vp; *s; ++s)
        {
            switch (*s)
            {
            case '(':
            case '[':
            case '{':
                ++count;
                break;

            case ')':
            case ']':
            case '}':
                --count;
                break;
            }
        }
        if (count > 0)
            count = -count;
        else
            count = 0;
        for (s = vp; *s; ++s)
        {
            unsigned char c = *s;
            if (!isprint(c))
            {
                const char      *cp;

                cp = strchr("\bb\ff\nn\rr\tt", c);
                if (cp)
                    trace_printf("\\%c", cp[1]);
                else
                {
                    escape:
                    trace_printf("\\%03o", c);
                }
            }
            else
            {
                switch (c)
                {
                case '(':
                case '[':
                case '{':
                    ++count;
                    if (count <= 0)
                        goto escape;
                    break;

                case ')':
                case ']':
                case '}':
                    --count;
                    if (count < 0)
                        goto escape;
                    break;

                case '\\':
                case '"':
                    trace_printf("\\");
                    break;
                }
                trace_printf("%c", c);
            }
        }
        trace_printf("\"");
    }
    trace_printf(name && *name ? ";\n" : ",\n");
}


void
trace_indent_reset()
{
    depth = 0;
}


const char *
unctrl(int c)
{
    static char     buffer[30];

    if (c < 0 || c >= 256)
        snprintf(buffer, sizeof(buffer), "%d", c);
    else
    {
        int             cc;

        switch (c)
        {
        case '\a':
            cc = 'a';
            goto escaped;

        case '\b':
            cc = 'b';
            goto escaped;

        case '\f':
            cc = 'f';
            goto escaped;

        case '\n':
            cc = 'n';
            goto escaped;

        case '\r':
            cc = 'r';
            goto escaped;

        case '\t':
            cc = 't';
            goto escaped;

        case '\v':
            cc = 'v';
            goto escaped;


        case '\'':
        case '\\':
            cc = c;
            escaped:
            snprintf(buffer, sizeof(buffer), "0x%02X '\\%c'", c, cc);
            break;

        case ' ': case '!': case '"': case '#':
        case '$': case '%': case '&':
        case '(': case ')': case '*': case '+':
        case ',': case '-': case '.': case '/':
        case '0': case '1': case '2': case '3':
        case '4': case '5': case '6': case '7':
        case '8': case '9': case ':': case ';':
        case '<': case '=': case '>': case '?':
        case '@': case 'A': case 'B': case 'C':
        case 'D': case 'E': case 'F': case 'G':
        case 'H': case 'I': case 'J': case 'K':
        case 'L': case 'M': case 'N': case 'O':
        case 'P': case 'Q': case 'R': case 'S':
        case 'T': case 'U': case 'V': case 'W':
        case 'X': case 'Y': case 'Z': case '[':
                  case ']': case '^': case '_':
        case '`': case 'a': case 'b': case 'c':
        case 'd': case 'e': case 'f': case 'g':
        case 'h': case 'i': case 'j': case 'k':
        case 'l': case 'm': case 'n': case 'o':
        case 'p': case 'q': case 'r': case 's':
        case 't': case 'u': case 'v': case 'w':
        case 'x': case 'y': case 'z': case '{':
        case '|': case '}': case '~':
            snprintf(buffer, sizeof(buffer), "0x%02X '%c'", c, c);
            break;

        default:
            snprintf(buffer, sizeof(buffer), "0x%02X '\\%o'", c, c);
            break;
        }
    }
    return buffer;
}


void
trace_time_real(const char *name, long fake)
{
    if (name && *name)
        trace_printf("%s = ", name);
    time_t value = (time_t)fake;
    trace_printf("%ld /* %.24s */", fake, ctime(&value));
    trace_printf(name && *name ? ";\n" : ",\n");
}


void
trace_double_real(const char *name, const double &value)
{
    if (name && *name)
        trace_printf("%s = ", name);
    trace_printf("%g", value);
    trace_printf(name && *name ? ";\n" : ",\n");
}


// vim: set ts=8 sw=4 et :

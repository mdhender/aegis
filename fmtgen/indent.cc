//
// aegis - project change supervisor
// Copyright (C) 1991-1993, 1995, 1999, 2002-2006, 2008, 2012 Peter Miller
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

#include <common/ac/ctype.h>
#include <common/ac/errno.h>
#include <common/ac/stdio.h>
#include <common/ac/stdarg.h>

#include <common/error.h>
#include <fmtgen/indent.h>
#include <common/trace.h>


int indent::in_tab_width = 4;

//
// The Unix convention is 8, but users routinely set tabs in their
// editor to other weird values, making the formatting look wrong.
// Sigh.  So we don't use output tabs at all.
//
int indent::out_tab_width = 0;


indent::~indent(void)
{
    trace(("indent::~indent()\n{\n"));
    trace_pointer(fp);
    if (out_col)
        putch('\n');
    if (fflush(fp))
        nfatal("write %s", fn.quote_c().c_str());
    if (fp != stdout && fclose(fp))
        nfatal("close %s", fn.quote_c().c_str());
    fp = 0;
    fn.clear();
    depth = 0;
    in_col = 0;
    out_col = 0;
    continuation_line = 0;
    within_string = 0;
    trace(("}\n"));
}


indent::indent(const nstring &s) :
    fp(0),
    fn(s),
    depth(0),
    in_col(0),
    out_col(0),
    continuation_line(0),
    within_string(0)
{
    trace(("indent_open(s = %s)\n{\n", s.quote_c().c_str()));
    if (s.empty() || s == "-")
    {
        fp = stdout;
        fn = "(stdout)";
    }
    else
    {
        // We actually defer opening the file until we have some output
        // for it, to avoid creating any output if the parser finds
        // something wrong.
    }
    trace(("}\n"));
}


indent::pointer
indent::create(const nstring &filename)
{
    return pointer(new indent(filename));
}


void
indent::more()
{
    ++depth;
}


void
indent::less()
{
    if (depth > 0)
        --depth;
}


void
indent::putch(char c)
{
    if (!fp)
    {
        fp = fopen(fn.c_str(), "w");
        if (!fp)
            nfatal("open %s", fn.quote_c().c_str());
    }
    switch (c)
    {
    case '\n':
        if (!continuation_line)
            within_string = 0;
        putc('\n', fp);
#ifdef DEBUG
        fflush(fp);
#endif
        in_col = 0;
        out_col = 0;
        if (continuation_line == 1)
            continuation_line = 2;
        else
            continuation_line = 0;
        break;

    case ' ':
        if (out_col)
            ++in_col;
        break;

    case '\t':
        if (out_col)
            in_col = (in_col / in_tab_width + 1) * in_tab_width;
        break;

    case '"':
    case '\'':
        if (within_string == 0)
            within_string = c;
        else if (continuation_line != 1 && within_string == c)
            within_string = 0;
        goto normal;

    case '}':
    case ')':
    case ']':
        if (!within_string)
            --depth;
        // fall through

    default:
        normal:
        if (!out_col && c != '#' && continuation_line != 2)
            in_col += in_tab_width * depth;
        if (out_tab_width > 1)
        {
            for (;;)
            {
                if (out_col + 1 >= in_col)
                    break;
                int maybe = ((out_col / out_tab_width) + 1) * out_tab_width;
                if (maybe > in_col)
                    break;
                putc('\t', fp);
                out_col = maybe;
            }
        }
        while (out_col < in_col)
        {
            putc(' ', fp);
            ++out_col;
        }
        if (!within_string && (c == '{' || c == '(' || c == '['))
            ++depth;
        putc(c, fp);
        in_col++;
        out_col = in_col;
        continuation_line = (c == '\\');
        break;
    }
    if (ferror(fp))
        nfatal("write %s", fn.quote_c().c_str());
}


void
indent::printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
}


void
indent::vprintf(const char *fmt, va_list ap)
{
    char buffer[2000];
    vsnprintf(buffer, sizeof(buffer), fmt, ap);
    puts(buffer);
}


void
indent::puts(const char *buffer)
{
    for (const char *cp = buffer; *cp; ++cp)
        putch(*cp);
}


void
indent::puts(const nstring &s)
{
    const char *cp = s.c_str();
    const char *ep = cp + s.size();
    while (cp < ep)
        putch(*cp++);
}


void
indent::include_once(const nstring &filename)
{
    if (included_to_date.member(filename))
        return;
    included_to_date.push_back(filename);
    if (out_col)
        putch('\n');
    printf("#include <%s>\n", filename.c_str());
}


#include <common/nstring/accumulator.h>


void
indent::wrap_and_print(const nstring &prefix, const nstring &text)
{
    if (out_col)
        putch('\n');
    int line_length = 75 - (in_tab_width * depth) - prefix.size();
    if (line_length < 10)
        line_length = 10;
    const char *s = text.c_str();
    const char *s_end = s + text.size();
    int column = 0;
    nstring_accumulator word;
    while (s < s_end)
    {
        unsigned char c = *s++;
        if (isspace(c))
            continue;

        //
        // collect the word
        //
        word.clear();
        word.push_back(c);
        for (;;)
        {
            if (s >= s_end)
                break;
            c = *s;
            if (isspace(c))
                break;
            ++s;
            word.push_back(c);
        }
        nstring w = word.mkstr();

        if (column > 0 && column + 1 + int(w.size()) > line_length)
        {
            putch('\n');
            column = 0;
        }
        if (column == 0)
            puts(prefix);
        else
        {
            putch(' ');
            ++column;
        }
        puts(w);
        column += w.size();
    }
    if (column)
        putch('\n');
}


void
indent::set_indent(int n)
{
    if (n < 1)
        n = 1;
    else if (n > 8)
        n = 8;
    in_tab_width = n;
}


// vim: set ts=8 sw=4 et :

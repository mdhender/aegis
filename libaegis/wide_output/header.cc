//
//      aegis - project change supervisor
//      Copyright (C) 1999-2006, 2008 Peter Miller
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

#include <common/ac/stdio.h> // for sprintf
#include <common/ac/wchar.h>
#include <common/ac/wctype.h>

#include <common/language.h>
#include <common/mem.h>
#include <common/now.h>
#include <common/page.h> // for DEFAULT_PRINTER_LENGTH
#include <common/str.h>
#include <common/trace.h>
#include <common/wstring/accumulator.h>
#include <libaegis/wide_output.h>
#include <libaegis/wide_output/header.h>

//
// This is how you figure out whether you are talking to a printer,
// or to a terminal.
//
#define PRINTER_THRESHOLD (DEFAULT_PRINTER_LENGTH / 2)
// assert(PRINTER_THRESHOLD > DEFAULT_PAGE_LENGTH);


wide_output_header::~wide_output_header()
{
    trace(("wide_output_header::~wide_output_header(this = %08lX)\n{\n",
        (long)this));
    flush();
    trace(("}\n"));
}


wide_output_header::wide_output_header(const wide_output::pointer &a_deeper) :
    deeper(a_deeper),
    line_number(0),
    length(deeper->page_length()),
    width(deeper->page_width()),
    is_a_printer(length > PRINTER_THRESHOLD),
    column(0),
    page_number(0),
    page_time(now()),
    already_top_diverted(false)
{
    if (is_a_printer)
    {
        //
        // Three blank lines at top (avoid perforation)
        // two title lines,
        // one blank line before columns.
        // Plus, 3 blank lines at bottom (avoid perforation).
        //
        length -= 9;
    }
    else if (length > 5)
    {
        //
        // One blank line at top,
        // two title lines,
        // one blank line before columns.
        // Leave the last line for the pager
        //
        length -= 5;
    }
    else
        length = 1;
    trace(("}\n"));
}


wide_output_header::hpointer
wide_output_header::open(const wide_output::pointer &a_deeper)
{
    trace(("wide_output_header::open(deeper = %08lX)\n", (long)a_deeper.get()));
    return hpointer(new wide_output_header(a_deeper));
}


nstring
wide_output_header::filename()
{
    return deeper->filename();
}


static wstring
wstr_subset_of_max_width(const wstring &wis, int len)
{
    const wchar_t *begin = wis.c_str();
    const wchar_t *s = begin;
    int s_wid = 0;
    while (*s)
    {
        int c_wid = s_wid + wcwidth(*s);
        if (c_wid > len)
            break;
        ++s;
        s_wid = c_wid;
    }
    return wstring(begin, s - begin);
}


static wstring
wstr_spaces(int n)
{
    trace(("mark\n"));
    static wstring_accumulator buffer;
    buffer.clear();
    while (n > 0)
    {
        buffer.push_back(L' ');
        --n;
    }
    return buffer.mkstr();
}


void
wide_output_header::left_and_right(const wstring &lhs, const char *rhs)
{
    trace(("wide_output_header::left_and_right()\n{\n"));
    wstring tmp3(rhs);
    trace(("mark\n"));
    int w3 = tmp3.column_width();
    int frac = width - 2 - w3;
    trace(("mark\n"));
    wstring tmp1 = wstr_subset_of_max_width(lhs, frac);
    trace(("mark\n"));
    frac = width - tmp1.column_width() - w3;
    trace(("frac = %d\n", frac));
    wstring tmp2 = wstr_spaces(frac);
    trace(("mark\n"));
    deeper->write(tmp1);
    deeper->write(tmp2);
    deeper->write(tmp3);
    deeper->put_wc(L'\n');
    trace(("}\n"));
}


void
wide_output_header::top_of_page_processing()
{
    //
    // advance the page number
    //
    if (already_top_diverted)
        return;
    already_top_diverted = true;
    trace(("wide_output_header::top_of_page(this = %08lX)\n{\n", (long)this));
    ++page_number;

    //
    // Blank line(s) before the title
    //
    if (is_a_printer)
    {
        if (page_number > 1)
            deeper->put_wc(L'\f');
        deeper->put_wc(L'\n');
        deeper->put_wc(L'\n');
    }
    deeper->put_wc(L'\n');

    //
    // first line of titles
    //
    trace(("mark\n"));
    char tmp1[30];
    snprintf(tmp1, sizeof(tmp1), "Page %d", page_number);
    left_and_right(title1, tmp1);

    //
    // second line of titles
    //
    trace(("mark\n"));
    snprintf(tmp1, sizeof(tmp1), "%.24s", ctime(&page_time));
    left_and_right(title2, tmp1);

    //
    // blank line between titles and columns
    //
    trace(("mark\n"));
    deeper->put_wc(L'\n');

    already_top_diverted = false;
    trace(("}\n"));
}


void
wide_output_header::bottom_of_page_processing()
{
    trace(("bottom of page\n"));

    //
    // Flush the output here.  This means that the last page will be
    // available on the output immediately.  This can be important
    // for reports and lists which take a long to to generate.
    //
    deeper->flush();
}


void
wide_output_header::write_inner(const wchar_t *data, size_t len)
{
    //
    // If we see a form-feed, advance to the next page.
    // Do this by inserting the right number of newlines.
    // By doing it first, by recursion, the rest of the unexceptional
    // processing simply falls out cleanly.
    //
    trace(("wide_output_header::write_inner(this = %08lX, data = %08lX, "
        "len = %ld)\n{\n", (long)this, (long)data, (long)len));
    while (len > 0)
    {
        wchar_t wc = *data++;
        --len;
        if (wc == L'\f')
        {
            //
            // If we are at the top of the page,
            // emit the page header.
            //
            if
            (
                line_number == 0
            &&
                column == 0
            &&
                !already_top_diverted
            )
                top_of_page_processing();
            for (;;)
            {
                //
                // Forward the character to the output.
                // Keep track of the column for eoln()
                // purposes; it doesn't need to be exact,
                // so we aren't using wcwidth.
                //
                deeper->put_wc(L'\n');

                //
                // Keep track of the line number so
                // that we can work out when we get to
                // the end of the page (and implicitly,
                // the top of the next page).
                //
                column = 0;
                ++line_number;
                trace(("linum = %d;\n", line_number));
                if (line_number >= length)
                {
                    bottom_of_page_processing();
                    line_number = 0;
                    break;
                }
            }
            continue;
        }

        //
        // If we are at the top of the page,
        // emit the page header.
        //
        if (line_number == 0 && column == 0 && !already_top_diverted)
            top_of_page_processing();

        //
        // Forward the character to the output.
        // Keep track of the column for eoln() purposes;
        // it doesn't need to be exact, so we aren't using wcwidth.
        //
        deeper->put_wc(wc);
        ++column;

        //
        // If the character was a newline, keep track of the line number
        // so that we can work out when we get to the end of the page
        // (and implicitly, the top of the next page).
        //
        if (wc == L'\n')
        {
            column = 0;
            ++line_number;
            trace(("line_number = %d;\n", line_number));
            if (line_number >= length)
            {
                bottom_of_page_processing();
                line_number = 0;
            }
        }
    }
    trace(("}\n"));
}


int
wide_output_header::page_width()
{
    trace(("wide_output_header::page_width(this = %08lX)\n", (long)this));
    return width;
}


void
wide_output_header::flush_inner()
{
    trace(("wide_output_header::flush_inner(this = %08lX)\n{\n", (long)this));
    deeper->flush();
    trace(("}\n"));
}


int
wide_output_header::page_length()
{
    trace(("wide_output_header::page_length(this = %08lX)\n", (long)this));
    return length;
}


void
wide_output_header::end_of_line_inner()
{
    trace(("wide_output_header::eoln_inner(this = %08lX)\n{\n", (long)this));
    if (column > 0)
        put_wc(L'\n');
    trace(("}\n"));
}


static wstring
censor(const char *s)
{
    //
    // convert to a wide string
    //
    wstring wis(s);

    //
    // make sure it doesn't have any unreasonable characters
    //
    language_human();
    size_t j = 0;
    for (; j < wis.size(); ++j)
        if (!iswprint(wis.c_str()[j]))
            break;
    language_C();

    //
    // Truncate at the first unprintable character,
    // if any are present
    //
    if (j < wis.size())
    {
        return wstring(wis.c_str(), j);
    }

    //
    // return result
    //
    return wis;
}


void
wide_output_header::title(const char *t1, const char *t2)
{
    if (!t1)
        t1 = "";
    if (!t2)
        t2 = "";
    trace(("wide_output_header::title(this = %08lX, \"%s\", \"%s\")\n{\n",
        (long)this, t1, t2));
    title1 = censor(t1);
    title2 = censor(t2);
    trace(("}\n"));
}


void
wide_output_header::need(int nlines)
{
    if (nlines <= 0)
        return;
    trace(("wide_output_header::need(this = %08lX, nlines = %d)\n{\n",
        (long)this, nlines));
    flush();
    if (line_number > 0 && line_number + nlines > length)
        put_wc(L'\f');
    trace(("}\n"));
}


void
wide_output_header::need1(int nlines)
{
    if (nlines <= 0)
        return;
    trace(("wide_output_header::need1(this = %08lX, nlines = %d)\n{\n",
        (long)this, nlines));
    flush();
    if (line_number > 0)
    {
        if (line_number + nlines > length)
            put_wc(L'\f');
        else
            put_wc(L'\n');
    }
    trace(("}\n"));
}


void
wide_output_header::eject()
{
    trace(("wide_output_header::eject(this = %08lX)\n{\n", (long)this));
    if (column > 0)
        put_wc(L'\n');
    if (line_number > 0)
        put_wc(L'\f');
    trace(("}\n"));
}


bool
wide_output_header::is_at_top_of_page()
{
    flush();
    return (line_number == 0 && column == 0);
}


const char *
wide_output_header::type_name()
    const
{
    return "wide_output_header";
}

//
//      aegis - project change supervisor
//      Copyright (C) 1999-2006, 2008, 2012 Peter Miller
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

#include <common/ac/wchar.h>
#include <common/ac/wctype.h>

#include <common/language.h>
#include <common/mem.h>
#include <common/str.h>
#include <common/trace.h>
#include <common/wstring/accumulator.h>
#include <libaegis/option.h>
#include <libaegis/wide_output.h>
#include <libaegis/wide_output/unexpand.h>


wide_output_unexpand::~wide_output_unexpand()
{
    flush();
}


wide_output_unexpand::wide_output_unexpand(const wide_output::pointer &a_deeper,
        int a_tab_width) :
    deeper(a_deeper),
    icol(0),
    ocol(0),
    tab_width(a_tab_width > 0 ? a_tab_width : option_tab_width_get())
{
}


wide_output::pointer
wide_output_unexpand::open(const wide_output::pointer &a_deeper, int a_tabwidth)
{
    return pointer(new wide_output_unexpand(a_deeper, a_tabwidth));
}


nstring
wide_output_unexpand::filename()
{
    return deeper->filename();
}


void
wide_output_unexpand::write_inner(const wchar_t *data, size_t len)
{
    //
    // Put all of the output into a stash, rather then direct calls
    // to deeper->put_wc().  This allows us to minimize
    // the number of language_C() and language_human() calls, which
    // tend to be slow-ish
    //
    wstring_accumulator stash;

    trace(("wide_output_unexpand::write_inner(this = %p, data = %p, "
        "len = %ld)\n{\n", this, data, (long)len));
    language_human();
    while (len > 0)
    {
        if (stash.size() > 2000)
        {
            language_C();
            deeper->write(stash.get_data(), stash.size());
            stash.clear();
            language_human();
        }

        wchar_t wc = *data++;
        --len;
        switch (wc)
        {
        case L'\n':
        case L'\f':
            stash.push_back(wc);
            icol = 0;
            ocol = 0;
            break;

        case L'\t':
            // internal tabs are 8 characters wide
            icol = (icol + 8) & ~7;
            break;

        case (wchar_t)0:
        case L' ':
            icol++;
            break;

        default:
            trace(("icol = %d\n", icol));
            if (tab_width >= 2)
            {
                trace(("tab_width = %d\n", tab_width));
                for (;;)
                {
                    trace(("ocol = %d\n", ocol));
                    if (ocol + 1 >= icol)
                        break;
                    int ncol = ((ocol / tab_width) + 1) * tab_width;
                    trace(("ncol = %d\n", ncol));
                    if (ncol > icol)
                        break;
                    stash.push_back(L'\t');
                    ocol = ncol;
                }
            }
            while (ocol < icol)
            {
                trace(("ocol = %d\n", ocol));
                stash.push_back(L' ');
                ocol++;
            }
            trace(("ocol = %d\n", ocol));
            icol += wcwidth(wc);
            stash.push_back(wc);
            trace(("icol = %d\n", icol));
            ocol = icol;
            break;
        }
    }
    language_C();
    if (!stash.empty())
    {
        deeper->write(stash.get_data(), stash.size());
    }
    trace(("}\n"));
}


void
wide_output_unexpand::flush_inner()
{
    deeper->flush();
}


int
wide_output_unexpand::page_width()
{
    return deeper->page_width();
}


int
wide_output_unexpand::page_length()
{
    return deeper->page_length();
}


void
wide_output_unexpand::end_of_line_inner()
{
    trace(("wide_output_unexpand::end_of_line_inner(this = %p)\n{\n",
        this));
    if (icol > 0)
        put_wc(L'\n');
    trace(("}\n"));
}


const char *
wide_output_unexpand::type_name()
    const
{
    return "wide_output_unexpand";
}


// vim: set ts=8 sw=4 et :

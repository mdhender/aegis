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
#include <libaegis/wide_output/expand.h>


wide_output_expand::~wide_output_expand()
{
    trace(("wide_output_expand::destructor(this = %p)\n{\n", this));
    flush();
    trace(("}\n"));
}


wide_output_expand::wide_output_expand(const wide_output::pointer &a_deeper) :
    deeper(a_deeper),
    column(0)
{
    trace(("wide_output_expand::wide_output_expand(this = %p, "
        "deeper = %p)\n{\n", this, deeper.get()));
    trace(("}\n"));
}


wide_output::pointer
wide_output_expand::open(const wide_output::pointer &a_deeper)
{
    return pointer(new wide_output_expand(a_deeper));
}


nstring
wide_output_expand::filename()
{
    return deeper->filename();
}


void
wide_output_expand::put_hex(int n)
{
    deeper->put_wc((wchar_t)("0123456789ABCDEF"[n & 15]));
}


void
wide_output_expand::write_inner(const wchar_t *data, size_t len)
{
    trace(("wide_output_expand::write_inner(this = %p, data = %p, "
        "len = %ld)\n{\n", this, data, (long)len));
    while (len > 0)
    {
        wchar_t wc = *data++;
        --len;

        switch (wc)
        {
        case L'\n':
        case L'\f':
            deeper->put_wc(wc);
            column = 0;
            break;

        case L'\t':
            // internally, treat tabs as 8 characters wide
            for (;;)
            {
                deeper->put_wc(L' ');
                column++;
                if (!(column & 7))
                    break;
            }
            break;

        case (wchar_t)0:
        case L' ':
            deeper->put_wc(L' ');
            column++;
            break;

        default:
            language_human();
            if (!iswprint(wc))
            {
                language_C();
                deeper->put_wc(L'\\');
                deeper->put_wc(L'x');
                put_hex((int)(wc >> 12));
                put_hex((int)(wc >>  8));
                put_hex((int)(wc >>  4));
                put_hex((int)(wc      ));
                column += 6;
            }
            else
            {
                column += wcwidth(wc);
                language_C();
                deeper->put_wc(wc);
            }
            break;
        }
    }
    trace(("}\n"));
}


void
wide_output_expand::flush_inner()
{
    trace(("wide_output_expand::flush(this = %p)\n{\n", this));
    deeper->flush();
    trace(("}\n"));
}


int
wide_output_expand::page_width()
{
    return deeper->page_width();
}


int
wide_output_expand::page_length()
{
    return deeper->page_length();
}


void
wide_output_expand::end_of_line_inner()
{
    trace(("wide_output_expand::end_of_line_inner(this = %p)\n{\n",
        this));
    if (column > 0)
        put_wc(L'\n');
    trace(("}\n"));
}


const char *
wide_output_expand::type_name()
    const
{
    return "wide_output_expand";
}


// vim: set ts=8 sw=4 et :

//
//      aegis - project change supervisor
//      Copyright (C) 1999, 2001-2008, 2012 Peter Miller
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

#include <common/language.h>
#include <common/page.h>
#include <common/str.h>
#include <common/trace.h>
#include <libaegis/wide_output/column.h>


wide_output_column::~wide_output_column()
{
    trace(("wide_output_column::destructor(this = %p)\n{\n", this));
    flush();
    for (size_t j = 0; j < nrows; ++j)
    {
        column_row_ty *rp = row + j;
        if (rp->text)
        {
            delete [] rp->text;
            rp->text = 0;
        }
    }
    delete [] row;
    row = 0;
    nrows = 0;
    nrows_max = 0;
    trace(("}\n"));
}


wide_output_column::wide_output_column(int a_width, int a_length) :
    width(a_width <= 0 ? page_width_get(-1) : a_width),
    length(a_length <= 0 ? page_length_get(-1) : a_length),
    row(0),
    nrows(0),
    nrows_max(0)
{
}


wide_output_column::cpointer
wide_output_column::open(int a_width, int a_length)
{
    return cpointer(new wide_output_column(a_width, a_length));
}


nstring
wide_output_column::filename()
{
    return "(column)";
}


void
wide_output_column::write_inner(const wchar_t *buf, size_t buflen)
{
    trace(("wide_output_column::write(this = %p, buf = %p, len = %ld)\n"
        "{\n", this, buf, (long)buflen));
    language_human();
    while (buflen > 0)
    {
        wchar_t wc = *buf++;
        --buflen;

        //
        // newlines simply advance down the column
        //
        if (wc == L'\n')
            ++nrows;

        //
        // Make sure we have a row structure to park the
        // character in.
        //
        while (nrows >= nrows_max)
        {
            //
            // The nrows_max value can easily be beyond the
            // end of the array.  For example, if the text starts with
            // a couple of newlines.  You must be careful about array
            // bounds here.
            //
            size_t prev = nrows_max;
            for (;;)
            {
                nrows_max = 4 + 2 * nrows_max;
                if (nrows < nrows_max)
                    break;
            }

            column_row_ty *new_row =
                new column_row_ty [nrows_max];
            for (size_t k = 0; k < prev; ++k)
                new_row[k] = row[k];
            delete [] row;
            row = new_row;

            for (size_t j = prev; j < nrows_max; ++j)
            {
                column_row_ty *rp = row + j;
                rp->length_max = 0;
                rp->length = 0;
                rp->text = 0;
                rp->printing_width = 0;
            }
        }

        if (wc == L'\n')
            continue;

        //
        // Make sure we have text space enough to park the character.
        //
        column_row_ty *rp = row + nrows;
        while (rp->length >= rp->length_max)
        {
            rp->length_max = 16 + 2 * rp->length_max;
            wchar_t *new_text = new wchar_t [rp->length_max ];
            for (size_t k = 0; k < rp->length; ++k)
                new_text[k] = rp->text[k];
            delete [] rp->text;
            rp->text = new_text;
        }

        //
        // Remember the character,
        // and keep a running tally of the printing width.
        //
        rp->text[rp->length++] = wc;
        rp->printing_width += wcwidth(wc);
    }
    language_C();
    trace(("}\n"));
}


int
wide_output_column::page_width()
{
    return width;
}


void
wide_output_column::flush_inner()
{
}


int
wide_output_column::page_length()
{
    return length;
}


void
wide_output_column::end_of_line_inner()
{
    trace(("wide_output_column::end_of_line_inner(this = %p)\n{\n",
        this));
    if (nrows > 0 && nrows < nrows_max)
    {
        column_row_ty *rp = row + nrows;
        if (rp->length > 0)
            put_wc(L'\n');
    }
    trace(("}\n"));
}


column_row_ty *
wide_output_column::get(int n)
{
    trace(("wide_output_column::get(this = %p, n = %d)\n{\n",
            this, n));
    if (n < 0)
    {
        trace(("return NULL;\n"));
        trace(("}\n"));
        return 0;
    }
    if (n >= (int)nrows_max)
    {
        trace(("return NULL;\n"));
        trace(("}\n"));
        return 0;
    }
    column_row_ty *rp = row + n;
    if (rp->length == 0)
    {
        //
        // See if all rows after this one are empty, too.
        // If they are, pretend this row doesn't exist.
        //
        size_t k;
        bool end_of_the_road = true;
        for (k = n + 1; k < nrows; ++k)
        {
            if (row[k].length)
            {
                end_of_the_road = false;
                break;
            }
        }
        if (end_of_the_road)
        {
            trace(("return NULL;\n"));
            trace(("}\n"));
            return 0;
        }
    }
    trace(("return %p;\n", rp));
    trace(("}\n"));
    return rp;
}


void
wide_output_column::clear_buffers()
{
    trace(("wide_output_column::reset(this = %p)\n{\n", this));
    for (size_t j = 0; j < nrows_max; ++j)
    {
        row[j].length = 0;
        row[j].printing_width = 0;
    }
    nrows = 0;
    trace(("}\n"));
}


const char *
wide_output_column::type_name()
    const
{
    return "wide_output_column";
}


// vim: set ts=8 sw=4 et :

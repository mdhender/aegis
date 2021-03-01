//
// aegis - project change supervisor
// Copyright (C) 2011, 2012 Peter Miller
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
// You should have received a copy of the GNU General Public License along
// with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/ctype.h>

#include <common/trace.h>
#include <libaegis/output/filter/wrap_make.h>


output_filter_wrap_make::~output_filter_wrap_make()
{
    trace(("~output_filter_wrap_make(this = %p)\n", this));
    flush();
    end_of_line_inner();
    trace(("}\n"));
}


output_filter_wrap_make::output_filter_wrap_make(
    const output::pointer &a_deeper
) :
    output_filter(a_deeper),
    column(0),
    tab(false)
{
    trace(("output_filter_wrap_make(this = %p)\n", this));
}


output_filter_wrap_make::pointer
output_filter_wrap_make::create(const output::pointer &a_deeper)
{
    return pointer(new output_filter_wrap_make(a_deeper));
}


nstring
output_filter_wrap_make::type_name(void)
    const
{
    return ("wrap_make " + output_filter::type_name());
}


void
output_filter_wrap_make::write_inner(const void *data, size_t data_size)
{
    trace(("output_filter_wrap_make::write_inner(this = %p, data = %p, "
        "data_size = %ld)\n", this, data, data_size));
    const char *cp = (const char *)data;
    const char *end = cp + data_size;
    while (cp < end)
    {
        unsigned char c = *cp++;
        if (column == 0 && c == '\t')
        {
            tab = true;
            column = 8;
            continue;
        }

        // comments are special
        if (!word.empty() && word[0] == '#')
        {
            if (c == '\n')
            {
                if (!word.empty())
                {
                    line.push_back(word.mkstr());
                    word.clear();
                }
                write_out_the_line();
            }
            else
            {
                word.push_back(c);
                ++column;
            }
            continue;
        }

        if (isspace(c))
        {
            if (!word.empty())
            {
                line.push_back(word.mkstr());
                word.clear();
            }
            if (c == '\n')
            {
                write_out_the_line();
                column = 0;
            }
            else
            {
                ++column;
            }
        }
        else
        {
            word.push_back(c);
            ++column;
        }
    }
    trace(("}\n"));
}


void
output_filter_wrap_make::end_of_line_inner(void)
{
    if (column)
    {
        write_inner("\n", 1);
    }
}


void
output_filter_wrap_make::write_out_the_line(void)
{
    trace(("output_filter_wrap_make::write_out_the_line(this = %p)\n{\n",
        this));
    unsigned width = output_filter::page_width();
    unsigned ocol = 0;
    if (tab && !line.empty())
    {
        deeper_fputc('\t');
        ocol = 8;
        tab = false;
    }
    for (size_t j = 0; j < line.size(); ++j)
    {
        nstring w = line[j];
        if (j)
        {
            unsigned w2 = width;
            if (j + 1 < line.size())
            {
                //assert(w2 >= 2);
                w2 -= 2;
            }
            if (ocol + 1 + w.size() > w2)
            {
                deeper_fputs(" \\\n");
                ocol = 0;
                int want = (int)w2 - (int)w.size();
                if (want < 0)
                    want = 0;
                else if (want > 16)
                    want = 16;
                while (want >= 8)
                {
                    deeper_fputc('\t');
                    ocol += 8;
                    want -= 8;
                }
                while (want > 0)
                {
                    deeper_fputc(' ');
                    ++ocol;
                    --want;
                }
            }
            else
            {
                deeper_fputc(' ');
                ++ocol;
            }
        }
        deeper_fputs(w);
        ocol += w.size();
    }
    deeper_fputc('\n');

    column = 0;
    line.clear();
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :

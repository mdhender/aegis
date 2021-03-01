//
// aegis - project change supervisor
// Copyright (C) 2011 Peter Miller
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
#include <libaegis/output/filter/wrap_simple.h>


output_filter_wrap_simple::~output_filter_wrap_simple()
{
    trace(("~output_filter_wrap_simple(this = %p)\n{\n", this));
    flush();
    trace(("mark\n"));
    end_of_line_inner();
    trace(("}\n"));
}


output_filter_wrap_simple::output_filter_wrap_simple(
    const output::pointer &a_deeper
) :
    output_filter(a_deeper),
    icol(0),
    ocol(0)
{
    trace(("output_filter_wrap_simple(this = %p)\n", this));
}


output_filter_wrap_simple::pointer
output_filter_wrap_simple::create(const output::pointer &a_deeper)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return pointer(new output_filter_wrap_simple(a_deeper));
}


void
output_filter_wrap_simple::write_inner(const void *data, size_t data_size)
{
    trace(("output_filter_wrap_simple::write_inner(this = %p, data = %p, "
        "data_size = %ld)\n{\n", this, data, data_size));
    unsigned width = output_filter::page_width();
    const char *cp = (const char *)data;
    const char *end = cp + data_size;
    while (cp < end)
    {
        unsigned char c = *cp++;
        if (isspace(c))
        {
            if (!word.empty())
            {
                nstring w = word.mkstr();
                trace(("w = %s\n", w.quote_c().c_str()));
                word.clear();
                if (ocol)
                {
                    if (ocol + 1 + w.size() > width)
                    {
                        deeper_fputc('\n');
                        ocol = 0;
                    }
                    else
                    {
                        deeper_fputc(' ');
                        ++ocol;
                    }
                }
                deeper_fputs(w);
                ocol += w.size();
                trace(("ocol = %d\n", ocol));
            }
            if (c == '\n')
            {
                deeper_fputc('\n');
                ocol = 0;
                icol = 0;
            }
            else
            {
                ++icol;
            }
        }
        else
        {
            word.push_back(c);
            ++icol;
        }
    }
    trace(("}\n"));
}


void
output_filter_wrap_simple::end_of_line_inner(void)
{
    trace(("output_filter_wrap_simple::end_of_line_inner(this = %p)\n{\n",
        this));
    if (icol)
        write_inner("\n", 1);
    trace(("}\n"));
}


nstring
output_filter_wrap_simple::type_name(void)
    const
{
    return ("wrap_simple " + output_filter::type_name());
}


// vim: set ts=8 sw=4 et :

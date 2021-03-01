//
// aegis - project change supervisor
// Copyright (C) 2009, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published
// by the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
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
#include <common/ac/string.h>

#include <libaegis/input/multipart.h>


input_multipart::~input_multipart()
{
}


input_multipart::input_multipart(input &a_deeper, const nstring &a_boundary) :
    deeper(a_deeper),
    boundary1("--" + a_boundary),
    boundary2("--" + a_boundary + "--"),
    state(0)
{
}


input
input_multipart::create(input &a_deeper, const nstring &a_boundary)
{
    return new input_multipart(a_deeper, a_boundary);
}


input_multipart::line_type_t
input_multipart::line_type(const nstring &line)
{
    if (line.empty())
        return line_type_empty;
    nstring tline = line.trim_right();
    if (tline == boundary1)
        return line_type_boundary;
    if (tline == boundary2)
        return line_type_last_boundary;
    return line_type_content;
}


ssize_t
input_multipart::read_inner(void *data, size_t len)
{
    assert(len > 0);
    switch (state)
    {
    case 0:
        // looking for initial boundary
        for (;;)
        {
            nstring line;
            if (!deeper->one_line(line))
            {
                state = 2;
                return 0;
            }
            switch (line_type(line))
            {
            case line_type_content:
            case line_type_empty:
                continue;

            case line_type_boundary:
                break;

            case line_type_last_boundary:
                state = 2;
                return 0;
            }
            break;
        }
        spos = deeper->ftell();
        state = 1;
        // fall through...

    case 1:
        // read data until boundary
        {
            nstring line;
            if (!deeper->one_line(line))
            {
                state = 2;
                return 0;
            }
            switch (line_type(line))
            {
            case line_type_content:
            case line_type_empty:
                line += "\n";
                if (len < line.size())
                {
                    memcpy(data, line.c_str(), len);
                    deeper->unread(line.c_str() + len, line.size() - len);
                    return len;
                }
                memcpy(data, line.c_str(), line.size());
                return line.size();

            case line_type_boundary:
            case line_type_last_boundary:
                state = 2;
                return 0;
            }
        }
        break;

    case 2:
    default:
        // end of data
        break;
    }
    return 0;
}


off_t
input_multipart::ftell_inner()
{
    return (deeper->ftell() - spos);
}


nstring
input_multipart::name()
{
    return deeper->name();
}


off_t
input_multipart::length()
{
    return -1;
}


void
input_multipart::keepalive()
{
    deeper->keepalive();
}


bool
input_multipart::is_remote()
    const
{
    return deeper->is_remote();
}


// vim: set ts=8 sw=4 et :

//
// aegis - project change supervisor
// Copyright (C) 1999, 2001, 2003-2008, 2010, 2012 Peter Miller
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

#include <common/mem.h>
#include <libaegis/input/crlf.h>
#include <libaegis/input/file.h>
#include <libaegis/input/file_text.h>


input
input_file_text_open(string_ty *fn, bool esc_nl)
{
    input temp(input_file_open(fn));
    return input_crlf::create(temp, esc_nl);
}


input
input_file_text_open(const nstring &fn, bool esc_nl)
{
    input temp(input_file_open(fn));
    return input_crlf::create(temp, esc_nl);
}


// vim: set ts=8 sw=4 et :

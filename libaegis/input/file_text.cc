//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2001, 2003-2007 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to read input from text files
//

#include <libaegis/input/crlf.h>
#include <libaegis/input/file.h>
#include <libaegis/input/file_text.h>


input
input_file_text_open(string_ty *fn, bool esc_nl)
{
    input temp(input_file_open(fn));
    return new input_crlf(temp, esc_nl);
}


input
input_file_text_open(const nstring &fn, bool esc_nl)
{
    input temp(input_file_open(fn));
    return new input_crlf(temp, esc_nl);
}

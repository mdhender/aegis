//
//	aegis - project change supervisor
//	Copyright (C) 2004 Peter Miller;
//	All rights reserved.
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
// MANIFEST: implementation of the nstring_list class
//

#pragma implementation "nstring_list"

#include <nstring_list.h>


nstring_list &
nstring_list::operator=(const nstring_list &arg)
{
    if (&arg != this)
    {
	clear();
	for (size_t j = 0; j < arg.content.nstrings; ++j)
	    string_list_append(&content, arg.content.string[j]);
    }
    return *this;
}


void
nstring_list::clear()
{
    while (content.nstrings)
    {
	content.nstrings--;
	str_free(content.string[content.nstrings]);
    }
}


nstring
nstring_list::operator[](int n)
    const
{
    if (n < 0 || (size_t)n >= content.nstrings)
	return nstring();
    return nstring(content.string[n]);
}

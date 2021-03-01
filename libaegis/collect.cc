//
//	aegis - project change supervisor
//	Copyright (C) 2004, 2005 Peter Miller;
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
// MANIFEST: implementation of the collect class
//

#include <common/ac/string.h>

#include <libaegis/collect.h>
#include <common/wstr.h>


collect::~collect()
{
    delete [] buf;
}


collect::collect() :
    pos(0),
    size(0),
    buf(0)
{
}


collect::collect(const collect &arg) :
    pos(0),
    size(0),
    buf(0)
{
    append(arg.buf, arg.size);
}


collect &
collect::operator=(const collect &arg)
{
    if (this != &arg)
    {
	pos = 0;
	append(arg.buf, arg.size);
    }
    return *this;
}


void
collect::append(wchar_t c)
{
    if (pos >= size)
    {
	size_t new_size = size * 2 + 8;
	wchar_t *new_buf = new wchar_t[new_size];
	if (pos)
	    memcpy(new_buf, buf, pos * sizeof(wchar_t));
	delete [] buf;
	size = new_size;
	buf = new_buf;
    }
    buf[pos++] = c;
}


void
collect::append(const wchar_t *s, size_t n)
{
    while (n > 0)
    {
	append(*s++);
	--n;
    }
}


wstring_ty *
collect::end()
{
    wstring_ty *result = wstr_n_from_wc(buf, pos);
    pos = 0;
    return result;
}

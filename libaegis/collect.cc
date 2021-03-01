//
//	aegis - project change supervisor
//	Copyright (C) 2004-2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <common/ac/string.h>

#include <common/mem.h>
#include <common/wstring.h>
#include <libaegis/collect.h>


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


void
collect::push_back(const wstring &s)
{
    append(s.c_str(), s.size());
}


wstring
collect::end()
{
    wstring result(buf, pos);
    pos = 0;
    return result;
}

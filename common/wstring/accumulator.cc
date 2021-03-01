//
// aegis - project change supervisor
// Copyright (C) 2008 Peter Miller
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

#include <common/ac/string.h>

#include <common/wstring/accumulator.h>



wstring_accumulator::~wstring_accumulator()
{
    delete [] buffer;
    buffer = 0;
    length = 0;
    maximum = 0;
}


wstring_accumulator::wstring_accumulator() :
    length(0),
    maximum(0),
    buffer(0)
{
}


wstring_accumulator::wstring_accumulator(const wstring_accumulator &arg) :
    length(0),
    maximum(0),
    buffer(0)
{
    push_back(arg);
}


wstring_accumulator &
wstring_accumulator::operator=(const wstring_accumulator &arg)
{
    if (this != &arg)
    {
	clear();
	push_back(arg);
    }
    return *this;
}


wstring
wstring_accumulator::mkstr()
    const
{
    return wstring(buffer, length);
}


void
wstring_accumulator::overflow(wchar_t c)
{
    if (length >= maximum)
    {
	size_t new_maximum = maximum * 2 + 16;
	wchar_t *new_buffer = new wchar_t [new_maximum];
	if (length)
	    memcpy(new_buffer, buffer, length * sizeof(buffer[0]));
	delete [] buffer;
	buffer = new_buffer;
	maximum = new_maximum;
    }
    buffer[length++] = c;
}


void
wstring_accumulator::push_back(const wstring_accumulator &arg)
{
    push_back(arg.buffer, arg.length);
}


void
wstring_accumulator::push_back(const wchar_t *cp, size_t n)
{
    if (!n)
	return;
    if (length + n > maximum)
    {
	size_t new_maximum = maximum;
	for (;;)
	{
	    new_maximum = new_maximum * 2 + 16;
	    if (length + n <= new_maximum)
		break;
	}
	wchar_t *new_buffer = new wchar_t [new_maximum];
	if (length)
	    memcpy(new_buffer, buffer, length);
	delete [] buffer;
	buffer = new_buffer;
	maximum = new_maximum;
    }
    memcpy(buffer + length, cp, n);
    length += n;
}


void
wstring_accumulator::push_back(const wchar_t *s)
{
    const wchar_t *e = s;
    while (*e)
        ++e;
    push_back(s, e - s);
}


void
wstring_accumulator::push_back(const wstring &s)
{
    push_back(s.c_str(), s.size());
}

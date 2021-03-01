//
//	aegis - project change supervisor
//	Copyright (C) 2004-2006, 2008 Peter Miller
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
#include <common/nstring/accumulator.h>


nstring_accumulator::~nstring_accumulator()
{
    delete [] buffer;
    buffer = 0;
    length = 0;
    maximum = 0;
}


nstring_accumulator::nstring_accumulator() :
    length(0),
    maximum(0),
    buffer(0)
{
}


nstring_accumulator::nstring_accumulator(const nstring_accumulator &arg) :
    length(0),
    maximum(0),
    buffer(0)
{
    push_back(arg);
}


nstring_accumulator &
nstring_accumulator::operator=(const nstring_accumulator &arg)
{
    if (this != &arg)
    {
	clear();
	push_back(arg);
    }
    return *this;
}


nstring
nstring_accumulator::mkstr()
    const
{
    return nstring(buffer, length);
}


void
nstring_accumulator::overflow(char c)
{
    if (length >= maximum)
    {
	size_t new_maximum = maximum * 2 + 16;
	char *new_buffer = new char [new_maximum];
	if (length)
	    memcpy(new_buffer, buffer, length);
	delete [] buffer;
	buffer = new_buffer;
	maximum = new_maximum;
    }
    buffer[length++] = c;
}


void
nstring_accumulator::push_back(const nstring_accumulator &arg)
{
    push_back(arg.buffer, arg.length);
}


void
nstring_accumulator::push_back(const char *cp, size_t n)
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
	char *new_buffer = new char [new_maximum];
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
nstring_accumulator::push_back(const char *s)
{
    push_back(s, strlen(s));
}


void
nstring_accumulator::push_back(const nstring &s)
{
    push_back(s.c_str(), s.size());
}

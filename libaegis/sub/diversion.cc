//
//	aegis - project change supervisor
//	Copyright (C) 2004-2007 Peter Miller
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
// MANIFEST: implementation of the sub_diversion class
//

#include <common/error.h> // for assert
#include <libaegis/sub/diversion.h>
#include <common/wstr.h>


sub_diversion::~sub_diversion()
{
}


sub_diversion::sub_diversion(const wstring &arg1, bool arg2) :
    pos(0),
    text(arg1),
    resubstitute(arg2)
{
}


sub_diversion::sub_diversion() :
    pos(0),
    resubstitute(false)
{
}


sub_diversion::sub_diversion(const sub_diversion &arg) :
    pos(arg.pos),
    text(arg.text),
    resubstitute(arg.resubstitute)
{
}


sub_diversion &
sub_diversion::operator=(const sub_diversion &arg)
{
    if (this != &arg)
    {
	text = arg.text;
	pos = arg.pos;
	resubstitute = arg.resubstitute;
    }
    return *this;
}


wchar_t
sub_diversion::getch()
{
    if (pos >= text.size())
	return 0;
    return text.c_str()[pos++];
}


void
sub_diversion::ungetch(wchar_t c)
{
    if (c == 0)
	return;
    assert(!text.empty());
    assert(pos >= 1);
    --pos;
    assert(c == text.c_str()[pos]);
}

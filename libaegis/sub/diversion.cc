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
// MANIFEST: implementation of the sub_diversion class
//

#pragma implementation "sub_diversion"

#include <error.h> // for assert
#include <sub/diversion.h>
#include <wstr.h>


sub_diversion::~sub_diversion()
{
    if (text)
	wstr_free(text);
}


sub_diversion::sub_diversion(wstring_ty *arg1, bool arg2) :
    pos(0),
    text(wstr_copy(arg1)),
    resubstitute(arg2)
{
}


sub_diversion::sub_diversion() :
    pos(0),
    text(0),
    resubstitute(false)
{
}


sub_diversion::sub_diversion(const sub_diversion &arg) :
    pos(arg.pos),
    text(wstr_copy(arg.text)),
    resubstitute(arg.resubstitute)
{
}


sub_diversion &
sub_diversion::operator=(const sub_diversion &arg)
{
    if (this != &arg)
    {
	wstr_free(text);
	text = wstr_copy(arg.text);
	pos = arg.pos;
	resubstitute = arg.resubstitute;
    }
    return *this;
}


wchar_t
sub_diversion::getch()
{
    if (!text)
	return 0;
    if (pos >= text->wstr_length)
	return 0;
    return text->wstr_text[pos++];
}


void
sub_diversion::ungetch(wchar_t c)
{
    if (c == 0)
	return;
    assert(text);
    assert(pos >= 1);
    --pos;
    assert(c == text->wstr_text[pos]);
}

//
//	aegis - project change supervisor
//	Copyright (C) 1996, 1998, 2004-2008 Peter Miller
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

#include <common/ac/stddef.h>
#include <common/ac/ctype.h>
#include <common/ac/wctype.h>


#ifndef HAVE_ISWPRINT

#ifdef iswprint
#undef iswprint
#endif

int
iswprint(wint_t c)
{
    //
    // Assume characters over 256 are printable.  Assume characters
    // under 256 are either ASCII or Latin-1.  These are dumb
    // assumptions, but real i18n support will provide a real
    // iswprint function.
    //
    return (c >= 256 || isprint((unsigned char)c));
}


#ifdef iswspace
#undef iswspace
#endif

int
iswspace(wint_t c)
{
    //
    // Assume characters over 256 are letters.  Assume characters
    // under 256 are either ASCII or Latin-1.  These are dumb
    // assumptions, but real i18n support will provide a real
    // iswspace function.
    //
    return (c < 256 && isspace((unsigned char)c));
}


#ifdef iswpunct
#undef iswpunct
#endif

int
iswpunct(wint_t c)
{
    //
    // Assume characters over 256 are letters.  Assume characters
    // under 256 are either ASCII or Latin-1.  These are dumb
    // assumptions, but real i18n support will provide a real
    // iswpunct function.
    //
    return (c < 256 && ispunct((unsigned char)c));
}


#ifdef iswupper
#undef iswupper
#endif

int
iswupper(wint_t c)
{
    return (c < 256 && isupper((unsigned char)c));
}


#ifdef iswlower
#undef iswlower
#endif

int
iswlower(wint_t c)
{
    return (c < 256 && islower((unsigned char)c));
}


#ifdef iswdigit
#undef iswdigit
#endif

int
iswdigit(wint_t c)
{
    return (c < 256 && isdigit((unsigned char)c));
}


#ifdef iswalnum
#undef iswalnum
#endif

int
iswalnum(wint_t c)
{
    return (c < 256 && isalnum((unsigned char)c));
}


#ifdef towupper
#undef towupper
#endif

wint_t
towupper(wint_t c)
{
    if (c < 256 && islower((unsigned char)c))
       	return toupper((unsigned char)c);
    return c;
}


#ifdef towlower
#undef towlower
#endif

wint_t
towlower(wint_t c)
{
    if (c < 256 && isupper((unsigned char)c))
       	return tolower((unsigned char)c);
    return c;
}

#endif // !HAVE_ISWPRINT

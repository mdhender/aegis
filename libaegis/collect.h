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
// MANIFEST: interface of the collect class
//

#ifndef LIBAEGIS_COLLECT_H
#define LIBAEGIS_COLLECT_H

#pragma interface "collect"

#include <ac/stddef.h>

struct wstring_ty; // forward

/**
  * The collect class is used to represent an accumulated wide character
  * string.
  */
class collect
{
public:
    /**
      * The destructor.
      */
    virtual ~collect();

    /**
      * The default constructor.
      */
    collect();

    /**
      * The copy constructor.
      */
    collect(const collect &);

    /**
      * The assignment operator.
      */
    collect &operator=(const collect &);

    /**
      * The append method is used to accumulate a string one character
      * at a time.  No size limit.
      *
      * \param c
      *	    the character being collected
      */
    void append(wchar_t c);

    /**
      * The append method is used to accumulate a string several
      * characters at a time.  No size limit.
      *
      * \param s
      *	    the character string to be appended
      * \param n
      *	    the number of characters to appended
      */
    void append(const wchar_t *s, size_t n);

    /**
      * The end method is used to fetch the string accumulated with
      * the collect function.  The buffer is cleared.
      *
      * \returns
      *     wstring_ty *; pointer to the string in dynamic memory.
      */
    wstring_ty *end();

private:
    size_t pos;
    size_t size;
    wchar_t *buf;
};

#endif // LIBAEGIS_COLLECT_H

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

#ifndef LIBAEGIS_COLLECT_H
#define LIBAEGIS_COLLECT_H

#include <common/ac/stddef.h>

class wstring; // forward

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
      * The push_back method may be used to append the given wide string
      * to the end of the buffer.
      *
      * @param s
      *     The string to append.
      */
    void push_back(const wstring &s);

    /**
      * The end method is used to fetch the string accumulated with
      * the collect function.  The buffer is cleared.
      *
      * \returns
      *     wstring; pointer to the string in dynamic memory.
      */
    wstring end();

private:
    size_t pos;
    size_t size;
    wchar_t *buf;
};

#endif // LIBAEGIS_COLLECT_H

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

#ifndef COMMON_WSTRING_ACCUMULATOR_H
#define COMMON_WSTRING_ACCUMULATOR_H

#include <common/wstring.h>

/**
  * The wstring_accumulator class is used to accumulate wide characters
  * into a buffer, for turning into a wide string.
  */
class wstring_accumulator
{
public:
    /**
      * The destructor.  Thou shalt not derive from this class because
      * it isn't virtual.
      */
    ~wstring_accumulator();

    /**
      * The default constructor.
      */
    wstring_accumulator();

    /**
      * The copy constructor.
      */
    wstring_accumulator(const wstring_accumulator &);

    /**
      * The assignment operator.
      */
    wstring_accumulator &operator=(const wstring_accumulator &);

    /**
      * The mkstr method is used to take the current contents of the
      * accumulated buffer and turn it into a wide string.
      */
    wstring mkstr() const;

    /**
      * The push_back method is used to append another character to the
      * end of the accumulator.
      *
      * \param c
      *     The character to be appended to the buffer.
      */
    void
    push_back(wchar_t c)
    {
	if (length < maximum)
	    buffer[length++] = c;
	else
	    overflow(c);
    }

    /**
      * The push_back method is used to append an array of characters to
      * the end of the accumulator.
      *
      * \param data
      *     The data to be appended to the buffer.
      * \param len
      *     The number of bytes of data.
      */
    void push_back(const wchar_t *data, size_t len);

    /**
      * The push_back method is used to append a NUL terminated string
      * to the end of the accumulator.
      *
      * \param data
      *     The string to be appended to the buffer.
      */
    void push_back(const wchar_t *data);

    /**
      * The push_back method is used to append another string
      * accumulator to the end of this accumulator.
      *
      * \param data
      *     The string to be appended to the buffer.
      */
    void push_back(const wstring_accumulator &data);

    /**
      * The push_back method is used to append another string
      * accumulator to the end of this accumulator.
      *
      * \param data
      *     The string to be appended to the buffer.
      */
    void push_back(const wstring &data);

    /**
      * The clear method is used to reset the length of the accumulated
      * string to zero.
      */
    void clear() { length = 0; }

    /**
      * The size method is used to obtain the current size of the buffer
      * in characters.
      */
    size_t size() const { return length; }

    /**
      * The empty method is used to determine if the buffer is empty
      * (has a size of zero).
      */
    bool empty() const { return (length == 0); }

    /**
      * The pop_back method is used to remove the last character from
      * the buffer.
      */
    void pop_back() { if (length) --length; }

    /**
      * The back method is used to obtain the last character in the
      * buffer, or NUL if the buffer is empty.
      */
    wchar_t back() { return (length ? buffer[length - 1] : 0); }

    /**
      * The get_data method is used to obtain a pointer to the base of
      * the array of characters being accumulated.
      *
      * \note
      *     The pointer is only garanteed to be valid until the next
      *     push_back method call.
      * \note
      *     Please use this methdo as little as possible.
      */
    const wchar_t *get_data() const { return (buffer ? buffer : L""); }

    /**
      * The array index operator is used to obtain thr nth character in
      * the buffer.
      *
      * \note
      *     No array bounds checking is performed.  If you really stuff
      *     up, it will segfault.  Caveat emptor.
      */
    wchar_t operator[](size_t n) { return buffer[n]; }

private:
    /**
      * The overflow method is used to append another character to the
      * end of the accumulator.  It is called by the putch method when
      * it get too too hard (and a new buffer is needed).
      *
      * \param c
      *     The character to be appended to the buffer.
      */
    void overflow(wchar_t c);

    /**
      * The length instance variable is used to remember how many
      * characters in the buffer are significant.
      */
    size_t length;

    /**
      * The length instance variable is used to remember the allocated
      * size of the buffer.
      */
    size_t maximum;

    /**
      * The length instance variable is used to remember the base of an
      * array of characters in the heap.
      */
    wchar_t *buffer;
};

#endif // COMMON_WSTRING_ACCUMULATOR_H

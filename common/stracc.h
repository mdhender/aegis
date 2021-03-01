//
//	aegis - project change supervisor
//	Copyright (C) 1998, 2001, 2004-2006, 2008 Peter Miller
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

#ifndef COMMON_STRACC_H
#define COMMON_STRACC_H

#include <common/str.h>

/** \addtogroup String_Accumulator
  * \brief String Accumulation functions
  * \ingroup String
  * @{
  */

/**
  * The stracc_t class is used to accumulate strings into a buffer.
  */
class stracc_t
{
public:
    /**
      * The destructor.  Thou shalt not derive from this class because
      * it isn't virtual.
      */
    ~stracc_t();

    /**
      * The default constructor.
      */
    stracc_t();

    /**
      * The copy constructor.
      */
    stracc_t(const stracc_t &);

    /**
      * The assignment operator.
      */
    stracc_t &operator=(const stracc_t &);

    /**
      * The mkstr method is used to take the current contents of the
      * accumulated buffer and turn it into a string.
      */
    string_ty *mkstr() const;

    /**
      * The push_back method is used to append another character to the
      * end of the accumulator.
      *
      * \param c
      *     The character to be appended to the buffer.
      */
    void
    push_back(char c)
    {
	//
        // The stracc::push_back(char) method shows up in the profiles
        // as occupying 10% of the time it takes to parse the database
        // files.  By making it an inline, things go measurably faster.
	//
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
    void push_back(const char *data, size_t len);

    /**
      * The push_back method is used to append a NUL terminated string
      * to the end of the accumulator.
      *
      * \param data
      *     The string to be appended to the buffer.
      */
    void push_back(const char *data);

    /**
      * The push_back method is used to append another string
      * accumulator to the end of this accumulator.
      *
      * \param data
      *     The string to be appended to the buffer.
      */
    void push_back(const stracc_t &data);

    /**
      * The back method is used to obtain the last character in the
      * buffer.  The result is undefined if the buffer is empty.
      */
    char back() { return buffer[length - 1]; }

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
      * The get_data method is used to obtain a pointer to the base of
      * the array of characters being accumulated.
      *
      * \note
      *     The pointer is only garanteed to be valid until the next
      *     push_back method call.
      * \note
      *     Please use this methdo as little as possible.
      */
    const char *get_data() const { return buffer; }

    /**
      * The array index operator is used to obtain thr nth character in
      * the buffer.
      *
      * \note
      *     No array bounds checking is performed.  Caveat emptor.
      */
    char operator[](size_t n) { return buffer[n]; }

    size_t count_nul_characters() const;

private:
    /**
      * The overflow method is used to append another character to the
      * end of the accumulator.  It is called by the putch method when
      * it get too too hard (and a new buffer is needed).
      *
      * \param c
      *     The character to be appended to the buffer.
      */
    void overflow(char c);

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
    char *buffer;
};

inline DEPRECATED void
stracc_constructor(stracc_t *p)
{
    p->clear();
}

inline DEPRECATED void
stracc_destructor(stracc_t *p)
{
    p->clear();
}

inline DEPRECATED void
stracc_open(stracc_t *p)
{
    p->clear();
}

inline DEPRECATED string_ty *
stracc_close(const stracc_t *p)
{
    return p->mkstr();
}

inline DEPRECATED void
stracc_char(stracc_t *p, char c)
{
    p->push_back(c);
}

inline DEPRECATED void
stracc_chars(stracc_t *p, const char *data, size_t len)
{
    p->push_back(data, len);
}


/** @} */
#endif // COMMON_STRACC_H

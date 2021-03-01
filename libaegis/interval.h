//
//	aegis - project change supervisor
//	Copyright (C) 1991-1993, 1995, 2002, 2004-2006, 2008 Peter Miller
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

#ifndef LIBAEGIS_INTERVAL_H
#define LIBAEGIS_INTERVAL_H

#include <common/ac/stddef.h>

#include <common/main.h>

/**
  * The interval class is used to represent disjoint intervals of an
  * integer type.  The expected interval arithmtic operators (union,
  * intersect and difference) are supported.
  */
class interval
{
public:
    typedef long data_t;

    /**
      * The destructor.
      *
      * @note
      *    It is not virtual.  Thou shalt not derive from this class.
      */
    ~interval();

    /**
      * The default constructor.
      * This constructs the empty interval.
      */
    interval();

    /**
      * The copy constructor.
      */
    interval(const interval &rhs);

    /**
      * The constructor.  It is used to construct a continuous interval
      * with no holes.  The arguments are inclusive.
      *
      * @param first
      *     The first element in the interval.
      * @param last
      *     The last element in the interval.
      */
    interval(data_t first, data_t last);

    /**
      * The assignment operator.
      */
    interval &operator=(const interval &rhs);

    /**
      * The valid method is used when debugging, to ensure that the
      * interval's internal state is self-consistent.
      */
    bool valid() const;

    /**
      * The clear method is used to discard all contents of the
      * interval.  It will be empty on return.
      */
    void clear();

    /**
      * The addition operator is used to calculate the union of two
      * intervals.
      *
      * @param rhs
      *     The right hand side of the addition expression.
      * @returns
      *     a new interval containing the union of the two intervals
      */
    interval operator+(const interval &rhs) const;
    void operator+=(const interval &rhs);

    /**
      * The multiplication operator is used to calculate the
      * intersection of two intervals.
      *
      * @param rhs
      *     The right hand side of the multiplication expression.
      * @returns
      *     a new interval containing the intersection of the two intervals
      */
    interval operator*(const interval &rhs) const;
    void operator*=(const interval &rhs);

    /**
      * The subtraction operator is used to calculate the difference of
      * two intervals.
      *
      * @param rhs
      *     The right hand side of the subtraction expression.
      * @returns
      *     a new interval containing the difference of the two intervals
      */
    interval operator-(const interval &rhs) const;
    void operator-=(const interval &rhs);

    /**
      * The member method is used to determine whether or not the given
      * datum is a member of the interval.
      *
      * @param datum
      *     The datum to look for in the interval.
      * @returns
      *     bool; true if is a member, false if is not
      */
    bool member(data_t datum) const;

    void scan_begin();

    bool scan_next(data_t &datum);

    void scan_end();

    /**
      * The empty method is used to determine whether or not the
      * interval is empty.
      *
      * @return
      *     bool; true if the interval is emoty, false if not.
      */
    bool empty() const { return (length == 0); }

    /**
      * The first member is used to obtain the value of the first value
      * in the interval.
      *
      * @note
      *     the result is undefined if the interval is empty (assertion
      *     failure when debug enable)
      */
    data_t first() const;

    /**
      * The last member is used to obtain the value of the last value in
      * the interval.
      *
      * @note
      *     the result is undefined if the interval is empty (assertion
      *     failure when debug enable)
      */
    data_t last() const;

    /**
      * The second_last member is used to obtain the value of the first
      * member of the contiguous portion of this interval.
      *
      * @note
      *     the result is undefined if the interval is empty (assertion
      *     failure when debug enable)
      */
    data_t second_last() const;

private:
    size_t length;
    size_t size;
    size_t scan_index;
    data_t scan_next_datum;
    data_t *data;

    /**
      * The append method is used to add another datum to the data.  It
      * shall always be called in pairs.  The first of every pair will
      * result in an internal state which fails the valid() test.
      */
    void append(data_t datum);

    /**
      * The normalize method is used to restore the interval to a
      * valid() state after calculating a union, intersection or
      * difference.
      */
    void normalize();
};

#endif // LIBAEGIS_INTERVAL_H

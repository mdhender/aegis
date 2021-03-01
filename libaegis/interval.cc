//
//      aegis - project change supervisor
//      Copyright (C) 1991, 1992, 1995, 1997, 2002-2006, 2008, 2012 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>

#include <common/mem.h>
#include <common/trace.h>
#include <libaegis/interval.h>


//
// NAME
//      interval_valid - internal consistency check
//
// SYNOPSIS
//      int interval_valid(interval *ip);
//
// DESCRIPTION
//      The interval_valid function is used to check the internal
//      consistency of an interval.
//
// ARGUMENTS
//      ip      - pointer to interval to check
//
// RETURNS
//      int     1 if interval is valid
//              0 if interval is not valid
//
// CAVEAT
//      This function is only available if DEBUG is defined,
//      and is intended for use in assert() statements.
//

#ifdef  DEBUG

bool
interval::valid()
    const
{
    if (length > size)
        return false;
    if (length & 1)
        return false;
    if (size == 0 && data == 0)
        return (length == 0);
    if ((size_t)data[length] != length)
        return false;
    for (size_t j = 1; j < length; ++j)
        if (data[j - 1] >= data[j])
            return false;
    return true;
}

#endif


//
// NAME
//      interval_create_empty - create an empty interval
//
// SYNOPSIS
//      interval *interval_create_empty(void);
//
// DESCRIPTION
//      The interval_create_empty function is used to create
//      an empty interval.
//
// RETURNS
//      a pointer to the new interval in dynamic memory
//
// CAVEAT
//      It is the responsibility of the caller to release the
//      interval to dynamic memory when no longer required.
//      Use the interval_free function for this purpose.
//

interval::interval() :
    length(0),
    size(0),
    scan_index(0),
    scan_next_datum(0),
    data(0)
{
    trace(("interval()\n"));
    assert(valid());
}


//
// NAME
//      interval_free - release interval memory
//
// SYNOPSIS
//      void interval_free(interval *ip);
//
// DESCRIPTION
//      The interval_free function is used to release the dynamic
//      memory used by an interval back to the dynamic memory pool.
//
// ARGUMENTS
//      ip      - the interval to release
//

interval::~interval()
{
    trace(("~interval()\n"));
    clear();
}


void
interval::clear()
{
    assert(valid());
    delete [] data;
    length = 0;
    size = 0;
    scan_index = 0;
    scan_next_datum = 0;
    data = 0;
    assert(valid());
}


//
// NAME
//      interval_create_range - create a single range interval
//
// SYNOPSIS
//      interval *interval_create_range(interval_data_ty first,
//              interval_data_ty last);
//
// DESCRIPTION
//      The interval_create_range function is used to create an interval
//      consisting of a single range, from first to last inclusive.
//
// ARGUMENTS
//      first   - the start of the range
//      last    - the end of the range (inclusive)
//
// RETURNS
//      a pointer to the new interval in dynamic memory
//
// CAVEAT
//      It is the responsibility of the caller to release the
//      interval to dynamic memory when no longer required.
//      Use the interval_free function for this purpose.
//

interval::interval(data_t a_first, data_t a_last) :
    length(2),
    size(2),
    scan_index(0),
    scan_next_datum(0),
    data(new data_t[3])
{
    trace(("interval(%ld, %ld)\n", a_first, a_last));
    data[0] = a_first;
    data[1] = a_last + 1;
    data[2] = 2;
    assert(valid());
}

interval::interval(const interval &rhs) :
    length(0),
    size(0),
    scan_index(0),
    scan_next_datum(0),
    data(0)
{
    trace(("interval()\n"));
    if (!rhs.empty())
    {
        for (;;)
        {
            size = size * 2 + 8;
            if (rhs.length <= size)
                break;
        }
        data = new data_t [size + 1];
        length = rhs.length;
        for (size_t j = 0; j < length; ++j)
            data[j] = rhs.data[j];
        data[length] = length;
    }
    assert(valid());
}


interval &
interval::operator=(const interval &rhs)
{
    if (this != &rhs)
    {
        if (rhs.empty())
            clear();
        else
        {
            if (rhs.length > size)
            {
                for (;;)
                {
                    size = size * 2 + 8;
                    if (rhs.length <= size)
                        break;
                }
                delete [] data;
                data = new data_t [size + 1];
            }
            length = rhs.length;
            for (size_t j = 0; j < length; ++j)
                data[j] = rhs.data[j];
            data[length] = length;
        }
    }
    return *this;
}


//
// NAME
//      append - append datum to interval data
//
// SYNOPSIS
//      void append(interval **ipp, interval_data_ty datum);
//
// DESCRIPTION
//      The append function is used to append a datum to
//      the end of an interval under construction.
//
// ARGUMENTS
//      ipp     - pointer to inerval pointer.
//      datum   - value to append.
//
// CAVEAT
//      The interval may move in dynamic memory, with is why ** is used.
//      The interval will need to be normalized before you
//      next use interval_valid.
//

void
interval::append(data_t datum)
{
    //
    // should always be increasing
    //
    trace(("interval::append()\n{\n"));
    assert(length < 1 || datum >= data[length - 1]);

    //
    // make it larger if necessary
    //
    if (length >= size)
    {
        size = size * 2 + 8;
        data_t *new_data = new data_t [size + 1];
        for (size_t j = 0; j < length; ++j)
            new_data[j] = data[j];
        delete [] data;
        data = new_data;
    }

    //
    // remeber the datum
    //
    data[length++] = datum;

    //
    // elide empty sequences
    //
    if (length >= 2 && data[length - 1] == data[length - 2])
        length -= 2;

    // NOTE: this may produce an invalid interval.  This method shall
    // always be balled an even number of times.
    trace(("}\n"));
}


//
// NAME
//      normalize - clean up after append
//
// SYNOPSIS
//      void normalize(interval **);
//
// DESCRIPTION
//      The normalize function is used to clean up after
//      the append function.
//
// ARGUMENTS
//      ipp     - pointer to interval to normalize
//
// CAVEAT
//      The interval may move in dynamic memory, with is why ** is used.
//

void
interval::normalize()
{
    trace(("interval::normalize()\n{\n"));
    if (length == 0)
    {
        assert(!size == !data);
        if (data)
        {
            delete [] data;
            size = 0;
        }
    }
    else
    {
        data[length] = length;
    }
    assert(valid());
    trace(("}\n"));
}


//
// NAME
//      interval_union - union of two intervals
//
// SYNOPSIS
//      interval *interval_union(interval *left, interval *right);
//
// DESCRIPTION
//      The interval_union function is used to form the
//      union of two intervals.
//
// ARGUMENTS
//      left    - interval to be unioned with
//      right   - another interval
//
// RETURNS
//      a pointer to the new interval in dynamic memory
//
// CAVEAT
//      It is the responsibility of the caller to release the
//      interval to dynamic memory when no longer required.
//      Use the interval_free function for this purpose.
//

interval
interval::operator+(const interval &rhs)
    const
{
    trace(("interval::operator+()\n{\n"));
    data_t place;
    assert(valid());
    assert(rhs.valid());

    interval result;
    size_t left_pos = 0;
    size_t right_pos = 0;
    int count = 0;
    for (;;)
    {
        int old_count = count;
        if (left_pos < length)
        {
            if (right_pos < rhs.length)
            {
                if (data[left_pos] < rhs.data[right_pos])
                {
                    count += ((left_pos & 1) ? -1 : 1);
                    place = data[left_pos++];
                }
                else
                {
                    count += (right_pos & 1 ? -1 : 1);
                    place = rhs.data[right_pos++];
                }
            }
            else
            {
                count += ((left_pos & 1) ? -1 : 1);
                place = data[left_pos++];
            }
        }
        else
        {
            if (right_pos < rhs.length)
            {
                count += ((right_pos & 1) ? -1 : 1);
                place = rhs.data[right_pos++];
            }
            else
                break;
        }
        if ((count >= 1) != (old_count >= 1))
            result.append(place);
    }
    result.normalize();
    trace(("}\n"));
    return result;
}


void
interval::operator+=(const interval &rhs)
{
    interval i = *this + rhs;
    *this = i;
}


//
// NAME
//      interval_intersection - intersection of two intervals
//
// SYNOPSIS
//      interval *interval_intersection(interval *left,
//              interval *right);
//
// DESCRIPTION
//      The interval_intersection function is used to form the
//      intersection of two intervals.
//
// ARGUMENTS
//      left    - interval to be intersected with
//      right   - another interval
//
// RETURNS
//      a pointer to the new interval in dynamic memory
//
// CAVEAT
//      It is the responsibility of the caller to release the
//      interval to dynamic memory when no longer required.
//      Use the interval_free function for this purpose.
//

interval
interval::operator*(const interval &rhs)
    const
{
    trace(("interval::operator*()\n{\n"));
    assert(valid());
    assert(rhs.valid());

    interval result;
    size_t left_pos = 0;
    size_t right_pos = 0;
    int count = 0;
    for (;;)
    {
        data_t place;
        int old_count = count;
        if (left_pos < length)
        {
            if (right_pos < rhs.length)
            {
                if (data[left_pos] < rhs.data[right_pos])
                {
                    count += ((left_pos & 1) ? -1 : 1);
                    place = data[left_pos++];
                }
                else
                {
                    count += ((right_pos & 1) ? -1 : 1);
                    place = rhs.data[right_pos++];
                }
            }
            else
            {
                count += ((left_pos & 1) ? -1 : 1);
                place = data[left_pos++];
            }
        }
        else
        {
            if (right_pos < rhs.length)
            {
                count += ((right_pos & 1) ? -1 : 1);
                place = rhs.data[right_pos++];
            }
            else
                break;
        }
        if ((count >= 2) != (old_count >= 2))
            result.append(place);
    }
    result.normalize();

    assert(result.valid());
    trace(("}\n"));
    return result;
}


void
interval::operator*=(const interval &rhs)
{
    interval i = *this * rhs;
    *this = i;
}


//
// NAME
//      interval_difference - difference of two intervals
//
// SYNOPSIS
//      interval *interval_difference(interval *left, interval *right);
//
// DESCRIPTION
//      The interval_difference function is used to form the
//      difference of two intervals.
//
// ARGUMENTS
//      left    - interval to take things out of
//      right   - things to take out of it
//
// RETURNS
//      a pointer to the new interval in dynamic memory
//
// CAVEAT
//      It is the responsibility of the caller to release the
//      interval to dynamic memory when no longer required.
//      Use the interval_free function for this purpose.
//

interval
interval::operator-(const interval &rhs)
    const
{
    trace(("interval::operator-()\n{\n"));
    assert(valid());
    assert(rhs.valid());

    interval result;
    size_t left_pos = 0;
    size_t right_pos = 0;
    int count = 0;
    for (;;)
    {
        data_t place;
        int old_count = count;
        if (left_pos < length)
        {
            if (right_pos < rhs.length)
            {
                if (data[left_pos] < rhs.data[right_pos])
                {
                    count += ((left_pos & 1) ? -1 : 1);
                    place = data[left_pos++];
                }
                else
                {
                    count -= ((right_pos & 1) ? -1 : 1);
                    place = rhs.data[right_pos++];
                }
            }
            else
            {
                count += ((left_pos & 1) ? -1 : 1);
                place = data[left_pos++];
            }
        }
        else
        {
            if (right_pos < rhs.length)
            {
                count -= ((right_pos & 1) ? -1 : 1);
                place = rhs.data[right_pos++];
            }
            else
                break;
        }
        if ((count >= 1) != (old_count >= 1))
            result.append(place);
    }
    result.normalize();
    trace(("}\n"));
    return result;
}


void
interval::operator-=(const interval &rhs)
{
    interval i = *this - rhs;
    *this = i;
}


//
// NAME
//      interval_member - test for membership
//
// SYNOPSIS
//      int interval_member(interval *, interval_data_ty datum);
//
// DESCRIPTION
//      The interval_member function is used to test if a particular
//      datum is included in an interval.
//
// ARGUMENTS
//      ip      - interval to test
//      datum   - value to test for
//
// RETURNS
//      int     1 if is a member
//              0 if is not a member
//

bool
interval::member(data_t datum)
    const
{
    trace(("interval::member(this = %p, datum = %ld)\n{\n",
        this, (long)datum));
    assert(valid());
    size_t min = 0;
    size_t max = length - 2;
    while (min <= max)
    {
        size_t mid = ((min + max) / 2) & ~1;
        if (data[mid] <= datum && datum < data[mid + 1])
        {
            trace(("return true;\n"));
            trace(("}\n"));
            return true;
        }
        if (data[mid] < datum)
            min = mid + 2;
        else
            max = mid - 2;
    }
    trace(("return false;\n"));
    trace(("}\n"));
    return false;
}


//
// NAME
//      interval_scan_begin
//
// SYNOPSIS
//      void interval_scan_begin(interval *ip);
//
// DESCRIPTION
//      The interval_scan_begin function is used to
//      start traversing every datum in the interval.
//
// ARGUMENTS
//      ip      - interval to scan
//

void
interval::scan_begin()
{
    assert(valid());
    assert(!scan_index);
    scan_index = 1;
    if (length)
        scan_next_datum = data[0];
    else
        scan_next_datum = 0;
}


//
// NAME
//      interval_scan_next
//
// SYNOPSIS
//      int interval_scan_next(interval *ip, interval_data_ty *datum);
//
// DESCRIPTION
//      The interval_scan_next function is used to
//      traverse every datum in the interval.
//
// ARGUMENTS
//      ip      - interval to scan
//      datum   - pointer to where to place datum
//
// RETURNS
//      int     1 if datum available
//              0 if reached end of interval
//

bool
interval::scan_next(data_t &datum)
{
    assert(valid());
    assert(scan_index & 1);
    if (scan_index >= length)
        return false;
    if (scan_next_datum >= data[scan_index])
    {
        scan_index += 2;
        if (scan_index >= length)
            return false;
        scan_next_datum = data[scan_index - 1];
    }
    datum = scan_next_datum++;
    return true;
}


//
// NAME
//      interval_scan_end
//
// SYNOPSIS
//      void interval_scan_end(interval *ip);
//
// DESCRIPTION
//      The interval_scan_end function is used to
//      finish traversing every datum in the interval.
//
// ARGUMENTS
//      ip      - interval to scan
//

void
interval::scan_end()
{
    assert(valid());
    assert(scan_index & 1);
    scan_index = 0;
    scan_next_datum = 0;
}


interval::data_t
interval::first()
    const
{
    assert(valid());
    assert(!empty());
    return data[0];
}


interval::data_t
interval::last()
    const
{
    assert(valid());
    assert(!empty());
    return (data[length - 1] - 1);
}


interval::data_t
interval::second_last()
    const
{
    assert(valid());
    assert(!empty());
    return data[length - 2];
}


// vim: set ts=8 sw=4 et :

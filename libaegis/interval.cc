//
//	aegis - project change supervisor
//	Copyright (C) 1991, 1992, 1995, 1997, 2002-2004 Peter Miller;
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
// MANIFEST: functions to manipulate intervals
//

#include <interval.h>
#include <error.h>
#include <mem.h>
#include <trace.h>


//
// NAME
//	interval_valid - internal consistency check
//
// SYNOPSIS
//	int interval_valid(interval_ty *ip);
//
// DESCRIPTION
//	The interval_valid function is used to check the internal
//	consistency of an interval.
//
// ARGUMENTS
//	ip	- pointer to interval to check
//
// RETURNS
//	int	1 if interval is valid
//		0 if interval is not valid
//
// CAVEAT
//	This function is only available if DEBUG is defined,
//	and is intended for use in assert() statements.
//

#ifdef	DEBUG

static int
interval_valid(interval_ty *ip)
{
    size_t	    j;

    if (ip->length != ip->size)
	return 0;
    if (ip->length & 1)
	return 0;
    if ((size_t)ip->data[ip->length] != ip->length)
	return 0;
    for (j = 1; j < ip->length; ++j)
	if (ip->data[j - 1] >= ip->data[j])
    	    return 0;
    return 1;
}

#endif


//
// NAME
//	interval_create_empty - create an empty interval
//
// SYNOPSIS
//	interval_ty *interval_create_empty(void);
//
// DESCRIPTION
//	The interval_create_empty function is used to create
//	an empty interval.
//
// RETURNS
//	a pointer to the new interval in dynamic memory
//
// CAVEAT
//	It is the responsibility of the caller to release the
//	interval to dynamic memory when no longer required.
//	Use the interval_free function for this purpose.
//

interval_ty *
interval_create_empty(void)
{
    interval_ty	*ip;

    trace(("interval_create_empty()\n{\n"));
    ip = (interval_ty *)mem_alloc(sizeof(interval_ty));
    ip->length = 0;
    ip->size = 0;
    ip->scan_index = 0;
    ip->scan_next_datum = 0;
    ip->data[0] = 0;
    assert(interval_valid(ip));
    trace(("return %8.8lX;\n", (long)ip));
    trace(("}\n"));
    return ip;
}


//
// NAME
//	interval_free - release interval memory
//
// SYNOPSIS
//	void interval_free(interval_ty *ip);
//
// DESCRIPTION
//	The interval_free function is used to release the dynamic
//	memory used by an interval back to the dynamic memory pool.
//
// ARGUMENTS
//	ip	- the interval to release
//

void
interval_free(interval_ty *ip)
{
    trace(("interval_free(ip = %8.8lX)\n{\n", (long)ip));
    assert(interval_valid(ip));
    mem_free(ip);
    trace(("}\n"));
}


//
// NAME
//	interval_create_range - create a single range interval
//
// SYNOPSIS
//	interval_ty *interval_create_range(interval_data_ty first,
//		interval_data_ty last);
//
// DESCRIPTION
//	The interval_create_range function is used to create an interval
//	consisting of a single range, from first to last inclusive.
//
// ARGUMENTS
//	first	- the start of the range
//	last	- the end of the range (inclusive)
//
// RETURNS
//	a pointer to the new interval in dynamic memory
//
// CAVEAT
//	It is the responsibility of the caller to release the
//	interval to dynamic memory when no longer required.
//	Use the interval_free function for this purpose.
//

interval_ty *
interval_create_range(interval_data_ty first, interval_data_ty last)
{
    interval_ty	*ip;

    trace(("interval_create_range(%ld, %ld)\n{\n", first, last));
    ip = (interval_ty *)mem_alloc(
        sizeof(interval_ty) + 2 * sizeof(interval_data_ty));
    ip->length = 2;
    ip->size = 2;
    ip->scan_index = 0;
    ip->scan_next_datum = 0;
    ip->data[0] = first;
    ip->data[1] = last + 1;				//lint !e415
    ip->data[2] = 2;					//lint !e415 !e416
    assert(interval_valid(ip));
    trace(("return %8.8lX;\n", (long)ip));
    trace(("}\n"));
    return ip;
}


//
// NAME
//	append - append datum to interval data
//
// SYNOPSIS
//	void append(interval_ty **ipp, interval_data_ty datum);
//
// DESCRIPTION
//	The append function is used to append a datum to
//	the end of an interval under construction.
//
// ARGUMENTS
//	ipp	- pointer to inerval pointer.
//	datum	- value to append.
//
// CAVEAT
//	The interval may move in dynamic memory, with is why ** is used.
//	The interval will need to be normalized before you
//	next use interval_valid.
//

static void
append(interval_ty **ipp, interval_data_ty datum)
{
    interval_ty	*ip;

    //
    // should always be increasing
    //
    trace(("append()\n{\n"));
    ip = *ipp;
    assert(ip->length < 1 || datum >= ip->data[ip->length - 1]);

    //
    // make it larger if necessary
    //
    if (ip->length >= ip->size)
    {
	size_t		nbytes;

	ip->size += 8;
	nbytes = sizeof(interval_ty) + ip->size * sizeof(interval_data_ty);
	ip = (interval_ty *)mem_change_size(ip, nbytes);
	*ipp = ip;
    }

    //
    // remeber the datum
    //
    ip->data[ip->length++] = datum;

    //
    // elide empty sequences
    //
    if (ip->length >= 2 && ip->data[ip->length - 1] == ip->data[ip->length - 2])
	ip->length -= 2;
    trace(("}\n"));
}


//
// NAME
//	normalize - clean up after append
//
// SYNOPSIS
//	void normalize(interval_ty **);
//
// DESCRIPTION
//	The normalize function is used to clean up after
//	the append function.
//
// ARGUMENTS
//	ipp	- pointer to interval to normalize
//
// CAVEAT
//	The interval may move in dynamic memory, with is why ** is used.
//

static void
normalize(interval_ty **ipp)
{
    interval_ty     *ip;

    trace(("normalize()\n{\n"));
    ip = *ipp;
    ip->data[ip->length] = ip->length;
    assert(ip->length <= ip->size);
    if (ip->length < ip->size)
    {
	size_t		nbytes;

	ip->size = ip->length;
	nbytes = sizeof(interval_ty) + ip->size * sizeof(interval_data_ty);
	ip = (interval_ty *)mem_change_size(ip, nbytes);
	*ipp = ip;
    }
    assert(interval_valid(ip));
    trace(("}\n"));
}


//
// NAME
//	interval_union - union of two intervals
//
// SYNOPSIS
//	interval_ty *interval_union(interval_ty *left, interval_ty *right);
//
// DESCRIPTION
//	The interval_union function is used to form the
//	union of two intervals.
//
// ARGUMENTS
//	left	- interval to be unioned with
//	right	- another interval
//
// RETURNS
//	a pointer to the new interval in dynamic memory
//
// CAVEAT
//	It is the responsibility of the caller to release the
//	interval to dynamic memory when no longer required.
//	Use the interval_free function for this purpose.
//

interval_ty *
interval_union(interval_ty *left, interval_ty *right)
{
    interval_ty     *result;
    size_t	    left_pos;
    size_t	    right_pos;
    int		    count;
    int		    old_count;
    interval_data_ty place;

    trace(("interval_union(left = %8.8lX, right = %8.8lX)\n{\n",
	(long)left, (long)right));
    assert(interval_valid(left));
    assert(interval_valid(right));

    result = interval_create_empty();
    left_pos = 0;
    right_pos = 0;
    count = 0;
    for (;;)
    {
	old_count = count;
	if (left_pos < left->length)
	{
	    if (right_pos < right->length)
	    {
		if (left->data[left_pos] < right->data[right_pos])
		{
	    	    count += (left_pos & 1 ? -1 : 1);
	    	    place = left->data[left_pos++];
		}
		else
		{
	    	    count += (right_pos & 1 ? -1 : 1);
	    	    place = right->data[right_pos++];
		}
	    }
	    else
	    {
		count += (left_pos & 1 ? -1 : 1);
		place = left->data[left_pos++];
	    }
	}
	else
	{
	    if (right_pos < right->length)
	    {
	       	count += (right_pos & 1 ? -1 : 1);
	       	place = right->data[right_pos++];
	    }
	    else
	       	break;
	}
	if ((count >= 1) != (old_count >= 1))
	    append(&result, place);
    }
    normalize(&result);

    assert(interval_valid(result));
    trace(("return %8.8lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


//
// NAME
//	interval_intersection - intersection of two intervals
//
// SYNOPSIS
//	interval_ty *interval_intersection(interval_ty *left,
//		interval_ty *right);
//
// DESCRIPTION
//	The interval_intersection function is used to form the
//	intersection of two intervals.
//
// ARGUMENTS
//	left	- interval to be intersected with
//	right	- another interval
//
// RETURNS
//	a pointer to the new interval in dynamic memory
//
// CAVEAT
//	It is the responsibility of the caller to release the
//	interval to dynamic memory when no longer required.
//	Use the interval_free function for this purpose.
//

interval_ty *
interval_intersection(interval_ty *left, interval_ty *right)
{
    interval_ty     *result;
    size_t	    left_pos;
    size_t	    right_pos;
    int		    count;
    int		    old_count;
    interval_data_ty place;

    trace(("interval_intersection(%8.8lX, %8.8lX)\n{\n",
	(long)left, (long)right));
    assert(interval_valid(left));
    assert(interval_valid(right));

    result = interval_create_empty();
    left_pos = 0;
    right_pos = 0;
    count = 0;
    for (;;)
    {
	old_count = count;
	if (left_pos < left->length)
	{
	    if (right_pos < right->length)
	    {
		if (left->data[left_pos] < right->data[right_pos])
		{
	    	    count += (left_pos & 1 ? -1 : 1);
	    	    place = left->data[left_pos++];
		}
		else
		{
	    	    count += (right_pos & 1 ? -1 : 1);
	    	    place = right->data[right_pos++];
		}
	    }
	    else
	    {
		count += (left_pos & 1 ? -1 : 1);
		place = left->data[left_pos++];
	    }
	}
	else
	{
	    if (right_pos < right->length)
	    {
	       	count += (right_pos & 1 ? -1 : 1);
	       	place = right->data[right_pos++];
	    }
	    else
	       	break;
	}
	if ((count >= 2) != (old_count >= 2))
	    append(&result, place);
    }
    normalize(&result);

    assert(interval_valid(result));
    trace(("return %8.8lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


//
// NAME
//	interval_difference - difference of two intervals
//
// SYNOPSIS
//	interval_ty *interval_difference(interval_ty *left, interval_ty *right);
//
// DESCRIPTION
//	The interval_difference function is used to form the
//	difference of two intervals.
//
// ARGUMENTS
//	left	- interval to take things out of
//	right	- things to take out of it
//
// RETURNS
//	a pointer to the new interval in dynamic memory
//
// CAVEAT
//	It is the responsibility of the caller to release the
//	interval to dynamic memory when no longer required.
//	Use the interval_free function for this purpose.
//

interval_ty *
interval_difference(interval_ty *left, interval_ty *right)
{
    interval_ty     *result;
    size_t	    left_pos;
    size_t	    right_pos;
    int		    count;
    int		    old_count;
    interval_data_ty place;

    trace(("interval_difference(%8.8lX, %8.8lX)\n{\n",
	(long)left, (long)right));
    assert(interval_valid(left));
    assert(interval_valid(right));

    result = interval_create_empty();
    left_pos = 0;
    right_pos = 0;
    count = 0;
    for (;;)
    {
	old_count = count;
	if (left_pos < left->length)
	{
	    if (right_pos < right->length)
	    {
		if (left->data[left_pos] < right->data[right_pos])
		{
	    	    count += (left_pos & 1 ? -1 : 1);
	    	    place = left->data[left_pos++];
		}
		else
		{
	    	    count -= (right_pos & 1 ? -1 : 1);
	    	    place = right->data[right_pos++];
		}
	    }
	    else
	    {
		count += (left_pos & 1 ? -1 : 1);
		place = left->data[left_pos++];
	    }
	}
	else
	{
	    if (right_pos < right->length)
	    {
		count -= (right_pos & 1 ? -1 : 1);
	       	place = right->data[right_pos++];
	    }
	    else
	       	break;
	}
	if ((count >= 1) != (old_count >= 1))
	    append(&result, place);
    }
    normalize(&result);

    assert(interval_valid(result));
    trace(("return %8.8lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


//
// NAME
//	interval_member - test for membership
//
// SYNOPSIS
//	int interval_member(interval_ty *, interval_data_ty datum);
//
// DESCRIPTION
//	The interval_member function is used to test if a particular
//	datum is included in an interval.
//
// ARGUMENTS
//	ip	- interval to test
//	datum	- value to test for
//
// RETURNS
//	int	1 if is a member
//		0 if is not a member
//

int
interval_member(interval_ty *ip, interval_data_ty datum)
{
    size_t	    min;
    size_t	    max;
    size_t	    mid;

    trace(("interval_member(ip = %8.8lX, datum = %ld)\n{\n",
	(long)ip, datum));
    assert(interval_valid(ip));
    min = 0;
    max = ip->length - 2;
    while (min <= max)
    {
	mid = ((min + max) / 2) & ~1;
	if (ip->data[mid] <= datum && datum < ip->data[mid + 1])
	{
	    trace(("return 1;\n"));
	    trace(("}\n"));
	    return 1;
	}
	if (ip->data[mid] < datum)
	    min = mid + 2;
	else
	    max = mid - 2;
    }
    trace(("return 0;\n"));
    trace(("}\n"));
    return 0;
}


//
// NAME
//	interval_scan_begin
//
// SYNOPSIS
//	void interval_scan_begin(interval_ty *ip);
//
// DESCRIPTION
//	The interval_scan_begin function is used to
//	start traversing every datum in the interval.
//
// ARGUMENTS
//	ip	- interval to scan
//

void
interval_scan_begin(interval_ty *ip)
{
    assert(interval_valid(ip));
    assert(!ip->scan_index);
    ip->scan_index = 1;
    if (ip->length)
       	ip->scan_next_datum = ip->data[0];
    else
       	ip->scan_next_datum = 0;
}


//
// NAME
//	interval_scan_next
//
// SYNOPSIS
//	int interval_scan_next(interval_ty *ip, interval_data_ty *datum);
//
// DESCRIPTION
//	The interval_scan_next function is used to
//	traverse every datum in the interval.
//
// ARGUMENTS
//	ip	- interval to scan
//	datum	- pointer to where to place datum
//
// RETURNS
//	int	1 if datum available
//		0 if reached end of interval
//

int
interval_scan_next(interval_ty *ip, interval_data_ty *datum)
{
    assert(interval_valid(ip));
    assert(ip->scan_index & 1);
    if (ip->scan_index >= ip->length)
	return 0;
    if (ip->scan_next_datum >= ip->data[ip->scan_index])
    {
	ip->scan_index += 2;
	if (ip->scan_index >= ip->length)
    	    return 0;
	ip->scan_next_datum = ip->data[ip->scan_index - 1];
    }
    *datum = ip->scan_next_datum++;
    return 1;
}


//
// NAME
//	interval_scan_end
//
// SYNOPSIS
//	void interval_scan_end(interval_ty *ip);
//
// DESCRIPTION
//	The interval_scan_end function is used to
//	finish traversing every datum in the interval.
//
// ARGUMENTS
//	ip	- interval to scan
//

void
interval_scan_end(interval_ty *ip)
{
    assert(interval_valid(ip));
    assert(ip->scan_index & 1);
    ip->scan_index = 0;
    ip->scan_next_datum = 0;
}

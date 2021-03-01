//
//	aegis - project change supervisor
//	Copyright (C) 2002-2008 Peter Miller
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

#include <common/ac/string.h>

#include <libaegis/change.h>
#include <common/error.h> // for assert
#include <aeannotate/line_list.h>
#include <common/trace.h>


#ifdef DEBUG
#define CHECK(llp) \
	assert(llp->start1 <= llp->start2); \
	assert(llp->start1 + llp->length1 <= llp->start2); \
	assert((llp->length1 != 0) || (llp->start1 == 0)); \
	assert(llp->start2 <= llp->maximum); \
	assert(llp->start2 + llp->length2 <= llp->maximum); \
	assert((llp->length2 != 0) || (llp->start2 == llp->maximum)); \
	{ \
	size_t mj, mk; \
	for (mj = 0; mj < llp->length1; ++mj) \
	assert(llp->item[llp->start1 + mj].cp); \
	for (mk = 0; mk < llp->length2; ++mk) \
	assert(llp->item[llp->start2 + mk].cp); \
	}
#else
#define CHECK(llp)
#endif


void
line_list_constructor(line_list_t *llp)
{
    trace(("line_list_constructor(llp = %08lX)\n{\n", (long)llp));
    llp->maximum = 0;
    llp->start1 = 0;
    llp->length1 = 0;
    llp->start2 = 0;
    llp->length2 = 0;
    llp->item = 0;
    CHECK(llp);
    trace(("}\n"));
}


void
line_list_destructor(line_list_t *llp)
{
    trace(("line_list_destructor(llp = %08lX)\n{\n", (long)llp));
    line_list_clear(llp);
    delete [] llp->item;
    llp->maximum = 0;
    llp->start1 = 0;
    llp->length1 = 0;
    llp->start2 = 0;
    llp->length2 = 0;
    llp->item = 0;
    trace(("}\n"));
}


void
line_list_clear(line_list_t *llp)
{
    size_t	    j;

    trace(("line_list_clear(llp = %08lX)\n{\n", (long)llp));
    CHECK(llp);
    for (j = 0; j < llp->length1; ++j)
	line_destructor(llp->item + llp->start1 + j);
    llp->start1 = 0;
    llp->length1 = 0;
    for (j = 0; j < llp->length2; ++j)
	line_destructor(llp->item + llp->start2 + j);
    llp->start2 = llp->maximum;
    llp->length2 = 0;
    CHECK(llp);
    trace(("}\n"));
}


void
line_list_delete(line_list_t *llp, size_t first_line, size_t num_lines)
{
    trace(("line_list_delete(llp = %08lX, first_line = %ld, "
	"num_lines = %ld)\n{\n", (long)llp, (long)first_line, (long)num_lines));
    CHECK(llp);
    if (num_lines == 0)
	first_line = 0;
    assert(first_line < llp->length1 + llp->length2);
    assert(first_line + num_lines <= llp->length1 + llp->length2);

    while (num_lines)
    {
	size_t		second_line;

	trace(("llp->start1 = %ld\n", (long)llp->start1));
	trace(("llp->length1 = %ld\n", (long)llp->length1));
	if (first_line < llp->length1)
	{
	    size_t	    partial_num_lines;
	    size_t	    remainder_num_lines;
	    size_t	    j;

	    //
	    // Destroy those lines which fall into the first range.
	    //
	    trace(("destroy mid range1\n"));
	    partial_num_lines = num_lines;
	    if (first_line + num_lines > llp->length1)
		partial_num_lines = llp->length1 - first_line;
	    trace(("partial_num_lines = %ld\n", (long)partial_num_lines));
	    for (j = 0; j < partial_num_lines; ++j)
		line_destructor(llp->item + llp->start1 + first_line + j);

	    //
	    // Move the end of the first range to the beginning of the
	    // second range.
	    //
	    remainder_num_lines = llp->length1 - first_line - partial_num_lines;
	    trace(("remainder_num_lines = %ld\n", (long)remainder_num_lines));
	    memmove
	    (
	    	llp->item + llp->start2 - remainder_num_lines,
	    	llp->item + llp->start1 + llp->length1 - remainder_num_lines,
		remainder_num_lines * sizeof(llp->item[0])
	    );
	    llp->start2 -= remainder_num_lines;
	    llp->length2 += remainder_num_lines;

	    //
	    // Adjust the length of the first range.
	    //
	    if (first_line)
		llp->length1 = first_line;
	    else
	    {
		llp->start1 = 0;
		llp->length1 = 0;
	    }

	    //
	    // Adjust the number of lines are are deleting.
	    //
	    // Note that "first_line" does not move, because we have
	    // deleted lines from the start of the range, but not moved
	    // the front of the delete range.
	    //
	    num_lines -= partial_num_lines;
	    trace(("num_lines = %ld\n", (long)num_lines));
	    continue;
	}

	trace(("llp->start2 = %ld\n", (long)llp->start2));
	trace(("llp->length2 = %ld\n", (long)llp->length2));
	second_line = first_line - llp->length1;
	trace(("second_line = %ld\n", (long)second_line));
	if (second_line < llp->length2)
	{
	    size_t	    partial_num_lines;
	    size_t	    j;

	    //
	    // Move the beginning of the second range to the end of the
	    // first range.
	    //
	    memmove
	    (
	    	llp->item + llp->start1 + llp->length1,
	    	llp->item + llp->start2,
		second_line * sizeof(llp->item[0])
	    );
	    llp->length1 += second_line;
	    llp->start2 += second_line;
	    llp->length2 -= second_line;
	    second_line = 0;

	    //
	    // Destroy those lines which fall into the second range.
	    //
	    trace(("destroy mid range2\n"));
	    partial_num_lines = num_lines;
	    if (partial_num_lines > llp->length2)
		partial_num_lines = llp->length2;
	    for (j = 0; j < partial_num_lines; ++j)
		line_destructor(llp->item + llp->start2 + j);

	    //
	    // Adjust the length of the second range.
	    //
	    llp->start2 += partial_num_lines;
	    llp->length2 -= partial_num_lines;
	    if (llp->length2 == 0)
	    	llp->start2 = llp->maximum;

	    //
	    // Adjust the number of lines are are deleting.
	    //
	    num_lines -= partial_num_lines;
	    trace(("num_lines = %ld\n", (long)num_lines));
	    first_line += partial_num_lines;
	    trace(("first_line = %ld\n", (long)first_line));
	    continue;
	}

	//
	// Oops.  This isn't means to happen.  It should also have been
	// caught by the assert at the beginning of this function.
	//
	assert(0);
	break;
    }

    trace(("Checking...\n"));
    trace(("llp->start1 = %ld\n", (long)llp->start1));
    trace(("llp->length1 = %ld\n", (long)llp->length1));
    trace(("llp->start2 = %ld\n", (long)llp->start2));
    trace(("llp->length2 = %ld\n", (long)llp->length2));
    trace(("llp->maximum = %ld\n", (long)llp->maximum));

    CHECK(llp);
    trace(("}\n"));
}


void
line_list_insert(line_list_t *llp,
                 size_t first_line,
                 change::pointer cp,
                 string_ty *text)
{
    trace(("line_list_insert(llp = %08lX, first_line = %ld, cp = %08lX, "
	"text = %08lX)\n{\n", (long)llp, (long)first_line, (long)cp,
	(long)text));
    CHECK(llp);
    assert(first_line <= llp->length1 + llp->length2);
    assert(cp);
    assert(text);

    for (;;)
    {
	size_t		second_line;

	trace(("llp->start1 = %ld\n", (long)llp->start1));
	trace(("llp->length1 = %ld\n", (long)llp->length1));
	trace(("llp->start2 = %ld\n", (long)llp->start2));
	trace(("llp->length2 = %ld\n", (long)llp->length2));
	if (first_line <= llp->length1)
	{
	    size_t	    remainder_num_lines;

	    //
	    // Move the first range down to the beginning of the buffer.
	    //
	    if (llp->start1)
	    {
		memmove
		(
		    llp->item,
		    llp->item + llp->start1,
		    llp->length1 * sizeof(llp->item[0])
		);
		llp->start1 = 0;
	    }

	    //
	    // We need to grow if we have run out of room in the first
	    // range (in which case the second range will be empty).
	    //
	    assert(llp->start1 == 0);
	    if (llp->length1 == llp->start2)
	    {
		trace(("growing...\n"));
		assert(llp->length2 == 0);
		assert(llp->start2 == llp->maximum);
		llp->maximum = llp->maximum * 2 + 4;
		line_t *new_item = new line_t [llp->maximum];
		for (size_t k = 0; k < llp->length1; ++k)
		    new_item[k] = llp->item[k];
		delete [] llp->item;
		llp->item = new_item;

		llp->start2 = llp->maximum;
		trace(("llp->start2 = %ld\n", (long)llp->start2));
	    }

	    //
	    // Move the end of the first range to the beginning of the
	    // second range.
	    //
	    remainder_num_lines = llp->length1 - first_line;
	    trace(("remainder_num_lines = %ld\n", (long)remainder_num_lines));
	    if (remainder_num_lines)
	    {
		memmove
		(
		    llp->item + llp->start2 - remainder_num_lines,
		    llp->item + llp->start1 + first_line,
		    remainder_num_lines * sizeof(llp->item[0])
		);
		llp->length1 = first_line;
		llp->start2 -= remainder_num_lines;
		llp->length2 += remainder_num_lines;
		trace(("llp->start1 = %ld\n", (long)llp->start1));
		trace(("llp->length1 = %ld\n", (long)llp->length1));
		trace(("llp->start2 = %ld\n", (long)llp->start2));
		trace(("llp->length2 = %ld\n", (long)llp->length2));
	    }

	    //
	    // Add the line to the end of the first range.
	    //
	    assert(llp->start1 == 0);
	    line_constructor
	    (
		llp->item // + llp->start1
 + first_line,
		cp,
		text
	    );

	    //
	    // Extend the first range to cover it.
	    //
	    // If it grows into the second range, rearrange things so
	    // that the first range contains both, and the second range
	    // is empty.
	    //
	    llp->length1++;
	    assert(llp->start1 == 0);
	    if (// llp->start1 +
 llp->length1 == llp->start2)
	    {
		llp->length1 += llp->length2;
		llp->start2 = llp->maximum;
		llp->length2 = 0;
	    }

	    //
	    // All done.
	    //
	    trace(("llp->start1 = %ld\n", (long)llp->start1));
	    trace(("llp->length1 = %ld\n", (long)llp->length1));
	    trace(("llp->start2 = %ld\n", (long)llp->start2));
	    trace(("llp->length2 = %ld\n", (long)llp->length2));
	    break;
	}

	trace(("llp->start2 = %ld\n", (long)llp->start2));
	trace(("llp->length2 = %ld\n", (long)llp->length2));
	second_line = first_line - llp->length1;
	trace(("second_line = %ld\n", (long)second_line));
	if (second_line <= llp->length2)
	{
	    //
	    // Move the beginning of the second range to the end of the
	    // first range.
	    //
	    memmove
	    (
	    	llp->item + llp->start1 + llp->length1,
	    	llp->item + llp->start2,
		second_line * sizeof(llp->item[0])
	    );
	    llp->length1 += second_line;
	    llp->start2 += second_line;
	    llp->length2 -= second_line;
	    second_line = 0;

	    //
	    // If the second range is now empty, move it to the end.
	    //
	    if (llp->length2 == 0)
		llp->start2 = llp->maximum;

	    //
	    // Go around again, now that we are the right shape.
	    //
	    continue;
	}

	//
	// Oops.  This isn't means to happen.  It should also have been
	// caught by the assert at the beginning of this function.
	//
	assert(0);
	break;
    }

    CHECK(llp);
    trace(("}\n"));
}

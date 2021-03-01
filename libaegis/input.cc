//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2003-2006 Peter Miller
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
// MANIFEST: functions for reading input
//

#include <common/ac/stdarg.h>
#include <common/ac/string.h>

#include <common/error.h> // for assert
#include <common/mem.h>
#include <common/trace.h>
#include <libaegis/input.h>


input_ty::~input_ty()
{
    trace(("input_ty::~input_ty(this = %08lX)\n{\n", (long)this));
    assert(reference_count_valid());
    reference_count = -666;
    assert(buffer);
    mem_free(buffer);
    buffer = 0;
    buffer_position = 0;
    buffer_end = 0;
    buffer_size = 0;
    trace(("}\n"));
}


input_ty::input_ty() :
    reference_count(1),
    buffer(0),
    buffer_size(0),
    buffer_position(0),
    buffer_end(0)
{
    trace(("input_ty::input_ty(this = %08lX)\n{\n", (long)this));
    buffer_size = (size_t)1 << 14;
    buffer = (unsigned char *)mem_alloc(buffer_size);
    buffer_position = buffer;
    buffer_end = buffer;
    trace(("}\n"));
}


void
input_ty::reference_count_up()
{
    trace(("input_ty::reference_count_up(this = %08lX)\n{\n", (long)this));
    assert(reference_count_valid());
    reference_count++;
    trace(("count = %ld after\n", reference_count));
    trace(("}\n"));
}


void
input_ty::reference_count_down()
{
    trace(("input_ty::reference_count_down(this = %08lX)\n{\n", (long)this));
    trace(("count = %ld before\n", reference_count));
    assert(reference_count_valid());
    if (reference_count <= 1)
	delete this;
    else
	reference_count--;
    trace(("}\n"));
}


long
input_ty::read(void *data, size_t len)
{
    //
    // If they asked for nothing, give them nothing.
    //
    if (len <= 0)
	return 0;

    //
    // If there is anything in the buffer, return the contents of
    // the buffer.
    //
    if (buffer_position < buffer_end)
    {
	size_t nbytes = buffer_end - buffer_position;
	if (nbytes > len)
	    nbytes = len;
	memcpy(data, buffer_position, nbytes);
	buffer_position += nbytes;
	return nbytes;
    }

    //
    // The buffer is empty.  Read the data directly into the
    // destination.  There is no profit in double handling.
    //
    return read_inner(data, len);
}


int
input_ty::getc_complicated()
{
    //
    // If there is anything in the buffer, return the first byte of the
    // buffer.  This should never happen, because the inline getch method
    // is supposed to make it go away.
    //
    if (buffer_position < buffer_end)
    {
	assert(0);
	return *buffer_position++;
    }

    //
    // Fill the buffer with data, and then return the first byte of
    // the new buffer.
    //
    long nbytes = read_inner(buffer, buffer_size);
    if (nbytes <= 0)
	return (-1);
    buffer_position = buffer;
    buffer_end = buffer + nbytes;
    return *buffer_position++;
}


void
input_ty::ungetc_complicated(int c)
{
    if (c < 0)
    {
	//
	// Toss the end-of-file indicator.
	//
	return;
    }
    if (buffer_position > buffer)
    {
	//
        // If there is room in the buffer, back up and put the character
        // in the buffer.  The inline ungetc method is supposed to have
        // taken care of this already.
	//
	assert(0);
    }
    else if (buffer_position >= buffer_end)
    {
	//
        // If the buffer is empty, just mangle the pointers to make it
        // possible to push the character back.
        //
	buffer_end = buffer + buffer_size;
	buffer_position = buffer_end;
    }
    else
    {
	//
	// Double the size of the buffer, moving the current
	// data into the second half.  That way, there will
	// always be enough room for the old data and always
	// enough room for the character to be pushed back.
	//
        // By doubling the buffer size every time (and halving the
        // probability we will need to grow again) it is still O(1)
        // overall.
	//
	buffer_size *= 2;
	unsigned char *tmp = (unsigned char *)mem_alloc(buffer_size);
	size_t nbytes = buffer_end - buffer_position;
	memcpy(tmp + buffer_size - nbytes, buffer_position, nbytes);
	mem_free(buffer);
	buffer = tmp;
	buffer_end = buffer + buffer_size;
	buffer_position = buffer_end - nbytes;
    }

    //
    // The character goes before the current pointer,
    // and the current pointer is moved back by one.
    //
    *--buffer_position = c;
}


void
input_ty::unread(const void *data, size_t len)
{
    if (buffer_position >= buffer_end)
    {
	//
	// If the buffer is empty, just mangle the pointers to
	// make it possible to push the characters back.
	//
	buffer_end = buffer + buffer_size;
	buffer_position = buffer_end - len;
	memcpy(buffer_position, data, len);
	return;
    }

    //
    // Make the buffer large enough to hold the pushback.
    //
    while (buffer_position - len < buffer)
    {
	//
	// Double the size of the buffer, moving the current
	// data into the second half.  That way, there will
	// always be enough room for the old data and always
	// enough room for the character to be pushed back.
	//
        // By doubling the buffer size every time (and halving the
        // probability we will need to grow again) it is still O(1)
        // overall.
	//
	buffer_size *= 2;
	unsigned char *tmp = (unsigned char *)mem_alloc(buffer_size);
	size_t nbytes = buffer_end - buffer_position;
	memcpy(tmp + buffer_size - nbytes, buffer_position, nbytes);
	mem_free(buffer);
	buffer = tmp;
	buffer_end = buffer + buffer_size;
	buffer_position = buffer_end - nbytes;
    }

    //
    // Copy the data into the buffer, before the current position.
    //
    buffer_position -= len;
    memcpy(buffer_position, data, len);
}


long
input_ty::ftell()
{
    //
    // The underlying file is going to thinks we are further along
    // than we do, because of the read-ahead we did to fill the
    // buffer.
    //
    // To correct for this, we subtract what's left in the buffer
    // from where the underlying file thinks we are.
    //
    return ftell_inner() + buffer_position - buffer_end;
}


void
input_ty::keepalive()
{
}


bool
input_ty::at_end()
{
    return (peek() < 0);
}


bool
input_ty::is_remote()
    const
{
    return false;
}

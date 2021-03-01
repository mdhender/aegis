/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions for reading input
 */

#include <ac/string.h>

#include <error.h> /* for assert */
#include <input/private.h>
#include <mem.h>


long
input_read(ip, data, len)
	input_ty	*ip;
	void		*data;
	size_t		len;
{
	/*
	 * If they asked for nothing, given them nothing.
	 */
	if (len <= 0)
		return 0;

	/*
	 * If there is anything in the buffer, return the contents of
	 * the buffer.
	 */
	assert(ip);
	if (ip->buffer_position < ip->buffer_end)
	{
		size_t		nbytes;

		nbytes = ip->buffer_end - ip->buffer_position;
		if (nbytes > len)
			nbytes = len;
		memcpy(data, ip->buffer_position, nbytes);
		ip->buffer_position += nbytes;
		return nbytes;
	}

	/*
	 * The buffer is empty.  Read the data directly into the
	 * destination.  There is no profit in double handling.
	 */
	assert(ip->vptr);
	assert(ip->vptr->read);
	return ip->vptr->read(ip, data, len);
}


int
input_getc_complicated(ip)
	input_ty	*ip;
{
	long		nbytes;

	/*
	 * If there is anything in the buffer, return the first byte of
	 * the buffer.	This should never happen, because the #defien
	 * is supposed to make it go away.
	 */
	assert(ip);
	if (ip->buffer_position < ip->buffer_end)
	{
		assert(0);
		return *ip->buffer_position++;
	}

	/*
	 * Fill the buffer with data, and then return the first byte of
	 * the new buffer.
	 */
	assert(ip->vptr);
	assert(ip->vptr->read);
	nbytes = ip->vptr->read(ip, ip->buffer, ip->buffer_size);
	if (nbytes <= 0)
		return (-1);
	ip->buffer_position = ip->buffer;
	ip->buffer_end = ip->buffer + nbytes;
	return *ip->buffer_position++;
}


void
input_ungetc_complicated(ip, c)
	input_ty	*ip;
	int		c;
{
	if (c < 0)
	{
		/*
		 * Toss the end-of-file indicator.
		 */
		return;
	}
	if (ip->buffer_position > ip->buffer)
	{
		/*
		 * If there is room in the buffer, back up and put the
		 * character in the buffer.  The #define is supposed to
		 * have taken care of this already.
		 */
		assert(0);
	}
	else if (ip->buffer_position >= ip->buffer_end)
	{
		/*
		 * If the buffer is empty, just mangle the pointers to
		 * make it possible to push the character back.
		 */
		ip->buffer_end = ip->buffer + ip->buffer_size;
		ip->buffer_position = ip->buffer_end;
	}
	else
	{
		unsigned char	*tmp;
		size_t		nbytes;

		/*
		 * Double the size of the buffer, moving the current
		 * data into the second half.  That way, there will
		 * always be enough room for the old data and always
		 * enough room for the character to be pushed back.
		 *
		 * By doubling the buffer size every time,
		 * it is still O(1) overall.
		 */
		ip->buffer_size *= 2;
		tmp = mem_alloc(ip->buffer_size);
		nbytes = ip->buffer_end - ip->buffer_position;
		memcpy(tmp + ip->buffer_size - nbytes, ip->buffer_position, nbytes);
		mem_free(ip->buffer);
		ip->buffer = tmp;
		ip->buffer_end = ip->buffer + ip->buffer_size;
		ip->buffer_position = ip->buffer_end - nbytes;
	}
	
	/*
	 * The character goes before the current pointer,
	 * and the current pointer is moved back by one.
	 */
	ip->buffer_position--;
	*ip->buffer_position = c;
}


void
input_unread(ip, data, len)
	input_ty	*ip;
	const void	*data;
	size_t		len;
{
	if (ip->buffer_position >= ip->buffer_end)
	{
		/*
		 * If the buffer is empty, just mangle the pointers to
		 * make it possible to push the characters back.
		 */
		ip->buffer_end = ip->buffer + ip->buffer_size;
		ip->buffer_position = ip->buffer_end - len;
		memcpy(ip->buffer_position, data, len);
		return;
	}

	/*
	 * Make the buffer large enough to hold the pushback.
	 */
	while (ip->buffer_position - len < ip->buffer)
	{
		unsigned char	*tmp;
		size_t		nbytes;

		/*
		 * Double the size of the buffer, moving the current
		 * data into the second half.  That way, there will
		 * always be enough room for the old data and always
		 * enough room for the character to be pushed back.
		 *
		 * By doubling the buffer size every time,
		 * it is still O(1) overall.
		 */
		ip->buffer_size *= 2;
		tmp = mem_alloc(ip->buffer_size);
		nbytes = ip->buffer_end - ip->buffer_position;
		memcpy(tmp + ip->buffer_size - nbytes, ip->buffer_position, nbytes);
		mem_free(ip->buffer);
		ip->buffer = tmp;
		ip->buffer_end = ip->buffer + ip->buffer_size;
		ip->buffer_position = ip->buffer_end - nbytes;
	}

	/*
	 * Copy the data into the buffer, before the current position.
	 */
	ip->buffer_position -= len;
	memcpy(ip->buffer_position, data, len);
}


long
input_ftell(fp)
	input_ty	*fp;
{
	/*
	 * The underlying file is going to thinks we are further along
	 * than we do, because of the read-ahead we did to fill the
	 * buffer.
	 *
	 * To correct for this, we subtract what's left in the buffer
	 * from where the underlying file thinks we are.
	 */
	assert(fp);
	assert(fp->vptr);
	assert(fp->vptr->ftell);
	return fp->vptr->ftell(fp) + fp->buffer_position - fp->buffer_end;
}


void
input_delete(ip)
	input_ty	*ip;
{
	assert(ip);
	assert(ip->vptr);
	assert(ip->buffer);
	if (ip->vptr->destruct)
		ip->vptr->destruct(ip);
	mem_free(ip->buffer);
	ip->vptr = 0;
	ip->buffer = 0;
	ip->buffer_position = 0;
	ip->buffer_end = 0;
	ip->buffer_size = 0;
	mem_free(ip);
}

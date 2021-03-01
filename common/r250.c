/*
 *	aegis - project change supervisor
 *	Copyright (C) 1992-1995, 2002 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 1, or (at your option)
 *	any later version.
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
 * MANIFEST: good fast random number generator
 *
 * from Kirkpatrick, S. and E. Stoll,
 * "A Very Fast Shift-Register Sequence Random Number Generator",
 * Journal of Computational Physics, vol. 40, p. 157 (1981).
 */

#include <ac/time.h>
#include <ac/stdlib.h>
#include <ac/unistd.h>

#include <r250.h>

static	unsigned long	buf[250];
static	unsigned long	*pos;


#define rand8() ((rand() >> 7) & 255)


void
r250_init(void)
{
    unsigned long   bit;
    unsigned long   *bp;

    /*
     * initialize crummy linear congruential
     */
    srand(time((time_t *)0) + getpid());

    /*
     * position to start of array
     */
    pos = buf;

    /*
     * initialise contents of array
     */
    for (bp = buf; bp < ENDOF(buf); ++bp)
    {
	*bp =
	    (
		((unsigned long)rand8() << 24)
	    |
		((unsigned long)rand8() << 16)
	    |
		((unsigned long)rand8() << 8)
	    |
		(unsigned long)rand8()
	    );
    }

    /*
     * make sure the bits are linearly independent
     */
    for (bit = 1, bp = buf + 3; bit; bp += 11, bit <<= 1)
    {
	if (bp >= ENDOF(buf))
    	    bp -= SIZEOF(buf);
	*bp = (*bp & ~(bit - 1)) | bit;
    }
}


unsigned long
r250(void)
{
    unsigned long   result;
    unsigned long   *other;

    other = pos + 103;
    if (other >= ENDOF(buf))
	other -= SIZEOF(buf);
    *pos ^= *other;
    result = *pos++;
    if (pos >= ENDOF(buf))
	pos = buf;
    return result;
}

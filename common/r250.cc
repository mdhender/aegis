//
// aegis - project change supervisor
// Copyright (C) 1992-1995, 2002-2006, 2008 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see
// <http://www.gnu.org/licenses/>.
//
//
//
// from Kirkpatrick, S. and E. Stoll,
// "A Very Fast Shift-Register Sequence Random Number Generator",
// Journal of Computational Physics, vol. 40, p. 157 (1981).
//

#include <common/ac/stdlib.h>
#include <common/ac/unistd.h>

#include <common/now.h>
#include <common/r250.h>

static unsigned long buf[250];
static unsigned long *pos;


#define rand8() ((rand() >> 7) & 255)


static void
r250_init(void)
{
    unsigned long   bit;
    unsigned long   *bp;

    //
    // initialize crummy linear congruential
    //
    srand(now() + getpid());

    //
    // position to start of array
    //
    pos = buf;

    //
    // initialise contents of array
    //
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

    //
    // make sure the bits are linearly independent
    //
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
    if (!pos)
	r250_init();
    unsigned long *other = pos + 103;
    if (other >= ENDOF(buf))
	other -= SIZEOF(buf);
    *pos ^= *other;
    unsigned long result = *pos++;
    if (pos >= ENDOF(buf))
	pos = buf;
    return result;
}

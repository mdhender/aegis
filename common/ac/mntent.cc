//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002, 2004 Peter Miller;
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
// MANIFEST: functions to implement mntent stubs
//

#include <ac/mntent.h>


#ifndef HAVE_MNTENT_H

FILE *
setmntent(const char *filename, const char *mode)
{
    //
    // This is a bogus value.  All that is necessary is that it not be
    // zero (otherwise it looks like the file could not be opened).
    // By choosing a power of two, we avoid alignment issues, too.
    //
    return (FILE *)1024;
}


struct mntent *
getmntent(FILE *fp)
{
    // do nothing
    return 0;
}


int
endmntent(FILE *fp)
{
    return 0;
}

#endif // HAVE_MNTENT_H
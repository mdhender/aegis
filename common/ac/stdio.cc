//
//	aegis - project change supervisor
//	Copyright (C) 2002, 2004 Peter Miller;
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
// MANIFEST: functions to manipulate stdios
//

#include <ac/stdarg.h>
#include <ac/stdio.h>


#ifndef HAVE_SNPRINTF

int
snprintf(char *buffer, size_t nbytes, const char *fmt, ...)
{
    va_list         ap;

    va_start(ap, fmt);
    vsnprintf(buffer, nbytes, fmt, ap);
    va_end(ap);
}

#endif


#ifndef HAVE_VSNPRINTF

int
vsnprintf(char *buffer, size_t nbytes, const char *fmt, ...)
{
    vsprintf(buffer, fmt, ap);
}

#endif
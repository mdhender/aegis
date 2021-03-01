//
//	aegis - project change supervisor
//	Copyright (C) 2007 Peter Miller
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
// MANIFEST: interface of the format_elpsd class
//

#ifndef COMMON_FORMAT_ELPSD_H
#define COMMON_FORMAT_ELPSD_H

#include <common/ac/time.h>

class nstring; // forward

/**
  * The format_elapsed function is used to format an elapsed number of
  * seconds into a (terse) human readable string.
  *
  * @param buffer
  *     The buffer into which the string is to be formatted.
  * @param buflen
  *     The size of the buffer in bytes.  The result string will never
  *     use more than this many bytes.  It includes the terminating NUL
  *     character.
  * @param number_of_seconds
  *     The number of seconds to be formatted.
  */
void format_elapsed(char *buffer, size_t buflen, time_t number_of_seconds);

/**
  * The format_elapsed function is used to format an elapsed number of
  * seconds into a (terse) human readable string.
  *
  * @param number_of_seconds
  *     The number of seconds to be formatted.
  * @returns
  *     a string containing the elapsed time
  */
nstring format_elapsed(time_t number_of_seconds);

#endif // COMMON_FORMAT_ELPSD_H

//
//	aegis - project change supervisor
//	Copyright (C) 1997, 1999, 2005, 2006, 2008 Peter Miller
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

#ifndef COMMON_GMATCH_H
#define COMMON_GMATCH_H

#include <common/main.h>

/** \addtogroup Misc
  * @{
  */

/**
  * The  gmatch()  function checks whether the "candidate" argument matches the
  * "pattern" argument, which is a shell wildcard pattern.
  *
  * \param pattern
  *     The pattern to be matched.
  *     See glob(3) for a description of file name patterns.
  * \param candidate
  *     The string to be tested for matching against the pattern.
  * \returns
  *     int; 1 for a match, 0 for no match, -1 for invalid pattern.
  */
int gmatch(const char *pattern, const char *candidate);

class nstring; // forward

/**
  * The  gmatch()  function checks whether the "candidate" argument matches the
  * "pattern" argument, which is a shell wildcard pattern.
  *
  * \param pattern
  *     The pattern to be matched.
  *     See glob(3) for a description of file name patterns.
  * \param candidate
  *     The string to be tested for matching against the pattern.
  * \returns
  *     int; 1 for a match, 0 for no match, -1 for invalid pattern.
  */
int gmatch(const nstring &pattern, const nstring &candidate);

/** @} */
#endif // COMMON_GMATCH_H

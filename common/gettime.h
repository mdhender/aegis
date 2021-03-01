//
//	aegis - project change supervisor
//	Copyright (C) 1992-1994, 1997, 2003, 2005, 2006, 2008 Peter Miller
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

#ifndef COMMON_GETTIME_H
#define COMMON_GETTIME_H

/** \addtogroup Date
  * \brief Date and time manipulation
  * \ingroup Common
  * @{
  */

#include <common/ac/time.h>
#include <common/main.h>

time_t date_scan(const char *);
const char *date_string(time_t);

/** @} */
#endif // COMMON_GETTIME_H

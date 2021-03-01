//
//	aegis - project change supervisor
//	Copyright (C) 2003, 2005, 2006, 2008 Peter Miller
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

#ifndef COMMON_PAGE_H
#define COMMON_PAGE_H

#include <common/main.h>

/** \addtogroup Page
  * \brief Page size manipulation
  * \ingroup Common
  * @{
  */

//
// These error values are returned by the page_width_set and
// page_length_set functions.
//
#define PAGE_SET_ERROR_NONE 0
#define PAGE_SET_ERROR_DUPLICATE -1
#define PAGE_SET_ERROR_RANGE -2

#define DEFAULT_PRINTER_WIDTH 132
#define DEFAULT_PRINTER_LENGTH 66

#define MAX_PAGE_WIDTH 5000

int page_width_set(int columns);
int page_width_get(int);
int page_length_set(int rows);
int page_length_get(int);

/** @} */
#endif // COMMON_PAGE_H

//
//	aegis - project change supervisor
//	Copyright (C) 1997, 2003, 2005, 2006, 2008 Peter Miller
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

#ifndef COMMON_ABBREVIATE_H
#define COMMON_ABBREVIATE_H

/** \addtogroup abbreviate
  * \brief Abbreviate file and directory names
  * \ingroup Common
  * @{
  */
#include <common/ac/stddef.h>

struct string_ty *abbreviate_filename(struct string_ty *, size_t);
struct string_ty *abbreviate_dirname(struct string_ty *, size_t);
struct string_ty *abbreviate_8dos3(struct string_ty *);

/** @} */
#endif // COMMON_ABBREVIATE_H

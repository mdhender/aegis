//
//	aegis - project change supervisor
//	Copyright (C) 2005, 2006, 2008 Peter Miller
//	Copyright (C) 2004 Walter Franzini;
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

#ifndef COMMON_UUIDENTIFIER_H
#define COMMON_UUIDENTIFIER_H

#include <common/nstring.h>

/** \addtogroup UUID
  * \brief UUID wrapper
  * \ingroup Common
  * @{
  */

#define ORIGINAL_UUID "original-UUID"
#define EDIT_ORIGIN_UUID "edit-origin-UUID"

/**
  * \brief Returns a string containing a new UUID.
  *
  * The universal_unique_identifier function is used to generate a
  * universally unique identifier (UUID).
  *
  * @returns
  *     a string containing a valid UUID, it is guaranteed to be in
  *     lower case.
  */
string_ty *universal_unique_identifier(void);

/**
  * \brief Check if the UUID passed as input is valid.
  */
bool universal_unique_identifier_valid(string_ty *);

/**
  * \brief Check if the UUID passed as input is valid.
  */
bool universal_unique_identifier_valid(const nstring &arg);

/**
  * \brief Check if the UUID passed as input is valid, or a valid
  * leading partial prefix.
  */
bool universal_unique_identifier_valid_partial(string_ty *);

/**
  * \brief Check if the UUID passed as input is valid, or a valid
  * leading partial prefix.
  */
bool universal_unique_identifier_valid_partial(const nstring &arg);

/** @} */
#endif // COMMON_UUIDENTIFIER_H

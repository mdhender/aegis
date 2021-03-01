//
//	aegis - project change supervisor
//	Copyright (C) 2005, 2006, 2008 Peter Miller
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

#ifndef LIB_BOOLEAN_H
#define LIB_BOOLEAN_H

#include <common/nstring.h>


/**
  * The string_to_bool function is used to translate strings into
  * boolean values.
  *
  * @param value
  *     The value of the string to be converted
  * @param dflt
  *     The default value if the value is empty
  * @returns
  *     a boolean value
  */
bool string_to_bool(const nstring &value, bool dflt = false);

/**
  * The bool_to_string function is used to turn a boolean value into a
  * string.
  *
  * @param value
  *     The value to be converted.
  * @returns
  *     a string; either "true" or "false"
  */
nstring bool_to_string(bool value);

#endif // LIB_BOOLEAN_H

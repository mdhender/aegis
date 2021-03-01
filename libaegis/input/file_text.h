//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2001, 2002, 2005-2008 Peter Miller
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

#ifndef LIBAEGIS_INPUT_FILE_TEXT_H
#define LIBAEGIS_INPUT_FILE_TEXT_H

#include <libaegis/input.h>

struct string_ty; // forward
class nstring; // forward

/**
  * The input_file_text_open function is used to open text files.
  *
  * @param filename
  *     The name of the file to be opened.
  * @param escaped_newlines
  *     True if the newline escape filter is to be applied.
  * @returns
  *     smart point to open file
  */
input input_file_text_open(struct string_ty *filename, bool escape_nl = false);

/**
  * The input_file_text_open function is used to open text files.
  *
  * @param filename
  *     The name of the file to be opened.
  * @param escaped_newlines
  *     True if the newline escape filter is to be applied.
  * @returns
  *     smart point to open file
  */
input input_file_text_open(const nstring &filename, bool escape_nl = false);

#endif // LIBAEGIS_INPUT_FILE_TEXT_H

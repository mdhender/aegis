//
//	aegis - project change supervisor
//	Copyright (C) 2003, 2005-2008 Peter Miller
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

#ifndef LIBAEGIS_EMIT_EDIT_NUMBER_H
#define LIBAEGIS_EMIT_EDIT_NUMBER_H

struct fstate_src_ty; // forward
class project_file_roll_forward; // forward

/**
  * The emit_edit_number function is used to print the edit number
  * (possibly with HTML links) on the standard output.
  *
  * @param cp
  *     The change in question
  * @param src
  *     The file in question
  * @param hp
  *     The historian to consult for ancient file versions, or NULL if
  *     no historian is available.
  */
void emit_edit_number(change::pointer cp, fstate_src_ty *src,
    project_file_roll_forward *hp);

#endif // LIBAEGIS_EMIT_EDIT_NUMBER_H

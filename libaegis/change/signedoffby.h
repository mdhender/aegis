//
//	aegis - project change supervisor
//	Copyright (C) 2004 Peter Miller;
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
// MANIFEST: interface of the change_signedoffby class
//

#ifndef LIBAEGIS_CHANGE_SIGNEDOFFBY_H
#define LIBAEGIS_CHANGE_SIGNEDOFFBY_H

struct change_ty; // forward
struct user_ty; // forward

/**
  * The change_signed_off_by function is used to append the email of
  * the given user to the end of the description, using the OSDL DCO
  * (Signed-off-by) line.  Redundant sign-offs are not inserted.
  */
void change_signed_off_by(change_ty *cp, user_ty *up);

/**
  * The change_signed_off_by_get function is used to get the pconf
  * setting of the signed_off_by field, or as overridden by the -sob or
  * -nsob command line options.
  */
bool change_signed_off_by_get(change_ty *cp);

#endif // LIBAEGIS_CHANGE_SIGNEDOFFBY_H

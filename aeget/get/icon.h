//
//	aegis - project change supervisor
//	Copyright (C) 2005 Peter Miller;
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
// MANIFEST: interface of the get_icon class
//

#ifndef AEGET_GET_ICON_H
#define AEGET_GET_ICON_H

struct string_ty; // forward
struct string_list_ty; // forward

/**
  * Get an icon.
  *
  * This function dumps the given icon file (with a leading Content
  * Mime-type header) to stdout.
  *
  * @param path
  *     The path in the GET request after aeget/
  * @param modifier
  *     The modifiers after @@ or ?
  */
void get_icon(string_ty *path, string_list_ty *modifier);

#endif // AEGET_GET_ICON_H

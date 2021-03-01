//
//	aegis - project change supervisor
//	Copyright (C) 1997, 2002, 2003, 2005-2008 Peter Miller
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

#ifndef AEFIND_FUNCTION_NEEDS_H
#define AEFIND_FUNCTION_NEEDS_H

class tree_list; // forward

void function_needs_one(const char *name, const tree_list &args);
void function_needs_two(const char *name, const tree_list &args);

#endif // AEFIND_FUNCTION_NEEDS_H

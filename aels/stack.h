//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2002, 2005-2008 Peter Miller
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

#ifndef AELS_STACK_H
#define AELS_STACK_H

#include <common/str.h>

struct project_ty; // forward

void stack_from_change(change::pointer );
void stack_from_project(struct project_ty *);
string_ty *stack_relative(string_ty *);
string_ty *stack_nth(size_t);

#endif // AELS_STACK_H

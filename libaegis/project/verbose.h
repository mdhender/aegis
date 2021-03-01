//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2002, 2005, 2006, 2008 Peter Miller
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

#ifndef LIBAEGIS_PROJECT_VERBOSE_H
#define LIBAEGIS_PROJECT_VERBOSE_H

#include <common/main.h>

struct project_ty; // forward
struct string_ty; // forward

void project_verbose_new_project_complete(struct project_ty *);
void project_verbose_new_branch_complete(struct project_ty *);
void project_verbose_directory(struct project_ty *, struct string_ty *);

#endif // LIBAEGIS_PROJECT_VERBOSE_H

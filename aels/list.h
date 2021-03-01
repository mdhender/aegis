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

#ifndef AELS_LIST_H
#define AELS_LIST_H

#include <common/str_list.h>

struct project_ty; // forward

extern int recursive_flag;
extern int long_flag;
extern int show_dot_files;
extern int show_removed_files;
extern int mode_flag;
extern int attr_flag;
extern int user_flag;
extern int group_flag;
extern int size_flag;
extern int when_flag;

void list(string_list_ty *, struct project_ty *, change::pointer );

#endif // AELS_LIST_H

//
//	aegis - project change supervisor
//	Copyright (C) 1993, 1994, 2002, 2005, 2006, 2008 Peter Miller.
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

#ifndef AEGIS_UNAME_H
#define AEGIS_UNAME_H

#include <common/str.h>

string_ty *uname_variant_get(void);
string_ty *uname_node_get(void);

#endif // AEGIS_UNAME_H

//
//	aegis - project change supervisor
//	Copyright (C) 1998, 2002, 2005-2008 Peter Miller
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

#ifndef LIBAEGIS_SUB_TRIM_DIRECTO_H
#define LIBAEGIS_SUB_TRIM_DIRECTO_H

class sub_context_ty; // forward
class wstring; // forward
class wstring_list; // forward

wstring sub_trim_directory(sub_context_ty *, const wstring_list &);

#endif // LIBAEGIS_SUB_TRIM_DIRECTO_H

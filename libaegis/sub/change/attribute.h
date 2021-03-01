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
// MANIFEST: interface of the sub_change_attribute class
//

#ifndef LIBAEGIS_SUB_CHANGE_ATTRIBUTE_H
#define LIBAEGIS_SUB_CHANGE_ATTRIBUTE_H

struct wstring_ty; // forward
struct sub_context_ty; // forward
struct wstring_list_ty; // forward

wstring_ty *sub_change_attribute(sub_context_ty *, wstring_list_ty *arg);

#endif // LIBAEGIS_SUB_CHANGE_ATTRIBUTE_H

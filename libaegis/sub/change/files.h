//
//	aegis - project change supervisor
//	Copyright (C) 2003, 2005-2007 Peter Miller
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
// MANIFEST: interface definition for libaegis/sub/change/files.c
//

#ifndef LIBAEGIS_SUB_CHANGE_FILES_H
#define LIBAEGIS_SUB_CHANGE_FILES_H

class sub_context_ty; // forward
class wstring; // forward
class wstring_list; // forward

wstring sub_change_files(sub_context_ty *, const wstring_list &);

#endif // LIBAEGIS_SUB_CHANGE_FILES_H

//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002, 2005, 2006 Peter Miller;
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
// MANIFEST: interface definition for libaegis/project/file/trojan.c
//

#ifndef LIBAEGIS_PROJECT_FILE_TROJAN_H
#define LIBAEGIS_PROJECT_FILE_TROJAN_H

#include <common/main.h>

struct project_ty; // existence
struct string_ty; // existence

int project_file_trojan_suspect(struct project_ty *, struct string_ty *);

#endif // LIBAEGIS_PROJECT_FILE_TROJAN_H

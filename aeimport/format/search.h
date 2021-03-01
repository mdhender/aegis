//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2002, 2004-2006, 2008 Peter Miller
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

#ifndef AEIMPORT_FORMAT_SEARCH_H
#define AEIMPORT_FORMAT_SEARCH_H

#include <aeimport/format/version.h>

struct format_search_ty
{
    string_ty       *filename_physical;
    string_ty       *filename_logical;
    format_version_ty *root;
};

format_search_ty *format_search_new(void);
void format_search_delete(format_search_ty *);

struct string_list_ty; // forward
void format_search_staff(format_search_ty *, struct string_list_ty *);
void format_search_validate(format_search_ty *);

#endif // AEIMPORT_FORMAT_SEARCH_H

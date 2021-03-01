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

#ifndef AEIMPORT_FORMAT_VERSION_LIST_H
#define AEIMPORT_FORMAT_VERSION_LIST_H

#include <aeimport/format/version.h>

struct format_version_list_ty
{
    format_version_ty **item;
    size_t          length;
    size_t	    maximum;
};

format_version_list_ty *format_version_list_new(void);
void format_version_list_delete(format_version_list_ty *, int);
void format_version_list_append(format_version_list_ty *, format_version_ty *);
void format_version_list_sort_by_date(format_version_list_ty *);
void format_version_list_validate(format_version_list_ty *);

#endif // AEIMPORT_FORMAT_VERSION_LIST_H

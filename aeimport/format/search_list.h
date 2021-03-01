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

#ifndef AEIMPORT_FORMAT_SEARCH_LIST_H
#define AEIMPORT_FORMAT_SEARCH_LIST_H

#include <aeimport/format/search.h>


struct format_search_list_ty
{
    format_search_ty **item;
    size_t	    length;
    size_t	    maximum;
};

format_search_list_ty *format_search_list_new(void);
void format_search_list_constructor(format_search_list_ty *);
void format_search_list_destructor(format_search_list_ty *);
void format_search_list_delete(format_search_list_ty *);
void format_search_list_append(format_search_list_ty *, format_search_ty *);
void format_search_list_staff(format_search_list_ty *, struct string_list_ty *);
void format_search_list_validate(format_search_list_ty *);

/**
  * The format_search_list_query function is used to determine if a
  * given filename is present in the formatsearch list.
  */
format_search_ty *format_search_list_query(format_search_list_ty *,
    string_ty *);

#endif // AEIMPORT_FORMAT_SEARCH_LIST_H

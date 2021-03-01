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

#ifndef AEIMPORT_CHANGE_SET_LIST_H
#define AEIMPORT_CHANGE_SET_LIST_H

#include <aeimport/change_set.h>

struct change_set_list_ty
{
    size_t          length;
    size_t	    maximum;
    change_set_ty   **item;
};


change_set_list_ty *change_set_list_new(void);
void change_set_list_delete(change_set_list_ty *);
void change_set_list_append(change_set_list_ty *, change_set_ty *);
void change_set_list_sort_by_date(change_set_list_ty *);
void change_set_list_validate(change_set_list_ty *);

#endif // AEIMPORT_CHANGE_SET_LIST_H

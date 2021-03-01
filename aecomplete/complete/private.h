//
//	aegis - project change supervisor
//	Copyright (C) 2002-2006, 2008 Peter Miller
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

#ifndef AECOMPLETE_COMPLETE_PRIVATE_H
#define AECOMPLETE_COMPLETE_PRIVATE_H

#include <aecomplete/complete.h>

struct complete_vtbl_ty
{
    void (*destructor)(complete_ty *);
    void (*perform)(complete_ty *, struct shell_ty *);
    int size;
    const char *name;
};

complete_ty *complete_new(complete_vtbl_ty *);

#endif // AECOMPLETE_COMPLETE_PRIVATE_H

//
//	aegis - project change supervisor
//	Copyright (C) 2003, 2004 Peter Miller;
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
// MANIFEST: interface definition for libaegis/change/list.c
//

#ifndef LIBAEGIS_CHANGE_LIST_H
#define LIBAEGIS_CHANGE_LIST_H

#pragma interface "change_list_ty"

#include <main.h>

class change_list_ty
{
public:
    size_t          length;
    size_t          maximum;
    struct change_ty **item;

    /**
      * The destructor.
      */
    ~change_list_ty();

    /**
      * The constructor.
      */
    change_list_ty();

    /**
      * The change_list_append function is used to append a change to a
      * change list.
      */
    void append(change_ty *cp);

    /**
      * The change_list_member_p function is used to determine if a
      * given change is a member of the given change list.
      */
    bool member_p(change_ty *cp) const;

    /**
      * Empty the list of members.
      */
    void clear();

private:
    /**
      * The copy constructor.  Do not use.
      */
    change_list_ty(const change_list_ty &);

    /**
      * The assignment operator.  Do not use.
      */
    change_list_ty &operator=(const change_list_ty &);
};

#endif // LIBAEGIS_CHANGE_LIST_H

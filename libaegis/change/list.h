//
//	aegis - project change supervisor
//	Copyright (C) 2003-2008 Peter Miller
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

#ifndef LIBAEGIS_CHANGE_LIST_H
#define LIBAEGIS_CHANGE_LIST_H

#include <common/main.h>

class change_list_ty
{
public:
    size_t          length;
private:
    size_t          maximum;
public:
    change::pointer *item;

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
    void append(change::pointer cp);

    /**
      * The change_list_member_p function is used to determine if a
      * given change is a member of the given change list.
      */
    bool member_p(change::pointer cp) const;

    /**
      * Empty the list of members.
      */
    void clear();

    /**
      * The size method is used to obtain the number of changes in the
      * change list.
      */
    size_t size() const { return length; }

    /**
      * The empty method is used to determine whether or not the list is
      * empty.
      *
      * @returns
      *     bool; true if not elements in list, false if there are.
      */
    bool empty() const { return (length == 0); }

    /**
      * The get method is used to obtain an element of the
      * change list.
      *
      * @param n
      *     The array element number.  No range check is performed.
      */
    change::pointer get(size_t n) const { return item[n]; }

    /**
      * The [] operator is used to obtain an element of the
      * change list.
      *
      * @param n
      *     The array element number.  No range check is performed.
      */
    change::pointer operator[](size_t n) const { return get(n); }

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

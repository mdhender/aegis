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

#ifndef AEIMPORT_CHANGE_SET_FILE_LIST_H
#define AEIMPORT_CHANGE_SET_FILE_LIST_H

#include <aeimport/change_set/file.h>

/**
  * The change_set_file_list_ty class is used to represent the list of
  * files involved in a single change set.
  */
class change_set_file_list_ty
{
public:
    /**
      * The destructor.
      * DO NOT subclass me, I'm not virtual.
      */
    ~change_set_file_list_ty();

    /**
      * The default constructor.
      */
    change_set_file_list_ty();

    /**
      * The size method is used to obtain the number of items in the list.
      */
    size_t size() const { return length; }

    /**
      * The [] operator is used to obtain the nth element of the array.
      * This has O(1) performance.
      *
      * \note
      *     No bounds checking is performed.
      */
    change_set_file_ty *operator[](size_t n) const { return item[n]; }

    /**
      * The clear method may be used to discard all of the items in the
      * list.
      */
    void clear();

    /**
      * The push_back method is used to append an item to the end of the
      * list.  This has O(1) behaviour.
      */
    void push_back(change_set_file_ty *csfp);

    /**
      * The validate method is used at debug to to check that this list
      * is still valid.
      */
    void validate() const;

private:
    /**
      * The length instance variable is used to remember how many items
      * exist in the list.
      */
    size_t length;

    /**
      * The maximum instance variable is used to remember how many items
      * have been allocated in the item array.
      *
      * \assert (length <= maximum)
      */
    size_t maximum;

    /**
      * The item instance variable is used to remember pointers to the
      * items in the list.
      *
      * \assert (!item == !maximum)
      */
    change_set_file_ty **item;

    /**
      * The copy constructor.  Do not use.
      */
    change_set_file_list_ty(const change_set_file_list_ty &);

    /**
      * The assignment operator.  Do not use.
      */
    change_set_file_list_ty &operator=(const change_set_file_list_ty &);
};

inline void
change_set_file_list_constructor(change_set_file_list_ty *csflp)
{
    csflp->clear();
}

inline void
change_set_file_list_destructor(change_set_file_list_ty *csflp)
{
    csflp->clear();
}

inline void
change_set_file_list_append(change_set_file_list_ty *csflp, string_ty *arg2,
    string_ty *arg3, change_set_file_action_ty arg4, string_list_ty *arg5)
{
    csflp->push_back(new change_set_file_ty(arg2, arg3, arg4, arg5));
}

inline void
change_set_file_list_validate(change_set_file_list_ty *csflp)
{
    csflp->validate();
}

#endif // AEIMPORT_CHANGE_SET_FILE_LIST_H

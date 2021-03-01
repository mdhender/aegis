//
//	aegis - project change supervisor
//	Copyright (C) 1997, 2002, 2004-2008 Peter Miller
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

#ifndef AEFIND_TREE_LIST_H
#define AEFIND_TREE_LIST_H

#include <aefind/tree.h>


/**
  * The tree_list class is used to represent a list of pointers to
  * expression tree nodes.
  */
class tree_list
{
public:
    /**
      * The destructor.
      */
    ~tree_list();

    /**
      * The default constructor.
      */
    tree_list();

    /**
      * The copy constructor.
      */
    tree_list(const tree_list &arg);

    /**
      * The assignment operator.
      */
    tree_list &operator=(const tree_list &arg);

    /**
      * The clear method may be used to discard the contents of this
      * list.
      */
    void clear();

    /**
      * The append method may be used to add another expression node to
      * the back of this list.
      */
    void append(const tree::pointer &tp);

    /**
      * The append method may be used to add the contents of another
      * expression node list to the back of this list.
      */
    void append(const tree_list &arg);

    /**
      * The size method may be used to obtain the number of items in
      * this list.
      */
    size_t size() const { return length; }

    /**
      * The get method may be used to obtain the n-th element of this
      * list.
      *
      * @note
      *     No range checking is performed.
      *
      */
    tree::pointer get(size_t n) const;

    /**
      * The array index operator method may be used to obtain the n-th
      * element of this list.
      */
    tree::pointer operator[](size_t n) const { return get(n); }

private:
    /**
      * The item instance variable is used to remember the address of a
      * dynamically allocated array of pointer to expression tre nodes.
      */
    tree::pointer *item;

    /**
      * The length instance variable is used to remember how many
      * elements of the "item" array have been used.
      */
    size_t length;

    /**
      * The maximum instance variable is used to remember how many
      * elements were allocated to the "item" array.
      */
    size_t maximum;
};

#endif // AEFIND_TREE_LIST_H

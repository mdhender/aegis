//
//	aegis - project change supervisor
//	Copyright (C) 2007, 2008 Peter Miller
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

#ifndef LIBAEGIS_SUB_FUNCTOR_LIST_H
#define LIBAEGIS_SUB_FUNCTOR_LIST_H

#include <libaegis/sub/functor.h>

/**
  * The sub_functor_list class is used to represent a list ofr pointers
  * to sub_functor instances.
  */
class sub_functor_list
{
public:
    /**
      * The destructor.
      */
    virtual ~sub_functor_list();

    /**
      * The default constructor.
      */
    sub_functor_list();

    /**
      * The push_back method is used to append another functor to the
      * end of the fuctor list.
      *
      * @param sfp
      *     The functor to add to the end of the list
      */
    void push_back(sub_functor::pointer sfp);

    /**
      * The get method is used to obtain the <i>n</i>th element of the
      * list.
      *
      * @param n
      *     The number (zero based) of the element of interest.
      */
    sub_functor::pointer get(size_t n) const;

    /**
      * The array index operator method is used to obtain the <i>n</i>th
      * element of the list.
      *
      * @param n
      *     The number (zero based) of the element of interest.
      */
    sub_functor::pointer operator[](size_t n) const { return get(n); }

    /**
      * The size method is used to obtain the number of elements in the
      * list at the moment.
      */
    size_t size() const { return length; }

    /**
      * The empty method is used to determie whether or not the list is
      * empty at the moment.
      */
    bool empty() const { return !length; }

    /**
      * The match method is ised to test the gven string against the
      * names of all the functors in the list, treating the names as
      * arglex_compare patterns.
      *
      * @param name
      *     The name of the substitution being looked for
      * @param result
      *     each matching functor is appended to the list.
      *     It is possible for there to be no matches.
      */
    void match(const nstring &name, sub_functor_list &result);

    /**
      * The clear method is used to discard all contents of the list
      */
    void clear();

    /**
      * The find method is used to find a list element by name.  This
      * differs from the \a match method in that it looks for an exact
      * name match, not a pattern match.
      *
      * @param name
      *     The name of the functor to look for
      * @returns
      *     pointer to functor, or NULL if not found
      */
    sub_functor::pointer find(const nstring &name) const;

private:
    /**
      * The list instance variable is used to remember the address of
      * the base of the dynamically allocated array of pointers to
      * functors.
      */
    sub_functor::pointer *list;

    /**
      * The length instance variable is used to remember how much of the
      * array of pointer to functors has been used to date.
      */
    size_t length;

    /**
      * The maximum instance variable is used to remember the number of
      * elements allocated to the array of pointers to functors.
      */
    size_t maximum;

    /**
      * The copy constructor.  Do not use.
      */
    sub_functor_list(const sub_functor_list &);

    /**
      * The assignment operator.  Do not use.
      */
    sub_functor_list &operator=(const sub_functor_list &);
};

#endif // LIBAEGIS_SUB_FUNCTOR_LIST_H

//
//	aegis - project change supervisor
//	Copyright (C) 2004 Peter Miller;
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
// MANIFEST: interface of the change_functor_inventory_list class
//

#ifndef LIBAEGIS_CHANGE_FUNCTOR_INVENTO_LIST_H
#define LIBAEGIS_CHANGE_FUNCTOR_INVENTO_LIST_H

#pragma interface "change_functor_inventory_list"

#include <change/functor.h>

struct col_ty; // forward
struct output_ty; // forward
struct project_ty; // forward

/**
  * The change_functor_inventory_list class is used to represent the
  * listing state for the "ael cin" listing, a project change set
  * inventory listing.
  */
class change_functor_inventory_list:
    public change_functor
{
public:
    /**
      * The destructor.
      */
    virtual ~change_functor_inventory_list();

    /**
      * The constructor.
      */
    change_functor_inventory_list(project_ty *);

    // See base class for documentation.
    void operator()(change_ty *cp);

private:
    /**
      * The colp instance variable is used to remember the columnar
      * output aggregator from which the columns below spring.
      */
    col_ty *colp;

    /**
      * The vers_col instance variable is used to remember the output
      * stream in which to print the change's version (including the
      * branch and delta in one string).
      */
    output_ty *vers_col;

    /**
      * The uuid_col instance variable is used to remember the output
      * stream in which to print the UUID.
      */
    output_ty *uuid_col;

    /**
      * The when_col instance variable is used to remember the output
      * stream in which to print the data and time.  Is set to NULL if
      * no date and time is to be listed.
      */
    output_ty *when_col;

    /**
      * The default constructor.  Do not use.
      */
    change_functor_inventory_list();

    /**
      * The copy constructor.   Do not use.
      */
    change_functor_inventory_list(const change_functor_inventory_list &);

    /**
      * The assignment operator.   Do not use.
      */
    change_functor_inventory_list &operator=(
	const change_functor_inventory_list &);
};

#endif // LIBAEGIS_CHANGE_FUNCTOR_INVENTO_LIST_H

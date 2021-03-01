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
// MANIFEST: interface of the change_functor_inventory class
//

#ifndef AEGET_CHANGE_FUNCTOR_INVENTORY_H
#define AEGET_CHANGE_FUNCTOR_INVENTORY_H

#pragma interface "change_functor_inventory"

#include <change/functor.h>

struct project_ty; // forward

/**
  * The change_functor_inventory class is used to represent the action
  * to be performed by project_inventory_walk when called by aeget.
  */
class change_functor_inventory:
    public change_functor
{
public:
    /**
      * The destructor.
      */
    virtual ~change_functor_inventory();

    /**
      * The constructor.
      */
    change_functor_inventory(project_ty *pp);

    // See base class for documentation.
    void operator()(change_ty *);

private:
    /**
      * The print_one_line method is used by the functor operator to
      * print a row of the table.  Not only is the change's UUID field
      * printed, but also any original-uuid attributes - this allows
      * supression of change sets you don't want because another change
      * set has subsumed them.
      */
    void print_one_line(change_ty *, string_ty *);

    /**
      * The pp instance variabel is used to remember the project being listed.
      */
    project_ty *pp;

    /**
      * The num instance variable is used to remember the number of row
      * emitted to date.
      */
    int num;

    /**
      * The default constructor.  Do not use.
      */
    change_functor_inventory();

    /**
      * The copy constructor.  Do not use.
      */
    change_functor_inventory(const change_functor_inventory &);

    /**
      * The assignment operator.  Do not use.
      */
    change_functor_inventory &operator=(const change_functor_inventory &);
};

#endif // AEGET_CHANGE_FUNCTOR_INVENTORY_H

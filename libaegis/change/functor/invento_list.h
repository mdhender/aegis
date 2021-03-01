//
//	aegis - project change supervisor
//	Copyright (C) 2004-2008 Peter Miller
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

#ifndef LIBAEGIS_CHANGE_FUNCTOR_INVENTO_LIST_H
#define LIBAEGIS_CHANGE_FUNCTOR_INVENTO_LIST_H

#include <libaegis/change/functor.h>
#include <libaegis/col.h>

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
    change_functor_inventory_list(bool incbr, project_ty *pp);

    // See base class for documentation.
    void operator()(change::pointer cp);

private:
    /**
      * The print_one_line method is used by the functor operator to
      * print a row of the table.  Not only is the change's UUID field
      * printed, but also any original-uuid attributes - this allows
      * supression of change sets you don't want because another change
      * set has subsumed them.
      */
    void print_one_line(change::pointer , string_ty *);

    /**
      * The colp instance variable is used to remember the columnar
      * output aggregator from which the columns below spring.
      */
    col::pointer colp;

    /**
      * The vers_col instance variable is used to remember the output
      * stream in which to print the change's version (including the
      * branch and delta in one string).
      */
    output::pointer vers_col;

    /**
      * The uuid_col instance variable is used to remember the output
      * stream in which to print the UUID.
      */
    output::pointer uuid_col;

    /**
      * The when_col instance variable is used to remember the output
      * stream in which to print the data and time.  Is set to NULL if
      * no date and time is to be listed.
      */
    output::pointer when_col;

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

//
//	aegis - project change supervisor
//	Copyright (C) 2004-2008 Peter Miller
//	Copyright (C) 2007 Walter Franzini
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

#ifndef AEGET_CHANGE_FUNCTOR_INVENTORY_H
#define AEGET_CHANGE_FUNCTOR_INVENTORY_H

#include <common/nstring/list.h>
#include <libaegis/change/functor.h>

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
      *
      * @param incbr
      *     whether or not to recurse down brabches
      * @param pp
      *     The project in question.
      * @param include_original_uuid
      *     whether or not we want to include original-UUID attributes
      *     in the output.
      * @param uuid_links
      *     This flag is used to request the generation of UUID based
      *     links instead of version based links.
      * @param prefix
      *     The prefix to prepend to the URL used to retrieve the
      *     aedist archive.
      * @param suffix
      *     The suffix to be appended to the URL used to retrieve the
      *     aedist archive.  Usually ".ae".
      * @param include_change
      *     The list of version patterns used to filter changes from
      *     the inventory.
      * @param exclude_change
      *     The list of version patterns used to filter out changes
      *     from the inventory.
      */
    change_functor_inventory(bool incbr, project_ty *pp,
	bool include_original_uuid, bool uuid_links, const nstring &prefix,
        const char *suffix, const nstring_list&, const nstring_list&);

    // See base class for documentation.
    void operator()(change::pointer );

private:
    /**
      * The print_one_line method is used by the functor operator to
      * print a row of the table.  Not only is the change's UUID field
      * printed, but also any original-uuid attributes - this allows
      * supression of change sets you don't want because another change
      * set has subsumed them.
      */
    void print_one_line(change::pointer , const nstring &);

    /**
      * The pp instance variabel is used to remember the project being listed.
      */
    project_ty *pp;

    nstring_list exclude_change;

    nstring_list include_change;

    /**
      * The include_original_uuid instance variable is used to remember
      * whether or not we want to include original-UUID attributes in
      * the output.
      */
    bool include_original_uuid;

    bool generate_uuid_based_links;

    nstring prefix;

    nstring suffix;

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

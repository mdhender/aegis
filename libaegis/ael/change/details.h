//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002-2004, 2006-2008 Peter Miller
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

#ifndef LIBAEGIS_AEL_CHANGE_DETAILS_H
#define LIBAEGIS_AEL_CHANGE_DETAILS_H

#include <libaegis/change.h>
#include <libaegis/col.h>

struct output_ty; // forward
struct string_ty; // forward
struct string_list_ty; // forward
class change_identifier; // forward

/**
  * The list_change_details_columns class is sued to represent the
  * output shape for a Change Details listing, and for actually listing
  * change details.
  */
class list_change_details_columns
{
public:
    /**
      * Destructor.
      */
    ~list_change_details_columns();

    /**
      * Default constructor.
      */
    list_change_details_columns();

    /**
      * The list method is used to output the details of the given change.
      *
      * @param cp
      *     The change to be listed.
      * @param recurse
      *     If the change is actually a branch, recusively print the
      *     details of each of the changes on the branch.
      */
    void list(change::pointer cp, bool recurse);

private:
    col::pointer colp;

    // general columns
    output::pointer head_col;
    output::pointer body_col;

    /**
      * Copy constructor.  Do not use.
      */
    list_change_details_columns(const list_change_details_columns &);

    /**
      * Assignment operator.  Do not use.
      */
    list_change_details_columns &operator=(const list_change_details_columns &);
};

/**
  * The list_change_details function is used to list the deatils of a change.
  *
  * @param cid
  *     The change to be listed.
  * @param arguments
  *     Additional listing arguments.
  */
void list_change_details(change_identifier &cid, string_list_ty *arguments);

#endif // LIBAEGIS_AEL_CHANGE_DETAILS_H

//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002-2004 Peter Miller;
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
// MANIFEST: interface definition for libaegis/ael/change/details.c
//

#ifndef LIBAEGIS_AEL_CHANGE_DETAILS_H
#define LIBAEGIS_AEL_CHANGE_DETAILS_H

struct change_ty; // forward
struct col_ty; // forward
struct output_ty; // forward
struct string_ty; // forward
struct string_list_ty; // forward

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
    void list(change_ty *cp, bool recurse);

private:
    col_ty *colp;

    // general columns
    output_ty	    *head_col;
    output_ty	    *body_col;

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
  * @param project_name
  *     The name of the project the change is in.
  * @param change_number
  *     The number of the change to be listed.
  * @param arguments
  *     Additional listing arguments.
  */
void list_change_details(string_ty *project_name, long change_number,
    string_list_ty *arguments);

#endif // LIBAEGIS_AEL_CHANGE_DETAILS_H

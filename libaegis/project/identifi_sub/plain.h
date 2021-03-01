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
// MANIFEST: interface of the project_identifi_sub_plain class
//

#ifndef LIBAEGIS_PROJECT_IDENTIFI_SUB_PLAIN_H
#define LIBAEGIS_PROJECT_IDENTIFI_SUB_PLAIN_H

#pragma interface "project_identifier_subset_plain"

#include <project/identifi_sub.h>

/**
  * The project_identifier_subset_plain class is used to represent the
  * project identified by the --project line option, or one of the
  * defaulting mechanisms.
  */
class project_identifier_subset_plain:
    public project_identifier_subset
{
public:
    /**
      * The destructor.
      */
    virtual ~project_identifier_subset_plain();

    /**
      * The default constructor.
      */
    project_identifier_subset_plain();

    // See base class for documentation.
    void command_line_parse(usage_t);

    // See base class for documentation.
    void command_line_check(usage_t);

    // See base class for documentation.
    project_ty *get_pp();

    // See base class for documentation.
    user_ty *get_up();

    // See base class for documentation.
    void parse_change_with_branch(long &change_number, usage_t usage);

    // See base class for documentation.
    void parse_change_with_branch(long &change_number, const char *&branch_arg,
	usage_t usage);

private:
    /**
      * The project_name instance variable is used to remember any
      * --project option specified.
      */
    string_ty *project_name;

    /**
      * The pp instance variable is used to remember which project we
      * are bound to.
      */
    project_ty *pp;

    /**
      * The up instance variable is used to remember which user we are
      * working in behalf of.
      */
    user_ty *up;

    /**
      * The copy constructor.  Do not use.
      */
    project_identifier_subset_plain(const project_identifier_subset_plain &);

    /**
      * The assignment operator.  Do not use.
      */
    project_identifier_subset_plain &operator=(
	const project_identifier_subset_plain &);
};

#endif // LIBAEGIS_PROJECT_IDENTIFI_SUB_PLAIN_H

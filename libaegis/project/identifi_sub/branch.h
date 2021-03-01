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
// MANIFEST: interface of the project_identifier_subset_branch class
//

#ifndef LIBAEGIS_PROJECT_IDENTIFI_SUB_BRANCH_H
#define LIBAEGIS_PROJECT_IDENTIFI_SUB_BRANCH_H

#pragma interface "project_identifier_subset_branch"

#include <project/identifi_sub.h>

/**
  * The project_identifier_subset_branch class is used to represent the
  * project branch identified (implictly or implicitly) by --project and
  * --branch command line options.
  */
class project_identifier_subset_branch:
    public project_identifier_subset
{
public:
    /**
      * The destructor.
      */
    virtual ~project_identifier_subset_branch();

    /**
      * The constructor.
      */
    project_identifier_subset_branch(project_identifier_subset &);

    // See base class for documentation.
    void command_line_parse(usage_t);

    // See base class for documentation.
    void command_line_check(usage_t);

    // See base class for documentation.
    project_ty* get_pp();

    user_ty* get_up();

    // See base class for documentation.
    void parse_change_with_branch(long &change_number, usage_t usage);

    // See base class for documentation.
    void parse_change_with_branch(long &change_number, const char *&branch,
	usage_t usage);

private:
    /**
      * The deeper instance variable is used to remember the base
      * project to which our branch refers.
      */
    project_identifier_subset &deeper;

    /**
      * The pp_with_branch instance variable is usedc to remember which
      * project branch we are bound to.
      */
    project_ty *pp_with_branch;

    /**
      * The branch instance variable is used to remember any --branch
      * command line option specified.  This could be implicit.
      */
    const char *branch;

    /**
      * The grandparent instance variable is used to remember wether or
      * not the --grandparent command line option has been specified.
      */
    bool grandparent;

    /**
      * The trunk instance variable is used to remember wether or not
      * the --trunk command line option has been specified.
      */
    bool trunk;

    /**
      * The default constructor.  Do not use.
      */
    project_identifier_subset_branch();

    /**
      * The copy constructor.  Do not use.
      */
    project_identifier_subset_branch(const project_identifier_subset_branch &);

    /**
      * The assignment operator.  Do not use.
      */
    project_identifier_subset_branch &operator=(
	const project_identifier_subset_branch &);
};

#endif // LIBAEGIS_PROJECT_IDENTIFI_SUB_BRANCH_H

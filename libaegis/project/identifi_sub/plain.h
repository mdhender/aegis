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

#ifndef LIBAEGIS_PROJECT_IDENTIFI_SUB_PLAIN_H
#define LIBAEGIS_PROJECT_IDENTIFI_SUB_PLAIN_H

#include <libaegis/project/identifi_sub.h>

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
    user_ty::pointer get_up();

    // See base class for documentation.
    void set_user_by_name(nstring &login);

    // See base class for documentation.
    void parse_change_with_branch(long &change_number, usage_t usage);

    // See base class for documentation.
    void parse_change_with_branch(long &change_number, const char *&branch_arg,
	usage_t usage);

    // See base class for documentation.
    bool set() const;

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
    user_ty::pointer up;

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

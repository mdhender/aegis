//
//	aegis - project change supervisor
//	Copyright (C) 2004, 2005 Peter Miller;
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
// MANIFEST: interface of the project_identifier_subset class
//

#ifndef LIBAEGIS_PROJECT_IDENTIFI_SUB_H
#define LIBAEGIS_PROJECT_IDENTIFI_SUB_H

struct project_ty; // forward
struct string_ty; // forward
struct user_ty; // forward

/**
  * The project_identifier_subset class is used to represent the project
  * identified by command line options.
  */
class project_identifier_subset
{
public:
    /**
      * The destructor.
      */
    virtual ~project_identifier_subset();

    /**
      * The default constructor.
      */
    project_identifier_subset();

    typedef void (*usage_t)(void);

    /**
      * The command_line_parse method is used to parse command line
      * options (via the arglex() fucntion) to set the change ID.  The
      * current token is expected to be meaningful for identifying a
      * change.  The position will be advanced past all relevant tokens.
      *
      * It understands all of the following command line options:
      *     [ -Project ] string
      */
    virtual void command_line_parse(usage_t) = 0;

    /**
      * The command_line_check method is used to verify that sensable
      * command line options have been specified, once the parse has
      * completed.
      */
    virtual void command_line_check(usage_t) = 0;

    /**
      * The get_pp method is used to get the project pointer for the
      * change identified.
      */
    virtual project_ty *get_pp() = 0;

    /**
      * The get_up method is used to get the user pointer for the
      * change identified.
      */
    virtual user_ty *get_up() = 0;

    /**
      * The parse_change_with_branch method is used to parse the
      * --change command line option, which may be in a number of forms.
      *
      * This method should only be called from the
      * change_identifier_subset::command_line_parse method.
      */
    virtual void parse_change_with_branch(long &change_number,
	usage_t usage) = 0;

    /**
      * The parse_change_with_branch method is used to parse the
      * --change command line option, when it could be given a 1.2.C345
      * style number.
      *
      * This method should only be called from the
      * change_identifier_subset::command_line_parse method.
      */
    virtual void parse_change_with_branch(long &change_number,
	const char *&branch_arg, usage_t usage) = 0;

private:
    /**
      * The copy constructor.  Do not use.
      */
    project_identifier_subset(const project_identifier_subset &);

    /**
      * The assignment operator.  Do not use.
      */
    project_identifier_subset &operator=(const project_identifier_subset &);
};

#endif // LIBAEGIS_PROJECT_IDENTIFI_SUB_H

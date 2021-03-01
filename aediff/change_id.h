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
// MANIFEST: interface of the change_id class
//

#ifndef AEDIFF_CHANGE_ID_H
#define AEDIFF_CHANGE_ID_H

#pragma interface "change_id"

#include <file_version.h>

class change_ty; // forward
class project_ty; // forward
class string_ty; // forward

/**
  * The change_id class is used to represent all the diffent comment
  * line options used to specify a change set.
  */
class change_id
{
public:
    /**
      * The destructor.
      * It isn't virtual; thou shalt not derive from this class.
      */
    ~change_id();

    /**
      * The default constructor.
      */
    change_id();

    /**
      * The set method is used to determine if this chaneg ID has been
      * set yet (via any of several command line options).
      */
    bool set() const;

    /**
      * The command_line_parse method is used to parse command line
      * options (via the arglex() fucntion) to set the change ID.  The
      * current token is expected to be meaningful for identifying a
      * change.  The position will be advanced past all relevant tokens.
      */
    void command_line_parse(void (*usage)(void));

    /**
      * The command_line_check method is used to verify that sensable
      * command line options have been specified, once the parse has
      * completed.
      */
    void command_line_check(void (*usage)(void));

    /**
      * The set_baseline method is used to specify that the baseline is
      * the change being identified.
      */
    void set_baseline() { baseline = true; }

    /**
      * The set_project method is used <i>after</i> the
      * command_line_check method has been called.  It is used to bind
      * to the appropriate project for the change identified.
      */
    void set_project();

    /**
      * The set_change method is used <i>after</i> the set_project
      * method has been called.  It is used to bind to the appropriate
      * change.
      */
    void set_change(void (*usage)(void));

    /**
      * The get_file_version is used to determine the path to the gioven
      * file at the time specified by the change ID.  It must be called
      * <i>after</i> the set_change method has been called.
      */
    file_version get_file_version(const nstring &filename);

    /**
      * The set_project_name method is used to set the name of the
      * project to be accessed.
      */
    void set_project_name(string_ty *);

private:
    enum { NO_TIME_SET = -1 };

    /**
      * The baseline instance variable is used to remember wether or not
      * the --baseline command lineoption has been specified.
      */
    bool baseline;

    /**
      * The branch instance variable is used to remember any --branch
      * command line option specified.  This could be implicit.
      */
    const char *branch;

    /**
      * The change_number instance variable is used to remember any
      * --change command line option specified.  This could be implicit.
      */
    long change_number;

    /**
      * The delta_date instance variable is used to remember any
      * --delta-date command line option specified.  This could be
      * implicit.
      */
    long delta_date;

    /**
      * The delta_name instance variable is used to remember any
      * --delta-name command line option specified.
      */
    nstring delta_name;

    /**
      * The delta_number instance variable is used to remember any
      * --delta command line option specified.  This could be implicit.
      */
    long delta_number;

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
      * The cp instance variable is used to remember which change we are
      * bound to.
      */
    change_ty *cp;

    /**
      * The copy constructor.  Do not use.
      */
    change_id(const change_id &);

    /**
      * The assignment operator.  Do not use.
      */
    change_id &operator=(const change_id &);
};

#endif // AEDIFF_CHANGE_ID_H

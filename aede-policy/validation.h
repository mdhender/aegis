//
//	aegis - project change supervisor
//	Copyright (C) 2005-2008 Peter Miller
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

#ifndef AEDE_POLICY_VALIDATION_H
#define AEDE_POLICY_VALIDATION_H

#include <libaegis/change.h>

struct fstate_src_ty; // forward
struct validation_list; // forward

/**
  * The validation class is used to represent an abstract validation to
  * be performed on a change set.
  */
class validation
{
public:
    /**
      * The destructor.
      */
    virtual ~validation();

    /**
      * The run method is used to run the validation.  If a problem is
      * found, it will be printed on stderr via the change_error function.
      *
      * @param cp
      *     The change to perform the validation upon.
      * @returns
      *     bool; true if no errors found, false if a problem was found.
      */
    virtual bool run(change::pointer cp) = 0;

    /**
      * The factory class method is used to produce new instances of
      * validations by name.
      *
      * @param name
      *     The name of the validation to create.  (If no such
      *     validation exists, a fatal error message will be issued, and
      *     this method will not return.)
      * @returns
      *     A dynamically allocated validation instance.  Use the delete
      *     operator when you are done with it.
      */
    static validation *factory(const char *name);

    /**
      * The list method is used to list the available validations.
      */
    static void list(void);

    /**
      * The all method is used to construct a list with all of the known
      * validations.
      *
      * @param where
      *     When to put the validations once they have been created.
      */
    static void all(validation_list &where);

protected:
    /**
      * The default constructor.
      */
    validation();

    /**
      * The was_downloaded class method is used to determine whether or
      * not a change was downloaded via aedist(1).  This is determined
      * by looking for the "Download" comment inserted by aedist.
      *
      * @param cp
      *     The change in question.
      * @returns
      *     bool; true is was downloaded, false if not.
      */
    static bool was_downloaded(change::pointer cp);

private:
    /**
      * The copy constructor.  Do not use.
      */
    validation(const validation &);

    /**
      * The assignment operator.  Do not use.
      */
    validation &operator=(const validation &);
};

#endif // AEDE_POLICY_VALIDATION_H

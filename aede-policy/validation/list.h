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

#ifndef AEDE_POLICY_VALIDATION_LIST_H
#define AEDE_POLICY_VALIDATION_LIST_H

#include <common/ac/stddef.h>

#include <aede-policy/validation.h>

/**
  * The validation_list class is used to represent and ordered list if
  * validations to be performed on a change set.
  */
class validation_list
{
public:
    /**
      * The destructor.
      */
    virtual ~validation_list();

    /**
      * The default constructor.
      */
    validation_list();

    /**
      * The push_back method is used to append another validation to the
      * end of the list.
      */
    void push_back(validation *vp);

    /**
      * The run method is used to run each of the validations in the
      * list.
      *
      * @param cp
      *     The change to be validated.
      * @returns
      *     bool; true if all went well, false if a problems was found
      *     (it has been printed already).
      */
    bool run(change::pointer cp) const;

    /**
      * The empty method is used to determine whether or not the list
      * has no members (is empty).
      */
    bool empty() const { return (length == 0); }

private:
    size_t length;
    size_t maximum;
    validation **list;

    /**
      * The clear method is used to delete all of the list members.
      */
    void clear();

    /**
      * The copy constructor.  Do not use.
      */
    validation_list(const validation_list &);

    /**
      * The assignment operator.  Do not use.
      */
    validation_list &operator=(const validation_list &);
};

#endif // AEDE_POLICY_VALIDATION_LIST_H

//
// aegis - project change supervisor
// Copyright (C) 2005-2008, 2010, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#ifndef AEDE_POLICY_VALIDATION_ALL_H
#define AEDE_POLICY_VALIDATION_ALL_H

#include <aede-policy/validation.h>
#include <aede-policy/validation/list.h>

/**
  * The validation_all class is used to represent a validation which
  * performs all of the known validations.
  */
class validation_all:
    public validation
{
public:
    /**
      * The destructor.
      */
    virtual ~validation_all();

private:
    /**
      * The default constructor.
      */
    validation_all();

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      */
    static pointer create(void);

protected:
    // See base class for documentation.
    bool run(change::pointer cp);

private:
    validation_list collection;

    /**
      * The copy constructor.  Do not use.
      */
    validation_all(const validation_all &);

    /**
      * The assignment operator.  Do not use.
      */
    validation_all &operator=(const validation_all &);
};

#endif // AEDE_POLICY_VALIDATION_ALL_H
// vim: set ts=8 sw=4 et :

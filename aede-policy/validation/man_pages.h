//
// aegis - project change supervisor
// Copyright (C) 2012 Peter Miller
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
// You should have received a copy of the GNU General Public License along
// with this program. If not, see <http://www.gnu.org/licenses/>.
//

#ifndef AEDE_POLICY_VALIDATION_MAN_PAGES_H
#define AEDE_POLICY_VALIDATION_MAN_PAGES_H

#include <aede-policy/validation.h>

/**
  * The validation_man_pages class is used to represent the processing
  * required to confirm that all installed programs have man pages.
  */
class validation_man_pages:
    public validation
{
public:
    /**
      * The destructor.
      */
    virtual ~validation_man_pages();

    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      */
    static pointer create(void);

protected:
    // See base class for documentation.
    bool run(change::pointer cp);

private:
    /**
      * The default constructor.
      * It is private on purpose, use the #create class method instead.
      */
    validation_man_pages();

    /**
      * The copy constructor.  Do not use.
      */
    validation_man_pages(const validation_man_pages &);

    /**
      * The assignment operator.  Do not use.
      */
    validation_man_pages &operator=(const validation_man_pages &);
};

// vim: set ts=8 sw=4 et :
#endif // AEDE_POLICY_VALIDATION_MAN_PAGES_H

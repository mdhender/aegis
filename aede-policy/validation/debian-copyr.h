//
// aegis - project change supervisor
// Copyright (C) 2009, 2010 Peter Miller
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

#ifndef AEDE_POLICY_VALIDATION_DEBIAN_COPYR_H
#define AEDE_POLICY_VALIDATION_DEBIAN_COPYR_H

#include <aede-policy/validation.h>


/**
  * The validation_debian_copyright class is used to represent the tests
  * needed to establish that the debian/copyright file is up-to-date.
  */
class validation_debian_copyright:
    public validation
{
public:
    /**
      * The destructor.
      */
    virtual ~validation_debian_copyright();

private:
    /**
      * The default constructor.
      */
    validation_debian_copyright();

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
    /**
      * The copy constructor.  Do not use.
      */
    validation_debian_copyright(const validation_debian_copyright &);

    /**
      * The assignment operator.  Do not use.
      */
    validation_debian_copyright &operator=(const validation_debian_copyright &);
};

#endif // AEDE_POLICY_VALIDATION_DEBIAN_COPYR_H

//
// aegis - project change supervisor
// Copyright (C) 2008, 2010 Peter Miller
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

#ifndef AEDE_POLICY_VALIDATION_AUTHORS_H
#define AEDE_POLICY_VALIDATION_AUTHORS_H

#include <aede-policy/validation.h>

/**
  * The validation_authors class is used to represent the tests needed
  * to establish that the AUTHORS file is up-to-date.
  */
class validation_authors:
    public validation
{
public:
    /**
      * The destructor.
      */
    virtual ~validation_authors();

private:
    /**
      * The default constructor.
      */
    validation_authors();

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
    validation_authors(const validation_authors &);

    /**
      * The assignment operator.  Do not use.
      */
    validation_authors &operator=(const validation_authors &);
};

#endif // AEDE_POLICY_VALIDATION_AUTHORS_H

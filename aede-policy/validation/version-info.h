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

#ifndef AEDE_POLICY_VALIDATION_VERSION_INFO_H
#define AEDE_POLICY_VALIDATION_VERSION_INFO_H

#include <aede-policy/validation.h>

/**
  * The validation_versin_info class is used to represent a check of a
  * shared library against the ancesttor version, to make sure that the
  * version-info of the shared library conforms to the policy laid out
  * by the libtool(1) command, and required by the Debian Policy Manual.
  */
class validation_version_info:
    public validation
{
public:
    /**
      * The destructor.
      */
    virtual ~validation_version_info();

private:
    /**
      * The default constructor.
      * It is private on purpose, use the #create class method instead.
      */
    validation_version_info();

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
    validation_version_info(const validation_version_info &);

    /**
      * The assignment operator.  Do not use.
      */
    validation_version_info &operator=(const validation_version_info &);
};

#endif // AEDE_POLICY_VALIDATION_VERSION_INFO_H

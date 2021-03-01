//
// aegis - project change supervisor
// Copyright (C) 2004-2008, 2012 Peter Miller
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

#ifndef FMTGEN_TYPE_BOOLEAN_H
#define FMTGEN_TYPE_BOOLEAN_H

#include <fmtgen/type.h>

/**
  * The type_boolean class is used to represent the type of a field
  * which is a boolean value.
  */
class type_boolean:
    public type
{
public:
    /**
      * The destructor.
      */
    virtual ~type_boolean();

protected:
    /**
      * The default constructor.
      * It is protected on purpose, only derived classes may call this.
      *
      * @param gen
      *     where to write the generated code
      */
    type_boolean(generator *gen);

protected:
    // See base class for documentation.
    nstring c_name_inner() const;

    // See base class for documentation.
    bool has_a_mask() const;

private:
    /**
      * The default constructor.  Do not use.
      */
    type_boolean();

    /**
      * The copy constructor.  Do not use.
      */
    type_boolean(const type_boolean &);

    /**
      * The assignment operator.  Do not use.
      */
    type_boolean &operator=(const type_boolean &);
};

#endif // FMTGEN_TYPE_BOOLEAN_H
// vim: set ts=8 sw=4 et :

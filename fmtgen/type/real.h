//
// aegis - project change supervisor
// Copyright (C) 1998, 2005-2008, 2012 Peter Miller
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

#ifndef FMTGEN_TYPE_REAL_H
#define FMTGEN_TYPE_REAL_H

#include <fmtgen/type.h>

/**
  * The type_real class is used to represent the type of a real/
  * double valued field.
  */
class type_real:
    public type
{
public:
    /**
      * The destructor.
      */
    virtual ~type_real();

protected:
    /**
      * The constructor.
      * It is protected on purpose, only derived classes may call it.
      *
      * @param gen
      *     where to write the generated code
      */
    type_real(generator *gen);

protected:
    // See base class for documentation.
    nstring c_name_inner() const;

    // See base class for documentation.
    bool has_a_mask() const;

private:
    /**
      * The default constructor.  Do not use.
      */
    type_real();

    /**
      * The copy constructor.  Do not use.
      */
    type_real(const type_real &);

    /**
      * The assignment operator.  Do not use.
      */
    type_real &operator=(const type_real &);
};

#endif // FMTGEN_TYPE_REAL_H
// vim: set ts=8 sw=4 et :

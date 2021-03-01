//
// aegis - project change supervisor
// Copyright (C) 2008 Peter Miller
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

#ifndef FMTGEN_TYPE_TOP_LEVEL_H
#define FMTGEN_TYPE_TOP_LEVEL_H

#include <fmtgen/type.h>

/**
  * The type_top_level class is used to represent
  */
class type_top_level:
    public type
{
public:
    /**
      * The destructor.
      */
    virtual ~type_top_level();

protected:
    /**
      * The constructor.
      * It is protected on purpose, only derived classes may call it.
      */
    type_top_level(generator *gen, const pointer &subtype);

protected:
    // See base class for documentation.
    bool has_a_mask() const;

    // See base class for documentation.
    nstring c_name_inner() const;

    // See base class for documentation.
    void get_reachable(type_vector &results) const;

protected:
    pointer subtype;

private:
    /**
      * The default constructor.  Do not use.
      */
    type_top_level();

    /**
      * The copy constructor.  Do not use.
      */
    type_top_level(const type_top_level &);

    /**
      * The assignment operator.  Do not use.
      */
    type_top_level &operator=(const type_top_level &);
};

#endif // FMTGEN_TYPE_TOP_LEVEL_H

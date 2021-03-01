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

#ifndef FMTGEN_TYPE_TOP_LEVEL_CODE_H
#define FMTGEN_TYPE_TOP_LEVEL_CODE_H

#include <fmtgen/type/top_level.h>

/**
  * The type_top_level_code class is used to represent the code
  * generation necessary for the top-level structure reading and
  * writing.
  */
class type_top_level_code:
    public type_top_level
{
public:
    /**
      * The destructor.
      */
    virtual ~type_top_level_code();

private:
    /**
      * The default constructor.
      * It is private on purpose, use the #create class method instead.
      *
      * @param gen
      *     where to send the generated code
      * @param subtype
      *     the underlying structure type
      */
    type_top_level_code(generator *gen, const pointer &subtype);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      */
    static pointer create(generator *gen, const pointer &subtype);

protected:
    // See base class for documentation.
    void gen_body() const;

private:
    /**
      * The default constructor.  Do not use.
      */
    type_top_level_code();

    /**
      * The copy constructor.  Do not use.
      */
    type_top_level_code(const type_top_level_code &);

    /**
      * The assignment operator.  Do not use.
      */
    type_top_level_code &operator=(const type_top_level_code &);
};

#endif // FMTGEN_TYPE_TOP_LEVEL_CODE_H

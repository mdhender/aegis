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

#ifndef FMTGEN_TYPE_TOP_LEVEL_INCLUDE_H
#define FMTGEN_TYPE_TOP_LEVEL_INCLUDE_H

#include <fmtgen/type/top_level.h>

/**
  * The type_top_level_include class is used to represent the include
  * file generation for the top-level structure read and write, in the
  * original (circa 1991) C style.
  */
class type_top_level_include:
    public type_top_level
{
public:
    /**
      * The destructor.
      */
    virtual ~type_top_level_include();

private:
    /**
      * The constructor.
      * It is private on purpose, use the #create class method instead.
      *
      * @param gen
      *     where to send the generated code
      * @param subtype
      *     the underlying structure type
      */
    type_top_level_include(generator *gen, const pointer &subtype);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param gen
      *     where to send the generated code
      * @param subtype
      *     the underlying structure type
      */
    static pointer create(generator *gen, const pointer &subtype);

protected:
    // See base class for documentation.
    void gen_body() const;

private:
    /**
      * The default constructor.  Do not use.
      */
    type_top_level_include();

    /**
      * The copy constructor.  Do not use.
      */
    type_top_level_include(const type_top_level_include &);

    /**
      * The assignment operator.  Do not use.
      */
    type_top_level_include &operator=(const type_top_level_include &);
};

#endif // FMTGEN_TYPE_TOP_LEVEL_INCLUDE_H

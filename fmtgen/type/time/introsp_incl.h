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

#ifndef FMTGEN_TYPE_TIME_INTROSP_INCL_H
#define FMTGEN_TYPE_TIME_INTROSP_INCL_H

#include <fmtgen/type/time.h>

/**
  * The type_time_introspector_include class is used to represent the
  * code generation necessary to produce the include file portions
  * corresponding to time (time_t) fields, in the C++ OO introspector
  * style.
  */
class type_time_introspector_include:
    public type_time
{
public:
    /**
      * The destructor.
      */
    virtual ~type_time_introspector_include();

private:
    /**
      * The constructor.
      * It is private on purpose, use the #create class method instead.
      *
      * @param gen
      *     where to write the generated code
      */
    type_time_introspector_include(generator *gen);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param gen
      *     where to write the generated code
      */
    static pointer create(generator *gen);

protected:
    // See base class for documentation.
    void gen_declarator(const nstring &variable_name, bool is_a_list,
        int attributes, const nstring &comment) const;

    // See base class for documentation.
    void gen_body() const;

private:
    /**
      * The default constructor.  Do not use.
      */
    type_time_introspector_include();

    /**
      * The copy constructor.  Do not use.
      */
    type_time_introspector_include(const type_time_introspector_include &);

    /**
      * The assignment operator.  Do not use.
      */
    type_time_introspector_include &operator=(
        const type_time_introspector_include &);
};

#endif // FMTGEN_TYPE_TIME_INTROSP_INCL_H

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

#ifndef FMTGEN_TYPE_ENUMERATION_INCLUDE_H
#define FMTGEN_TYPE_ENUMERATION_INCLUDE_H

#include <fmtgen/type/enumeration.h>

/**
  * The type_enumeration_include class is used to represent the code
  * generation for manipulating meta-data enumerations, in the orginal C
  * (circa 1991) style.
  */
class type_enumeration_include:
    public type_enumeration
{
public:
    /**
      * The destructor.
      */
    virtual ~type_enumeration_include();

private:
    /**
      * The constructor.
      * It is private on purpose, use the #create class method instead.
      *
      * @param gen
      *     where to write the generated code
      * @param name
      *     The name of the enumerated type.
      * @param global
      *     true if the name is a global name, false if it is local to
      *     the containing .def file.
      */
    type_enumeration_include(generator *gen, const nstring &name, bool global);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param gen
      *     where to write the generated code
      * @param name
      *     The name of the enumerated type.
      * @param global
      *     true if the name is a global name, false if it is local to
      *     the containing .def file.
      */
    static pointer create(generator *gen, const nstring &name, bool global);

protected:
    // See base class for documentation.
    void gen_body() const;

    // See base class for documentation.
    void gen_declarator(const nstring &variable_name, bool is_a_list,
        int attributes, const nstring &comment) const;

private:
    /**
      * The default constructor.   Do not use.
      */
    type_enumeration_include();

    /**
      * The copy constructor.   Do not use.
      */
    type_enumeration_include(const type_enumeration_include &);

    /**
      * The assignment operator.   Do not use.
      */
    type_enumeration_include &operator=(const type_enumeration_include &);
};

#endif // FMTGEN_TYPE_ENUMERATION_INCLUDE_H

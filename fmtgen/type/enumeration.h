//
// aegis - project change supervisor
// Copyright (C) 1994, 2005-2008, 2012 Peter Miller
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

#ifndef FMTGEN_TYPE_ENUMERATION_H
#define FMTGEN_TYPE_ENUMERATION_H

#include <fmtgen/type.h>
#include <common/nstring/list.h>

/**
  * The type_enumeration class is used to represent the type of a
  * field with an enumerated value.
  */
class type_enumeration:
    public type
{
public:
    /**
      * The destructor.
      */
    virtual ~type_enumeration();

protected:
    /**
      * The constructor.
      * It is protected on purpose, only derived classes may call it.
      *
      * @param gen
      *     where to write the generated code
      * @param name
      *     The name of the enumerated type.
      * @param global
      *     true if the name is a global name, false if it is local to
      *     the containing .def file.
      */
    type_enumeration(generator *gen, const nstring &name, bool global);

protected:
    // See base class for documentation.
    void member_add(const nstring &member_name,
        const type::pointer &member_type, int attributes,
        const nstring &comment);

    // See base class for documentation.
    nstring c_name_inner() const;

    // See base class for documentation.
    bool has_a_mask() const;

protected:
    // FIXME: make this private
    nstring_list elements;

private:
    /**
      * The default constructor.   Do not use.
      */
    type_enumeration();

    /**
      * The copy constructor.   Do not use.
      */
    type_enumeration(const type_enumeration &);

    /**
      * The assignment operator.   Do not use.
      */
    type_enumeration &operator=(const type_enumeration &);
};

#endif // FMTGEN_TYPE_ENUMERATION_H
// vim: set ts=8 sw=4 et :

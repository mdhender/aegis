//
// aegis - project change supervisor
// Copyright (C) 1994, 2005-2008, 2012 Peter Miller.
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

#ifndef FMTGEN_TYPE_LIST_H
#define FMTGEN_TYPE_LIST_H

#include <fmtgen/type.h>

/**
  * The type_list class is used to represent a type which is a list
  * of come other type.
  */
class type_list:
    public type
{
public:
    /**
      * The destructor.
      */
    virtual ~type_list();

protected:
    /**
      * The constructor.
      * It is protected on purpose, only derived classes may call it.
      *
      * @param gen
      *     where to write the generated code
      * @param a_name
      *      The name of the list type.
      * @param a_global
      *     true if the name is a global name, false if it is local to
      *     the containing .def file.
      * @param a_type
      *      The type of things in the list.
      */
    type_list(generator *gen, const nstring &a_name, bool a_global,
        const type::pointer &a_type);

protected:
    // See base class for documentation.
    nstring c_name_inner() const;

    // See base class for documentation.
    bool has_a_mask() const;

    // See base class for documentation.
    void in_include_file();

    // See base class for documentation.
    void get_reachable(type_vector &results) const;

protected:
    /**
      * The subtype instance variable is used to remember the type of the
      * list members.
      */
    type::pointer subtype;

private:
    /**
      * The default constructor.  Do not use.
      */
    type_list();

    /**
      * The copy constructor.  Do not use.
      */
    type_list(const type_list &);

    /**
      * The assignment operator.  Do not use.
      */
    type_list &operator=(const type_list &);
};

#endif // FMTGEN_TYPE_LIST_H
// vim: set ts=8 sw=4 et :

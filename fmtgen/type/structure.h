//
// aegis - project change supervisor
// Copyright (C) 1994, 2002, 2005-2008, 2012 Peter Miller.
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

#ifndef FMTGEN_TYPE_STRUCTURE_H
#define FMTGEN_TYPE_STRUCTURE_H

#include <fmtgen/type.h>

/**
  * The type_structure class is used to represent the type of a field
  * which is a compound type, made up of several fields.
  */
class type_structure:
    public type
{
public:
    /**
      * The destructor.
      */
    virtual ~type_structure();

protected:
    /**
      * The constructor.
      * It is protected on purpose, only derived classes may call it.
      *
      * @param gen
      *     where to write the generated code
      * @param name
      *     The name of this structure type.
      * @param global
      *     true if the name is a global name, false if it is local to
      *     the containing .def file.
      */
    type_structure(generator *gen, const nstring &name, bool global);

protected:
    // See base class for documentation.
    void member_add(const nstring &member_name,
        const type::pointer &member_type, int attributes,
        const nstring &comment);

    // See base class for documentation.
    nstring c_name_inner() const;

    // See base class for documentation.
    bool has_a_mask() const;

    // See base class for documentation.
    void in_include_file();

    // See base class for documentation.
    void toplevel();

    // See base class for documentation.
    void get_reachable(type_vector &results) const;

protected:
    // but should be private
    struct element_ty
    {
        element_ty() : attributes(0) { }
        nstring name;
        type::pointer etype;
        int attributes;
        nstring comment;
    };

    size_t nelements;
    size_t nelements_max;
    element_ty *element;
    bool toplevel_flag;

private:
    /**
      * The default constructor.  Do not use.
      */
    type_structure();

    /**
      * The copy constructor.  Do not use.
      */
    type_structure(const type_structure &);

    /**
      * The assignment operator.  Do not use.
      */
    type_structure &operator=(const type_structure &);
};

#endif // FMTGEN_TYPE_STRUCTURE_H
// vim: set ts=8 sw=4 et :

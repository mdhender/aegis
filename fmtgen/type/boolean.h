//
//	aegis - project change supervisor
//	Copyright (C) 2004 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: interface of the type_boolean class
//

#ifndef FMTGEN_TYPE_BOOLEAN_H
#define FMTGEN_TYPE_BOOLEAN_H

#pragma interface "type_boolean"

#include <type.h>

/**
  * The type_boolean class is used to represent the type of a field
  * which is a boolean value.
  */
class type_boolean:
    public type_ty
{
public:
    /**
      * The destructor.
      */
    virtual ~type_boolean();

    /**
      * The default constructor.
      */
    type_boolean();

    // See base class for documentation.
    void gen_include_declarator(const nstring &variable_name, bool is_a_list)
	const;

    // See base class for documentation.
    void gen_code_declarator(const nstring &variable_name, bool is_a_list,
	int attributes) const;

    // See base class for documentation.
    void gen_code_call_xml(const nstring &form_name, const nstring &member_name,
	int attributes) const;

    // See base class for documentation.
    void gen_free_declarator(const nstring &variable_name, bool is_a_list)
	const;

    // See base class for documentation.
    nstring c_name_inner() const;

    // See base class for documentation.
    bool has_a_mask() const;

private:
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
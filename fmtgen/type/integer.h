//
//	aegis - project change supervisor
//	Copyright (C) 1994, 2005-2007 Peter Miller.
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
// MANIFEST: interface definition for fmtgen/type/integer.c
//

#ifndef FMTGEN_TYPE_INTEGER_H
#define FMTGEN_TYPE_INTEGER_H

#include <fmtgen/type.h>

/**
  * The type_integer class is used to represent the type of a field
  * which is an integer.
  */
class type_integer:
    public type
{
public:
    /**
      * The destructor.
      */
    virtual ~type_integer();

    /**
      * The default constructor.
      */
    type_integer();

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
    void gen_code_copy(const nstring &member_name) const;

    // See base class for documentation.
    void gen_code_trace(const nstring &name, const nstring &value) const;

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
    type_integer(const type_integer &);

    /**
      * The assignment operator.  Do not use.
      */
    type_integer &operator=(const type_integer &);
};

#endif // FMTGEN_TYPE_INTEGER_H

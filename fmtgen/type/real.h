//
//	aegis - project change supervisor
//	Copyright (C) 1998 Peter Miller;
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
// MANIFEST: interface definition for fmtgen/type/real.c
//

#ifndef FMTGEN_TYPE_REAL_H
#define FMTGEN_TYPE_REAL_H

#pragma interface "type_real_ty"

#include <type.h>

/**
  * The type_real_ty class is used to represent the type of a real/
  * double valued field.
  */
class type_real_ty:
    public type_ty
{
public:
    /**
      * The destructor.
      */
    virtual ~type_real_ty();

    /**
      * the default constructor.
      */
    type_real_ty();

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
    type_real_ty(const type_real_ty &);

    /**
      * The assignment operator.  Do not use.
      */
    type_real_ty &operator=(const type_real_ty &);
};

#endif // FMTGEN_TYPE_REAL_H

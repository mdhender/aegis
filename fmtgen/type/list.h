//
//	aegis - project change supervisor
//	Copyright (C) 1994, 2005 Peter Miller.
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
// MANIFEST: interface definition for fmtgen/type/list.c
//

#ifndef FMTGEN_TYPE_LIST_H
#define FMTGEN_TYPE_LIST_H

#include <fmtgen/type.h>

/**
  * The type_list_ty class is used to represent a type which is a list
  * of come other type.
  */
class type_list_ty:
    public type_ty
{
public:
    /**
      * The destructor.
      */
    virtual ~type_list_ty();

    /**
      * The constructor.
      */
    type_list_ty(const nstring &a_name, type_ty *a_type);

    // See base class for documentation.
    void gen_include() const;

    // See base class for documentation.
    void gen_include_declarator(const nstring &variable_name, bool is_a_list)
	const;

    // See base class for documentation.
    void gen_code() const;

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

    // See base class for documentation.
    void in_include_file();

private:
    /**
      * The subtype instance variable is used to remember the type of the
      * list members.
      */
    type_ty *subtype;

    /**
      * The default constructor.  Do not use.
      */
    type_list_ty();

    /**
      * The copy constructor.  Do not use.
      */
    type_list_ty(const type_list_ty &);

    /**
      * The assignment operator.  Do not use.
      */
    type_list_ty &operator=(const type_list_ty &);
};

#endif // FMTGEN_TYPE_LIST_H

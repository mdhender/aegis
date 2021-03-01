//
//	aegis - project change supervisor
//	Copyright (C) 1994, 2002 Peter Miller.
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
// MANIFEST: interface definition for fmtgen/type/structure.c
//

#ifndef FMTGEN_TYPE_STRUCTURE_H
#define FMTGEN_TYPE_STRUCTURE_H

#pragma interface "type_structure_ty"

#include <type.h>

/**
  * The type_structure_ty class is used to represent the type of a field
  * which is a compound type, made up of several fields.
  */
class type_structure_ty:
    public type_ty
{
public:
    /**
      * The destructor.
      */
    virtual ~type_structure_ty();

    /**
      * The constructor.
      */
    type_structure_ty(const nstring &arg);

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
    void gen_free_declarator(const nstring &variable_name, bool is_a_list)
	const;

    // See base class for documentation.
    void member_add(const nstring &member_name, type_ty *member_type,
	int attributes);

    // See base class for documentation.
    nstring c_name_inner() const;

    // See base class for documentation.
    bool has_a_mask() const;

    // See base class for documentation.
    void in_include_file();

    // See base class for documentation.
    void toplevel();

private:
    struct element_ty
    {
	element_ty() : type(0), attributes(0) { }
	nstring name;
        type_ty *type;
	int attributes;
    };

    size_t nelements;
    size_t nelements_max;
    element_ty *element;
    bool toplevel_flag;

    /**
      * The default constructor.  Do not use.
      */
    type_structure_ty();

    /**
      * The copy constructor.  Do not use.
      */
    type_structure_ty(const type_structure_ty &);

    /**
      * The assignment operator.  Do not use.
      */
    type_structure_ty &operator=(const type_structure_ty &);
};

#endif // FMTGEN_TYPE_STRUCTURE_H

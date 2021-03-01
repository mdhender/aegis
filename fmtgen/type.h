//
//	aegis - project change supervisor
//	Copyright (C) 1991-1994, 1998, 2002-2005 Peter Miller;
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
// MANIFEST: interface definition for fmtgen/type.c
//

#ifndef TYPE_H
#define TYPE_H

#include <fmtgen/lex.h>
#include <common/nstring.h>

#define ATTRIBUTE_REDEFINITION_OK 1
#define ATTRIBUTE_SHOW_IF_DEFAULT 2
#define ATTRIBUTE_HIDE_IF_DEFAULT 4

/**
  * The class type is used to represent the type of a field.  This is an
  * abstract base class.
  */
class type_ty
{
public:
    /**
      * The destructor.
      */
    virtual ~type_ty();

protected:
    /**
      * The constructor.
      */
    type_ty(const nstring &name);

public:
    virtual void gen_include() const;
    virtual void gen_include_declarator(const nstring &name, bool is_a_list)
	const;
    virtual void gen_code() const;
    virtual void gen_code_declarator(const nstring &name, bool is_a_list,
	int attributes) const;
    virtual void gen_code_call_xml(const nstring &form_name,
	const nstring &member_name, int show) const;

    /**
      * The gen_code_copy method is used to generate deep-copy code for
      * a specific instance variable.
      */
    virtual void gen_code_copy(const nstring &member_name) const = 0;

    /**
      * The gen_code_trace method is used to generate trace code for
      * a specific instance variable.
      */
    virtual void gen_code_trace(const nstring &name, const nstring &value)
	const = 0;

    virtual void gen_free_declarator(const nstring &name, bool is_a_list) const;
    virtual void member_add(const nstring &member_name, type_ty *member_type,
	int attributes);

    /**
      * The in_include_file method is used to tell the type is in an
      * included file, not the top-level defintion file.
      */
    virtual void in_include_file();

    bool is_in_include_file() const { return included_flag; }

    /**
      * The c_name function is used to get the C name to be used in
      * abstract declarators (and casts) of this type.
      */
    const nstring &
    c_name()
    {
	if (!c_name_cache)
	    c_name_cache = c_name_inner();
	return c_name_cache;
    }

    virtual bool has_a_mask() const = 0;

    const nstring &def_name() const { return name; }

    virtual void toplevel();

    void typedef_set() { is_a_typedef = true; }

private:
    /**
      * The c_name_inner method is used to get the C name to be used in
      * abstract declarators (and casts) of this type.  Usually the names
      * match (with _ty on the end), but integer->int, real->double,
      * time->time_t, etc, are exceptions.
      */
    virtual nstring c_name_inner() const = 0;

    /**
      * The name instance variable is used to remember the name of the
      * type in the defintion file.
      */
    nstring name;

    /**
      * The c_name_cache instance variable is used to remember the name of the
      * type in the generated code.
      */
    nstring c_name_cache;

    /**
      * The is_a_typedef instance is used to remember whether the type
      * is a typedef (type definition) or not.
      */
    bool is_a_typedef;

    /**
      * The included_flag instance variable is used to remember whether
      * or not the symbol was defined in a deeper include file.
      */
    bool included_flag;

    /**
      * The default constructor.  Do not use.
      */
    type_ty();

    /**
      * The copy constructor.  Do not use.
      */
    type_ty(const type_ty &);

    /**
      * The assignment operator.  Do not use.
      */
    type_ty &operator=(const type_ty &);
};

#endif // TYPE_H

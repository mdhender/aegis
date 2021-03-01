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

#ifndef FMTGEN_TYPE_BOOLEAN_INTROSP_CODE_H
#define FMTGEN_TYPE_BOOLEAN_INTROSP_CODE_H

#include <fmtgen/type/boolean.h>

/**
  * The type_boolean_introspector_code class is used to represent the
  * type of a field which is a boolean value, in the C++ OO introspector
  * style.
  */
class type_boolean_introspector_code:
    public type_boolean
{
public:
    /**
      * The destructor.
      */
    virtual ~type_boolean_introspector_code();

private:
    /**
      * The default constructor.
      * It is private on purpose, use the #create class method instead.
      *
      * @param gen
      *     where to write the generated code
      */
    type_boolean_introspector_code(generator *gen);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param gen
      *     where to write the generated code
      */
    static pointer create(generator *gen);

protected:
    // See base class for documentation.
    void gen_declarator(const nstring &variable_name, bool is_a_list,
        int attributes, const nstring &comment) const;

    // See base class for documentation.
    void gen_call_xml(const nstring &form_name, const nstring &member_name,
        int attributes) const;

    // See base class for documentation.
    void gen_default_constructor(const nstring &inst_var_name);

    // See base class for documentation.
    void gen_copy_constructor(const nstring &inst_var_name);

    // See base class for documentation.
    void gen_assignment_operator(const nstring &inst_var_name);

    // See base class for documentation.
    void gen_write(const nstring&, const nstring &member_name,
        bool show_default) const;

    // See base class for documentation.
    void gen_copy(const nstring &member_name) const;

    // See base class for documentation.
    void gen_trace(const nstring &name, const nstring &value) const;

    // See base class for documentation.
    void gen_free_declarator(const nstring &variable_name, bool is_a_list)
        const;

    // See base class for documentation.
    void gen_methods(const nstring &class_name, const nstring &inst_var_name,
        int attributes) const;

private:
    /**
      * The default constructor.  Do not use.
      */
    type_boolean_introspector_code();

    /**
      * The copy constructor.  Do not use.
      */
    type_boolean_introspector_code(const type_boolean_introspector_code &);

    /**
      * The assignment operator.  Do not use.
      */
    type_boolean_introspector_code &operator=(
        const type_boolean_introspector_code &);
};

#endif // FMTGEN_TYPE_BOOLEAN_INTROSP_CODE_H

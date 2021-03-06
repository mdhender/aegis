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

#ifndef FMTGEN_TYPE_STRING_CODE_H
#define FMTGEN_TYPE_STRING_CODE_H

#include <fmtgen/type/string.h>

/**
  * The type_string_code class is used to represent the code generation
  * needed to manage strings in the meta-data, in the C style (circa
  * 1991), for the code files.
  */
class type_string_code:
    public type_string
{
public:
    /**
      * The destructor.
      */
    virtual ~type_string_code();

private:
    /**
      * The constructor.
      * It is private on purpose, use the #create class method instead.
      *
      * @param gen
      *     where to write the generated code
      */
    type_string_code(generator *gen);

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
    void gen_body() const;

    // See base class for documentation.
    void gen_declarator(const nstring &variable_name, bool is_a_list,
        int attributes, const nstring &comment) const;

    // See base class for documentation.
    void gen_call_xml(const nstring &form_name, const nstring &member_name,
        int attributes) const;

    // See base class for documentation.
    void gen_copy(const nstring &member_name) const;

    // See base class for documentation.
    void gen_trace(const nstring &name, const nstring &value) const;

    // See base class for documentation.
    void gen_free_declarator(const nstring &variable_name, bool is_a_list)
        const;

private:
    /**
      * The default constructor.  Do not use.
      */
    type_string_code();

    /**
      * The copy constructor.  Do not use.
      */
    type_string_code(const type_string_code &);

    /**
      * The assignment operator.  Do not use.
      */
    type_string_code &operator=(const type_string_code &);
};

#endif // FMTGEN_TYPE_STRING_CODE_H

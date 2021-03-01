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

#ifndef FMTGEN_TYPE_PAIR_H
#define FMTGEN_TYPE_PAIR_H

#include <fmtgen/type.h>

/**
  * The type_pair class is used to represent a type which is actually a
  * pair of type code generators, each generating different code.
  */
class type_pair:
    public type
{
public:
    /**
      * The destructor.
      */
    virtual ~type_pair();

private:
    /**
      * The constructor.
      * It is private on purpose, use the #create class method instead.
      *
      * @param gen
      *     where to write the generated code
      * @param t1
      *     The first of two types.
      * @param t2
      *     The second of two types.
      */
    type_pair(generator *gen, const pointer &t1, const pointer &t2);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param gen
      *     where to write the generated code
      * @param t1
      *     The first of two types.
      * @param t2
      *     The second of two types.
      */
    static pointer create(generator *gen, const pointer &t1, const pointer &t2);

    pointer get_t1() const { return t1; }
    pointer get_t2() const { return t2; }

protected:
    // See base class for documentation.
    void gen_body() const;

    // See base class for documentation.
    void gen_declarator(const nstring &name, bool is_a_list, int attributes,
        const nstring &comment) const;

    // See base class for documentation.
    void gen_call_xml(const nstring &form_name, const nstring &member_name,
        int show) const;

    // See base class for documentation.
    void gen_copy(const nstring &member_name) const;

    // See base class for documentation.
    void gen_trace(const nstring &name, const nstring &value) const;

    // See base class for documentation.
    void gen_free_declarator(const nstring &name, bool is_a_list) const;

    // See base class for documentation.
    void gen_report_initializations();

    // See base class for documentation.
    void member_add(const nstring &member_name,
        const type::pointer &member_type, int attributes,
        const nstring &comment);

    // See base class for documentation.
    void toplevel();

    // See base class for documentation.
    bool has_a_mask() const;

    // See base class for documentation.
    nstring c_name_inner() const;

    // See base class for documentation.
    void in_include_file();

    // See base class for documentation.
    void get_reachable(type_vector &results) const;

private:
    /**
      * The t1 instance variable isused to remember the first of two
      * parallel types, typically a .h and .cc pair.
      */
    pointer t1;

    /**
      * The t2 instance variable isused to remember the second of two
      * parallel types, typically a .h and .cc pair.
      */
    pointer t2;

    /**
      * The default constructor.  Do not use.
      */
    type_pair();

    /**
      * The copy constructor.  Do not use.
      */
    type_pair(const type_pair &);

    /**
      * The assignment operator.  Do not use.
      */
    type_pair &operator=(const type_pair &);
};

#endif // FMTGEN_TYPE_PAIR_H

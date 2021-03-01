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

#ifndef FMTGEN_GENERATOR_PAIR_H
#define FMTGEN_GENERATOR_PAIR_H

#include <fmtgen/generator.h>

/**
  * The generator_pair class is used to represent
  */
class generator_pair:
    public generator
{
public:
    /**
      * The destructor.
      */
    virtual ~generator_pair();

private:
    /**
      * The constructor.
      * It is private on purpose, use the #create class method instead.
      *
      * @param g1
      *     The first of two generators to be run in parallel.
      * @param g2
      *     The second of two generators to be run in parallel.
      */
    generator_pair(const pointer &g1, const pointer &g2);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param g1
      *     The first of two generators to be run in parallel.
      * @param g2
      *     The second of two generators to be run in parallel.
      */
    static pointer create(const pointer &g1, const pointer &g2);

protected:
    // See base class for documentation.
    type::pointer type_boolean_factory();

    // See base class for documentation.
    type::pointer type_enum_factory(const nstring &name, bool global);

    // See base class for documentation.
    type::pointer type_integer_factory();

    // See base class for documentation.
    type::pointer type_list_factory(const nstring &name, bool global,
        const type::pointer &subtype);

    // See base class for documentation.
    type::pointer type_real_factory();

    // See base class for documentation.
    type::pointer type_string_factory();

    // See base class for documentation.
    type::pointer type_structure_factory(const nstring &name, bool global);

    type::pointer type_time_factory();

    // See base class for documentation.
    void generate_file();

    // See base class for documentation.
    type::pointer top_level_factory(const type::pointer &subtype);

private:
    /**
      * The g1 instance variable is used to remember the first of two
      * child generators.
      */
    pointer g1;

    /**
      * The g2 instance variable is used to remember the second of two
      * child generators.
      */
    pointer g2;

    /**
      * The default constructor.  Do not use.
      */
    generator_pair();

    /**
      * The copy constructor.  Do not use.
      */
    generator_pair(const generator_pair &);

    /**
      * The assignment operator.  Do not use.
      */
    generator_pair &operator=(const generator_pair &);
};

#endif // FMTGEN_GENERATOR_PAIR_H

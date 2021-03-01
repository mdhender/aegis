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

#ifndef FMTGEN_GENERATOR_ORIG_CODE_H
#define FMTGEN_GENERATOR_ORIG_CODE_H

#include <fmtgen/generator.h>

/**
  * The generator_original_code class is used to represent the code
  * generator in the original (circa 1991) format, which was C with some
  * helpers.  This is the code (.c) half.
  */
class generator_original_code:
    public generator
{
public:
    /**
      * The destructor.
      */
    virtual ~generator_original_code();

private:
    /**
      * The constructor.
      * It is private on purpose, use the #create class method instead.
      *
      * @param filename
      *     The file in which to place the generated code
      */
    generator_original_code(const nstring &filename);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param filename
      *     The file in which to place the generated code
      */
    static pointer create(const nstring &filename);

protected:
    // See base class for documentation.
    void generate_file();

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

    // See base class for documentation.
    type::pointer type_time_factory();

    // See base class for documentation.
    type::pointer top_level_factory(const type::pointer &subtype);

private:
    /**
      * The default constructor.  Do not use.
      */
    generator_original_code();

    /**
      * The copy constructor.  Do not use.
      */
    generator_original_code(const generator_original_code &);

    /**
      * The assignment operator.  Do not use.
      */
    generator_original_code &operator=(const generator_original_code &);
};

#endif // FMTGEN_GENERATOR_ORIG_CODE_H

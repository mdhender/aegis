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

#ifndef FMTGEN_GENERATOR_H
#define FMTGEN_GENERATOR_H

#include <common/ac/stdarg.h>

#include <fmtgen/indent.h>
#include <fmtgen/type.h>
#include <fmtgen/type/vector.h>


/**
  * The generator abstract base class is used to represent a generic
  * code generator driven from fmtgen .def files.
  */
class generator
{
public:
    typedef aegis_shared_ptr<generator> pointer;

    /**
      * The destructor.
      */
    virtual ~generator();

protected:
    /**
      * The constructor.
      * Only derived classes may use this constructor.
      *
      * @param filename
      *     The name of the file to be written.
      */
    generator(const nstring &filename);

public:
    /**
      * The factory class method may be used to create a new generator
      * by name.
      *
      * @param name
      *     The name of the generator to use (arglex token number).
      * @param filename
      *     The name of the file to be written.
      * @returns
      *     pointer to instance
      */
    static pointer factory(int name, const nstring &filename);

    /**
      * The type_boolean_factory method is used to create new instances
      * of boolean type.  We don't go direct, as different generator
      * will use diffent type classes, depending on the code to be
      * generated.
      *
      * @returns
      *     pointer to new type instance
      */
    virtual type::pointer type_boolean_factory() = 0;

    /**
      * The type_enum_factory method is used to create new
      * instances of enumerated types.  We don't go direct, as different
      * generator will use diffent type classes, depending on the code
      * to be generated.
      *
      * @param name
      *     the name of the enum
      * @returns
      *     pointer to new type instance
      */
    virtual type::pointer type_enum_factory(const nstring &name,
        bool global) = 0;

    /**
      * The type_integer_factory method is used to create new instances
      * of integer type.  We don't go direct, as different generator
      * will use diffent type classes, depending on the code to be
      * generated.
      *
      * @returns
      *     pointer to new type instance
      */
    virtual type::pointer type_integer_factory() = 0;

    /**
      * The type_list_factory method is used to create new
      * instances of list types.  We don't go direct, as different
      * generator will use diffent type classes, depending on the code
      * to be generated.
      *
      * @param name
      *     the name of the list type
      * @param subtype
      *     the type of the list members
      * @returns
      *     pointer to new type instance
      */
    virtual type::pointer type_list_factory(const nstring &name, bool global,
        const type::pointer &subtype) = 0;

    /**
      * The type_real_factory method is used to create new instances of
      * real type.  We don't go direct, as different generator will use
      * diffent type classes, depending on the code to be generated.
      *
      * @returns
      *     pointer to new type instance
      */
    virtual type::pointer type_real_factory() = 0;

    /**
      * The type_string_factory method is used to create new instances
      * of string type.  We don't go direct, as different generator will
      * use diffent type classes, depending on the code to be generated.
      *
      * @returns
      *     pointer to new type instance
      */
    virtual type::pointer type_string_factory() = 0;

    /**
      * The type_structure_factory method is used to create new
      * instances of struct types.  We don't go direct, as different
      * generator will use diffent type classes, depending on the code
      * to be generated.
      *
      * @param name
      *     the name of the structure
      * @returns
      *     pointer to new type instance
      */
    virtual type::pointer type_structure_factory(const nstring &name,
        bool global) = 0;

    /**
      * The type_time_factory method is used to create new instances of
      * time type.  We don't go direct, as different generator will use
      * diffent type classes, depending on the code to be generated.
      *
      * @returns
      *     pointer to new type instance
      */
    virtual type::pointer type_time_factory() = 0;

    /**
      * The top_level_factory method is used to create new instances of
      * the top level type (this writes the code to read and write files
      * of the top level struct).  We don't go direct, as different
      * generator will use diffent type classes, depending on the code
      * to be generated.
      */
    virtual type::pointer top_level_factory(const type::pointer &subtype) = 0;

    /**
      * The generate_file method is used to create the output file for
      * whatever purpose this generator serves.
      */
    virtual void generate_file() = 0;

    static nstring base_name(const nstring &path);

    /**
      * The printf method is used to print formatted output.
      *
      * @param fmt
      *     The format string, ehich controls the number and types
      *     of the remaining arguments.  See printf(3) for more
      *     information.
      */
    void printf(const char *fmt, ...)                         ATTR_PRINTF(2, 3);

    /**
      * The vprintf method is used to print formatted output.
      *
      * @param fmt
      *     The format string, ehich controls the number and types
      *     of the remaining arguments.  See vprintf(3) for more
      *     information.
      * @param ap
      *     where to find the remaining arguments
      */
    void vprintf(const char *fmt, va_list ap)                   ATTR_VPRINTF(2);

    /**
      * The wrap_and_print method is used to wrap the given text, and
      * emit the wrapped lines.
      *
      * @param text
      *     The text to pe wrapped and emitted.
      */
    void wrap_and_print(const nstring &text);

    /**
      * The wrap_and_print method is used to wrap the given text, and
      * emit the wrapped lines, adding the given prefix.
      *
      * @param prefix
      *     The constant string to add to the start of each wrapped line.
      * @param text
      *     The text to pe wrapped and emitted.
      */
    void wrap_and_print(const nstring &prefix, const nstring &text);

    void indent_more() { ip->more(); }

    void indent_less() { ip->less(); }

    /**
      * The include_once method is used to insert include lines into the
      * output.  Each include file will only ever be included once.
      *
      * @param filename
      *     the name of the file ot be included.
      */
    void include_once(const nstring &filename);

protected:
    /**
      * The this_file_is_generated method is used to insert a consistent
      * file header into the generated files.
      */
    void this_file_is_generated(void);

    /**
      * The get_file_name method may be used to obtain the name of the
      * file being written.
      */
    nstring get_file_name() const { return ip->get_file_name(); }

    /**
      * The calculate_include_file_name method may be used to calculate
      * the include file name (.h) when given the code file name (.cc).
      *
      * @param code_file-name
      *     the name of the code file (.cc)
      * @returns
      *     the name of the include file (.h)
      */
    nstring calculate_include_file_name(const nstring &code_file_name);

private:
    /**
      * The ip instance variable is used to remember when to write the output.
      */
    indent::pointer ip;

    /**
      * The default constructor.  Do not use.
      */
    generator();

    /**
      * The copy constructor.  Do not use.
      */
    generator(const generator &);

    /**
      * The assignment operator.  Do not use.
      */
    generator &operator=(const generator &);
};

#endif // FMTGEN_GENERATOR_H

//
// aegis - project change supervisor
// Copyright (C) 1991-1994, 1998, 2002-2008, 2012 Peter Miller
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
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#ifndef FMTGEN_TYPE_H
#define FMTGEN_TYPE_H

#include <common/nstring.h>
#include <common/ac/shared_ptr.h>

#include <fmtgen/lex.h>

#define ATTRIBUTE_REDEFINITION_OK 1
#define ATTRIBUTE_SHOW_IF_DEFAULT 2
#define ATTRIBUTE_HIDE_IF_DEFAULT 4

class generator; // forward
class type_vector; // forward

/**
  * The class type is used to represent the type of a field.  This is an
  * abstract base class.
  */
class type
{
public:
    typedef aegis_shared_ptr<type> pointer;

    /**
      * The destructor.
      */
    virtual ~type();

protected:
    /**
      * The constructor.
      *
      * @param gen
      *     the generator this type is bound to, typically for output
      *     context, but it could be more.
      * @param name
      *     The name of this type.
      * @param global
      *     true if the name is a global name, false if it is local to
      *     the containing .def file.
      */
    type(generator *gen, const nstring &name, bool global);

public:
    /**
      * The gen_body method is used to generate the file
      * contents for the typedef of this type.
      */
    virtual void gen_body() const;

    /**
      * The gen_declarator method is used to emit
      * a declarator (for the class interface)
      * for this type.
      *
      * @param name
      *     the name of the variable (member) being generated.
      * @param is_a_list
      *     true if this is a list object, false for POD or structures
      * @param attributes
      *     modifiers for the behaviour
      * @param comment
      *     The comment associated with this member.
      */
    virtual void gen_declarator(const nstring &name, bool is_a_list,
        int attributes, const nstring &comment) const;

    /**
      * The gen_write method is used to emit code which will
      * emit a serialization of this object.
      *
      * @param class_name
      *     the name of the class being generated.
      * @param member_name
      *     the name of the instance variable being generated.
      * @param show_default
      *     whether or not to show default values
      */
    virtual void gen_write(const nstring &form_name,
        const nstring &member_name, bool show_default) const;

    /**
      * The gen_call_xml method is used to emit code which will
      * emit an XML serialization of this object.
      *
      * @param form_name
      *     the name of the xml form being generated.
      * @param member_name
      *     the name of the variable (member) being generated.
      * @param show
      *     whether or not to show default values
      */
    virtual void gen_call_xml(const nstring &form_name,
        const nstring &member_name, int show) const;

    /**
      * The gen_copy method is used to generate deep-copy code for
      * a specific instance variable.
      *
      * @param name
      *     the name of the variable (member) being generated.
      */
    virtual void gen_copy(const nstring &member_name) const;

    /**
      * The gen_trace method is used to generate trace code for
      * a specific instance variable.
      *
      * @param name
      *     the name of the variable (member) being generated.
      * @param value
      *     FIXME: can't remember
      */
    virtual void gen_trace(const nstring &name, const nstring &value) const;

    /**
      * The gen_free_declarator method is sued to
      *
      * @param name
      *     the name of the variable (member) being generated.
      * @param is_a_list
      *     true if is a list, false if POD or a structure
      */
    virtual void gen_free_declarator(const nstring &name, bool is_a_list) const;

    /**
      * The gen_report_initializations method is used to emit the code
      * to initialize the enumeration value in the report generator.
      * The default implementation does nothing.
      *
      */
    virtual void gen_report_initializations();

    /**
      * The gen_default_constructor method is called to generate a
      * suitable default constructor of an instance variable, when
      * generating code for a C++ class.
      *
      * @param inst_var_name
      *     The name of the instance variable to be constructed.
      */
    virtual void gen_default_constructor(const nstring &inst_var_name);

    /**
      * The gen_copy_constructor method is called to generate a
      * suitable copy constructor of an instance variable, when
      * generating code for a C++ class.
      *
      * @param inst_var_name
      *     The name of the instance variable to be constructed.
      */
    virtual void gen_copy_constructor(const nstring &inst_var_name);

    /**
      * The gen_assignment_operator method is called to generate a
      * suitable assignment of an instance variable, when generating
      * code for a C++ class.
      *
      * @param inst_var_name
      *     The name of the instance variable to be copied.
      */
    virtual void gen_assignment_operator(const nstring &inst_var_name);

    /**
      * The gen_method method is called to generate getters and setters
      * for the given instance variable, for a C++ class.
      *
      * @param class_name
      *     The name of the class in which the getters and setters appear.
      * @param inst_var_name
      *     The name of the instance variable to be copied.
      */
    virtual void gen_methods(const nstring &class_name,
        const nstring &inst_var_name, int attributes) const;

    /**
      * The member_add method is used to extend a structure or an
      * enumeration by another member.
      *
      * @param mamber_name
      *     The name of the member being added
      * @param membet_type
      *     The type of the member being added
      * @param attributes
      *     The attributes (a bit map) of the member being added
      * @param comment
      *     The comment associated with (appears in the text immediately
      *     before) the member being added.
      */
    virtual void member_add(const nstring &member_name,
        const type::pointer &member_type, int attributes,
        const nstring &comment);

    /**
      * The in_include_file method is used to tell the type is in an
      * included file, not the top-level defintion file.
      */
    virtual void in_include_file();

    bool is_in_include_file() const;

    /**
      * The c_name function is used to get the C name to be used in
      * abstract declarators (and casts) of this type.
      */
    const nstring &
    c_name()
        const
    {
        if (!c_name_cache)
            c_name_cache = c_name_inner();
        return c_name_cache;
    }

    virtual bool has_a_mask() const = 0;

    const nstring &def_name() const { return name; }

    virtual void toplevel();

    void typedef_set() { is_a_typedef = true; }

    /**
      * The get_reachable method is used to obtain a list of types whech
      * are accesses by this type, and all the types of its members (if
      * any).
      *
      * @results
      *     append yourself and your subtypes here
      */
    virtual void get_reachable(type_vector &results) const;

    /**
      * The comment_set method is used to associate a comment with a
      * type.  This allows passing comments through from the source
      * .def. file and into (for example) Doxygen comments in the
      * generated .h file.
      *
      * @param text
      *     The text of the comment.
      */
    virtual void comment_set(const nstring &text);

    /**
      * The set_bit_fields class method is used to set the bit fields
      * flag, indicating whether or not to use bit fields in the
      * meta-data structures.  The advantage of bit fields is that they
      * potentially back to use less memory.  The disadvantage is that
      * they need slightly more CPU to access them.
      *
      * @param yesno
      *     true if use bit fields, false if not.
      */
    static void set_bit_fields(bool yesno);

protected:
    /**
      * The printf method is used to print formatted output, via our
      * associated generator.
      *
      * @param fmt
      *     The format string, which controls the number and types
      *     of the remaining arguments.  See printf(3) for more
      *     information.
      */
    void printf(const char *fmt, ...) const                   ATTR_PRINTF(2, 3);

    /**
      * The wrap_and_print method is used to print wrapped output, via our
      * associated generator.
      *
      * @param text
      *     The text to be wrapped and emitted.
      */
    void wrap_and_print(const nstring &text) const;

    /**
      * The wrap_and_print method is used to print wrapped output, via our
      * associated generator.
      *
      * @param prefix
      *     The constant string to add to the start of each wrapped line.
      * @param text
      *     The text to be wrapped and emitted.
      */
    void wrap_and_print(const nstring &prefix, const nstring &text) const;

    /**
      * The printf method is used to print formatted output, via our
      * associated generator.
      *
      * @param fmt
      *     The format string, which controls the number and types
      *     of the remaining arguments.  See vprintf(3) for more
      *     information.
      * @param ap
      *     where to find the remaining arguments
      */
    void vprintf(const char *fmt, va_list ap) const             ATTR_VPRINTF(2);

    void indent_more() const;
    void indent_less() const;

    /**
      * This helper method is used to replay an include_once method call
      * to the containing generator.  It causes an include directeive
      * to appear in the output, but each include file is only ever
      * included once.
      *
      * @param filename
      *     The name of the file to be included.
      */
    void include_once(const nstring &filename) const;

protected:
    /**
      * The gen instance vaiable is used to rememberr the generator this
      * type is bound to, typically for output context, but it could be
      * more.
      */
    generator *gen;

    /**
      * The default_instance_variable_comment method is used to create
      * an approximation of a useful Doxygen comment for an uncommented
      * instance variable.
      *
      * @param inst_var_name
      *     The name of the instance variable to be commented.
      * @param is_a_list
      *     true of he instance variable is a pointer to a list (vector)
      *     of values, false if normal.
      */
    nstring default_instance_variable_comment(const nstring &inst_var_name,
        bool is_a_list = false) const;

    /**
      * The comment instance variable is used to remember the comment
      * from the .def file associated with this type.
      */
    nstring comment;

    /**
      * The use_bit_fields class variable is used to remember whether or
      * not to use bit fields in the meta-data structures.
      */
    static bool use_bit_fields;

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
    mutable nstring c_name_cache;

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
    type();

    /**
      * The copy constructor.  Do not use.
      */
    type(const type &);

    /**
      * The assignment operator.  Do not use.
      */
    type &operator=(const type &);
};

#endif // FMTGEN_TYPE_H
// vim: set ts=8 sw=4 et :

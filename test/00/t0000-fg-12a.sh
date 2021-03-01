#!/bin/sh
#
# aegis - project change supervisor.
# Copyright (C) 2008, 2011, 2012 Peter Miller
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or (at
# your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program. If not, see <http://www.gnu.org/licenses/>.
#

TEST_SUBJECT="fmtgen vs redefinition_ok"

# load up standard prelude and test functions
. test_funcs

#
# test the functionality
#
activity="fmtgen"
cat > test.def << 'fubar'
t1a = boolean redefinition_ok;
t1b = [boolean] redefinition_ok;
fubar
if test $? -ne 0 ; then no_result; fi


cat > test.fmtgen.h.ok << 'fubar'
//
// This file is generated by fmtgen.
//

#ifndef TEST_FMTGEN_H
#define TEST_FMTGEN_H
#include <libaegis/introspector.h>
#include <libaegis/output.h>

#ifndef TEST_T1B_LIST_DEF
#define TEST_T1B_LIST_DEF

class test_t1b_list
{
public:
    /**
    * The pointer typedef is used to have a pointer type which may be used
    * by clients of this API, allowing the pointer implementation to be
    * changed without extensive code changes throughout the rest of the
    * code base.
    */
    typedef aegis_shared_ptr<test_t1b_list> pointer;

    /**
    * The destructor.
    */
    virtual ~test_t1b_list();

private:
    /**
    * The default constructor. It is private on purpose, use the #create
    * class method instead.
    */
    test_t1b_list();

public:
    /**
    * The create class method is used to create new dynamically allocated
    * instances of this class.
    */
    static pointer create();

private:
    /**
    * The copy constructor. It is private on purpose, use the #create class
    * method instead.
    *
    * @param rhs
    *     The right hand side of the initialization.
    */
    test_t1b_list(const test_t1b_list &rhs);

public:
    /**
    * The create class method is used to create new dynamically allocated
    * instances of this class.
    *
    * @param rhs
    *     The right hand side of the initialization.
    */
    static pointer create(const test_t1b_list &rhs);

    /**
    * The assignment operator.
    *
    * @param rhs
    *     The right hand side of the assignment.
    */
    test_t1b_list &operator=(const test_t1b_list &rhs);

    /**
    * The size method may be used to obtain the numer of items in this
    * list.
    */
    size_t size() const { return length; }

    /**
    * The get method is used to get the n'th element of this list.
    *
    * @param n
    *    The list element of interest, zero based.
    *    The results are undefined if n >= size().
    */
    bool &get(size_t n);

    /**
    * The clear method may be used to discard the list contents.
    */
    void clear();

    /**
    * The push_back method may be used to append a value to the end of this
    * list.
    *
    * @param value
    *     The value to be appended to the list.
    */
    void push_back(bool value);

    /**
    * The introspector_factory method is used to create an introspector for
    * this class and instance, usually used with the meta-data parser when
    * reading the contents of a meta-data file.
    */
    introspector::pointer introspector_factory();

    /**
    * The write method is used to write this structure's contents to the
    * given output.
    *
    * @param fp
    *     Where to write the output.
    * @param name
    *     The name of this structure member.
    */
    void write(const output::pointer &fp, const nstring &name) const;

    /**
    * The write_xml method is used to write this structure's contents to
    * the given output as XML.
    *
    * @param fp
    *     Where to write the output.
    * @param name
    *     The name of this structure member.
    */
    void write_xml(const output::pointer &fp, const nstring &name) const;

    /**
    * The trace_print method is used by the trace_test_t1b_list macro to
    * print an object of this type.
    *
    * @param name
    *     The name of the variable being printed.
    */
    void trace_print(const char *name) const;

    /**
    * The push_back_new_and_introspector_factory method is used to append a
    * value to the end of the list, and return an introspector for managing
    * that new value.
    */
    introspector::pointer push_back_new_and_introspector_factory();

private:
    /**
    * The list instance variable is used to remember the base address of a
    * dynamically allocated array of boolean values.
    */
    bool *list;

public:
    /**
    * The list_set method is used to set the #list instance variable, and
    * remember that it has been set.
    *
    * @param n
    *     The new value.
    */
    void list_set(bool n);

    /**
    * The list_get method is used to obtain the value of the #list instance
    * variable, or false if it has not been set.
    */
    bool list_get() const;

    /**
    * The list_is_set method is used to determine whether or not the #list
    * instance variable is set.
    */
    bool list_is_set() const;

    /**
    * The list_clear method is used to clear the value of the #list
    * instance variable, and remember that it is no longer set.
    */
    void list_clear();

    /**
    * The list_introspector_factory method is used to obtain an
    * introspector for the #list instance variable.
    */
    introspector::pointer list_introspector_factory();

    /**
    * The length instance variable is used to remember the number items
    * used in the #list array.
    */
    size_t length;

    /**
    * The maximum instance variable is used to remember the number items
    * allocated to the #list array.
    * @note
    *     assert(!list == !maximum);
    *     assert(length <= maximum);
    */
    size_t maximum;
};
#endif // TEST_T1B_LIST_DEF
#include <common/debug.h>
#ifndef test_t1b_list_trace
#ifdef DEBUG
#define test_t1b_list_trace(x) ((void)(trace_pretest_ && (trace_where_, test_t1b_list_trace_real(trace_stringize(x), x), 0)))
#else
#define test_t1b_list_trace(x)
#endif
#endif

#ifndef test_DEF
#define test_DEF

/**
* The test class is used to represent ''test'' formatted Aegis meta-data.
* See aetest(5) for more information.
*/
class test
{
public:
    /**
    * The pointer typedef is used to have a pointer type which may be used
    * by clients of this API, allowing the pointer implementation to be
    * changed without extensive code changes throughout the rest of the
    * code base.
    */
    typedef aegis_shared_ptr<test> pointer;

    /**
    * The destructor.
    */
    virtual ~test();

private:
    /**
    * The default constructor. It is private on purpose, use the #create
    * class method instead.
    */
    test();

public:
    /**
    * The create class method is used to create new dynamically allocated
    * instances of this class.
    */
    static pointer create();

private:
    /**
    * The copy constructor. It is private on purpose, use the #create class
    * method instead.
    *
    * @param rhs
    *     The right hand side of the initialization.
    */
    test(const test &rhs);

public:
    /**
    * The create class method is used to create new dynamically allocated
    * instances of this class.
    *
    * @param rhs
    *     The right hand side of the initialization.
    */
    static pointer create(const test &rhs);

    /**
    * The assignment operator.
    *
    * @param rhs
    *     The right hand side of the assignment.
    */
    test &operator=(const test &rhs);

private:
    /**
    * The t1a instance variable is used to remember the boolean t1a.
    */
    bool t1a: 1;

    /**
    * The t1a_is_set_flag instance variable is used to remember whether or
    * not the #t1a instance variable has been set.
    */
    bool t1a_is_set_flag: 1;

public:
    /**
    * The t1a_set method is used to set the #t1a instance variable, and
    * remember that it has been set.
    *
    * @param n
    *     The new value.
    */
    void t1a_set(bool n);

    /**
    * The t1a_get method is used to obtain the value of the #t1a instance
    * variable, or false if it has not been set.
    */
    bool t1a_get() const;

    /**
    * The t1a_is_set method is used to determine whether or not the #t1a
    * instance variable is set.
    */
    bool t1a_is_set() const;

    /**
    * The t1a_clear method is used to clear the value of the #t1a instance
    * variable, and remember that it is no longer set.
    */
    void t1a_clear();

    /**
    * The t1a_introspector_factory method is used to obtain an introspector
    * for the #t1a instance variable.
    */
    introspector::pointer t1a_introspector_factory();

private:
    /**
    * The t1b instance variable is used to remember the test t1b list t1b.
    */
    test_t1b_list::pointer t1b;

public:
    /**
    * The t1b_get method is used to obtain the value of the #t1b instance
    * variable. It will be created if it does not yet exist.
    */
    test_t1b_list::pointer t1b_get();

    /**
    * The t1b_is_set method is used to determine whether or not the #t1b
    * instance variable is set.
    */
    bool t1b_is_set() const;

    /**
    * The t1b_clear method is used to clear the value of the #t1b instance
    * variable, and remember that it is no longer set.
    */
    void t1b_clear();

    /**
    * The t1b_introspector_factory method is used to obtain an introspector
    * for the #t1b instance variable.
    */
    introspector::pointer t1b_introspector_factory();

public:
    /**
    * The introspector_factory method is used to create an introspector for
    * this class and instance, usually used with the meta-data parser when
    * reading the contents of a meta-data file.
    */
    introspector::pointer introspector_factory();

    /**
    * The write method is used to write this structure's contents to the
    * given output.
    *
    * @param fp
    *     Where to write the output.
    */
    void write(const output::pointer &fp) const;

    /**
    * The write_xml method is used to write this structure's contents to
    * the given output as XML.
    *
    * @param fp
    *     Where to write the output.
    */
    void write_xml(const output::pointer &fp) const;

    /**
    * The trace_print method is used by the trace_test macro to print an
    * object of this type.
    *
    * @param name
    *     The name of the variable being printed.
    */
    void trace_print(const char *name) const;

    /**
    * The create_from_file class method is used to read the given file to
    * create a new dynamically allocated instance of this class.
    *
    * @param filename
    *     The name of the file to be read.
    * @note
    *     If any errors are encountered, this method will not return. All
    *     errors will print a fatal error message, and exit with an exit
    *     status of 1.
    */
    static pointer create_from_file(const nstring &filename);

    /**
    * The write_file method is used to write the data to the named file,
    * via the #write method.
    *
    * @param filename
    *     The name of the file to be written.
    * @param compress
    *     true if data should be compressed, false if not.
    * @note
    *     If any errors are encountered, this method will not return. All
    *     errors will print a fatal error message, and exit with an exit
    *     status of 1.
    */
    void write_file(const nstring &filename, bool compress) const;

    /**
    * The report_init class method is used to initialize the report
    * generator with the names and values of enumerations used by this
    * meta-data.
    */
    static void report_init();
};

/**
* The test_trace macro is used to conditionally print a trace of a test
* value, if the DEBUG macro is defined.
*/
#ifndef test_trace
#ifdef DEBUG
#define test_trace(x) ((void)(trace_pretest_ && (trace_where_, (x).trace_print(trace_stringize(x)), 0)))
#else
#define test_trace(x)
#endif
#endif

#endif // test_DEF

#endif // TEST_FMTGEN_H
fubar
if test $? -ne 0 ; then no_result; fi

fmtgen -ii test.fmtgen.h test.def
test $? -eq 0 || fail

diff test.fmtgen.h.ok test.fmtgen.h
test $? -eq 0 || fail


cat > test.fmtgen.cc.ok << 'fubar'
//
// This file is generated by fmtgen.
//
#include <test.fmtgen.h>
#include <common/ac/assert.h>
#include <libaegis/aer/value/list.h>
#include <common/trace.h>

void
test_t1b_list::write(const output::pointer &fp, const nstring &name)
    const
{
    trace(("%s {\n", __PRETTY_FUNCTION__));
    if (name)
    {
        fp->fputs(name);
        fp->fputs(" =\n");
    }
    assert(length <= maximum);
    assert(!list == !maximum);
    fp->fputs("[\n");
    for (size_t j = 0; j < length; ++j)
    {
        introspector_boolean::write(fp, "list[j]", list[j], true);
        fp->fputs(",\n");
    }
    fp->fputs("]");
    if (name)
        fp->fputs(";\n");
    trace(("}\n"));
}

void
test_t1b_list::write_xml(const output::pointer &fp, const nstring &name)
    const
{
    trace(("%s {\n", __PRETTY_FUNCTION__));
    assert(name);
    fp->fputc('<');
    fp->fputs(name);
    fp->fputs(">\n");
    assert(length <= maximum);
    assert(!list == !maximum);
    for (size_t j = 0; j < length; ++j)
    {
        introspector_boolean::write_xml(fp, "list[j]", list[j], true);
    }
    fp->fputs("</");
    fp->fputs(name);
    fp->fputs(">\n");
    trace(("}\n"));
}

test_t1b_list::test_t1b_list() :
    list(0),
    length(0),
    maximum(0)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}

test_t1b_list::~test_t1b_list()
{
    trace(("%s {\n", __PRETTY_FUNCTION__));
    delete [] list;
    list = 0;
    length = 0;
    maximum = 0;
    trace(("}\n"));
}

test::~test()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}

test::test() :
    t1a(false),
    t1a_is_set_flag(false),
    t1b()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}

test::pointer
test::create()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return pointer(new test());
}

test::test(const test &rhs) :
    t1a(rhs.t1a),
    t1a_is_set_flag(rhs.t1a_is_set_flag),
    t1b(rhs.t1b)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}

test::pointer
test::create(const test &rhs)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return pointer(new test(rhs));
}

test &
test::operator=(const test &rhs)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    if (this != &rhs)
    {
        t1a = rhs.t1a;
        t1a_is_set_flag = rhs.t1a_is_set_flag;
        t1b = rhs.t1b;
    }
    return *this;
}

bool
test::t1a_get()
    const
{
    return t1a;
}

bool
test::t1a_is_set()
    const
{
    return (t1a || t1a_is_set_flag);
}

void
test::t1a_set(bool x)
{
    t1a = x;
    t1a_is_set_flag = true;
}

void
test::t1a_clear()
{
    t1a = false;
    t1a_is_set_flag = false;
}
#include <libaegis/introspector/boolean/by_method.h>

introspector::pointer
test::t1a_introspector_factory()
{
    return introspector_boolean_by_method < test > ::create ( *this,
        &test::t1a_set, &test::t1a_is_set);
}

test_t1b_list::pointer
test::t1b_get()
{
    if (!t1b)
    {
        t1b = test_t1b_list::create();
    }
    return t1b;
}

bool
test::t1b_is_set()
    const
{
    return !!t1b;
}

introspector::pointer
test::t1b_introspector_factory()
{
    return t1b_get()->introspector_factory();
}
#include <libaegis/introspector/structure.h>

introspector::pointer
test::introspector_factory()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    introspector_structure::pointer ip =
        introspector_structure::create("test");
    ip->register_member ( "t1a", introspector_structure::adapter_by_method
        < test > ::create ( this, &test::t1a_introspector_factory,
            &test::t1a_is_set));
    ip->register_member ( "t1b", introspector_structure::adapter_by_method
        < test > ::create ( this, &test::t1b_introspector_factory,
            &test::t1b_is_set));
    return ip;
};

void
test::write(const output::pointer &fp)
    const
{
    trace(("%s\n{\n", __PRETTY_FUNCTION__));
    introspector_boolean::write(fp, "t1a", t1a, t1a_is_set_flag);
    if (t1b)
    {
        t1b->write ( fp, "t1b");
    }
    trace(("}\n"));
}

void
test::write_xml(const output::pointer &fp)
    const
{
    trace(("%s\n{\n", __PRETTY_FUNCTION__));
    fp->fputs("<test>\n");
    introspector_boolean::write_xml(fp, "t1a", t1a, t1a_is_set_flag);
    if (t1b)
        t1b->write_xml(fp, "t1b");
    fp->fputs("</test>\n");
    trace(("}\n"));
}

void
test::trace_print(const char *name_)
    const
{
    if (name_ && *name_)
    {
        trace_printf("%s = ", name_);
    }
    trace_printf("{\n");
    if (t1a_is_set())
        trace_bool_real("t1a", t1a_get());
    if (t1b)
    {
        t1b->trace_print( "t1b");
    }
    trace_printf("}");
    trace_printf((name_ && *name_) ? ";\n" : ",\n");
}
#include <libaegis/meta_context/introspector.h>
#include <libaegis/os.h>

test::pointer
test::create_from_file(const nstring &filename)
{
    trace(("%s\n{\n", __PRETTY_FUNCTION__));
    pointer p = create();
    introspector::pointer ip = p->introspector_factory();
    meta_context_introspector meta;
    os_become_must_be_active();
    meta.parse_file(filename, ip);
    trace(("}\n"));
    return p;
}
#include <libaegis/io.h>
#include <libaegis/output/file.h>
#include <libaegis/output/filter/gzip.h>
#include <libaegis/output/filter/indent.h>

void
test::write_file(const nstring &filename, bool needs_compression)
    const
{
    trace(("%s\n{\n", __PRETTY_FUNCTION__));
    if (filename)
        os_become_must_be_active();
    output::pointer fp = (needs_compression ?
        output_filter_gzip::create(output_file::binary_open(filename)) :
        output_file::text_open(filename));
    fp = output_filter_indent::create(fp);
    io_comment_emit(fp);
    write(fp);
    type_enum_option_clear();
    trace(("}\n"));
}

void
test::report_init(void)
{
    trace(("%s\n{\n", __PRETTY_FUNCTION__));
    trace(("}\n"));
}
fubar
if test $? -ne 0 ; then no_result; fi

fmtgen -ic test.fmtgen.cc test.def
test $? -eq 0 || fail

diff test.fmtgen.cc.ok test.fmtgen.cc
test $? -eq 0 || fail

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass


// vim: set ts=8 sw=4 et :

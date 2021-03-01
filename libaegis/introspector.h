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

#ifndef LIBAEGIS_INTROSPECTOR_H
#define LIBAEGIS_INTROSPECTOR_H

#include <common/ac/shared_ptr.h>

class nstring; // forward
class sub_context_ty; // forward

/**
  * The introspector class is used to represent the ability to reach
  * into a variable and manipulate it.  This is exploited by the
  * meta-data parser to enable it to set various fields within a
  * meta-data structure.
  */
class introspector
{
public:
    typedef aegis_shared_ptr<introspector> pointer;

    /**
      * The destructor.
      */
    virtual ~introspector();

protected:
    /**
      * The default constructor.
      * It is protected on purpose, only derived classes may call it.
      */
    introspector();

public:
    /**
      * The integer method is called by the parser when an integer value
      * is seen.
      *
      * @param n
      *     The value of the number.
      */
    virtual void integer(long n);

    /**
      * The real method is called by the parser when a floating point
      * value is seen.
      *
      * @param n
      *     The value of the number.
      */
    virtual void real(double n);

    /**
      * The string method is called by the parser when a string value is
      * seen.
      *
      * @param text
      *     The value of the string constant.
      */
    virtual void string(const nstring &text);

    /**
      * The enumeration method is called when the parser sees a name in
      * an enumeration tag place.
      *
      * @param name
      *     The name of the enumerand.
      */
    virtual void enumeration(const nstring &name);

    /**
      * The list method is called when the parser sees the start of a
      * list value.
      */
    virtual pointer list();

    /**
      * The field method is called when the parser sees the start of a
      * name=value field.
      *
      * @param name
      *     The name of the field.
      */
    virtual pointer field(const nstring &name);

    /**
      * The get_name mathod is used to get the name of the type of value
      * the introspector is managing.  Used in error messages.
      */
    virtual nstring get_name() const = 0;

protected:
    /**
      * The error method is a helper which proxies errors to the
      * global lex_error function, for reporting error messages.
      * (Reduces include explosion.)
      *
      * @param scp
      *     substitution context
      * @param text
      *     the text of the error message
      */
    void error(sub_context_ty *scp, const char *text);

    /**
      * The value_of_type_integer_required method is used to emit the
      * "value of type <name> required" error message.
      */
    void value_of_type_required();

private:
    /**
      * The copy constructor.  Do not use.
      */
    introspector(const introspector &);

    /**
      * The assignment operator.  Do not use.
      */
    introspector &operator=(const introspector &);
};

#endif // LIBAEGIS_INTROSPECTOR_H

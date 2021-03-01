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

#ifndef LIBAEGIS_META_CONTEXT_H
#define LIBAEGIS_META_CONTEXT_H

#include <common/nstring.h>

class sub_context_ty; // forward

/**
  * The meta_context abstract base class is used to represent
  * the interface to the use of parsed Aegis meta-data.
  */
class meta_context
{
public:
    /**
      * The destructor.
      */
    virtual ~meta_context();

    /**
      * The default constructor.
      */
    meta_context();

    /**
      * The integer method is called by the parser when an integer value
      * is seen.
      *
      * @param n
      *     The value of the number.
      */
    virtual void integer(long n) = 0;

    /**
      * The real method is called by the parser when a floating point
      * value is seen.
      *
      * @param n
      *     The value of the number.
      */
    virtual void real(double n) = 0;

    /**
      * The string method is called by the parser when a string value is
      * seen.
      *
      * @param s
      *     The value of the string constant.
      */
    virtual void string(const nstring &s) = 0;

    /**
      * The enumeration method is called when the parser sees a name in
      * an enumeration tag place.
      *
      * @param s
      *     The name of the enumerand.
      */
    virtual void enumeration(const nstring &s) = 0;

    /**
      * The list method is called when the parser sees the start of a
      * list value.  The pop method will be called after the value has been
      * parsed.
      */
    virtual void list() = 0;

    /**
      * The list_end method is called by the parser when it sees the
      * end of a list element.
      */
    virtual void list_end() = 0;

    /**
      * The field method is called when the parser sees the start of a
      * name=value field.  The field_end method will be called after the
      * value has been parsed.
      *
      * @param name
      *     The nam eof the field.
      */
    virtual void field(const nstring &name) = 0;

    /**
      * The field_end method is called by the parser when it sees the
      * end of a name=value field.
      */
    virtual void field_end() = 0;

    /**
      * The end method is called at the end of input.
      */
    virtual void end() = 0;

protected:
    /**
      * The lex_error method is a helper which proxies errors to the
      * global lex_error function, for reporting error messages.
      * (Reduces include explosion.)
      *
      * @param scp
      *     substitution context
      * @param text
      *     the text of the error message
      */
    void error(sub_context_ty *scp, const char *text);

private:
    /**
      * The copy constructor.  Do not use.
      */
    meta_context(const meta_context &);

    /**
      * The assignment operator.  Do not use.
      */
    meta_context &operator=(const meta_context &);
};

#endif // LIBAEGIS_META_CONTEXT_H

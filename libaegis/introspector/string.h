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

#ifndef LIBAEGIS_INTROSPECTOR_STRING_H
#define LIBAEGIS_INTROSPECTOR_STRING_H

#include <libaegis/introspector.h>
#include <libaegis/output.h>

class string_ty; // forward

/**
  * The introspector_string class is used to represent the manipulations
  * required to manage string_ty* values by the meta-data parser.
  */
class introspector_string:
    public introspector
{
public:
    /**
      * The destructor.
      */
    virtual ~introspector_string();

private:
    /**
      * The constructor.
      * It is private on purpose, use the #create class method instead.
      *
      * @param where
      *     The location of the string_ty* value to be manipulated.
      */
    introspector_string(string_ty *&where);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param where
      *     The location of the string_ty* value to be manipulated.
      */
    static pointer create(string_ty *&where);

    /**
      * The write class method is used to write the value of the given
      * string text to the given output.
      *
      * @param op
      *     The output stream to write on
      * @param name
      *     The name of the string field (NULL if list).
      * @param value
      *     The value of he string field.
      */
    static void write(const output::pointer &op, const nstring &name,
        const nstring &value);

    /**
      * The write_xml class method is used to write the value of the
      * given string as an XML element to the given output.
      *
      * @param op
      *     The output stream to write on
      * @param name
      *     The name of the string field (NULL if list).
      * @param value
      *     The value of he string field.
      */
    static void write_xml(const output::pointer &op, const nstring &name,
        const nstring &value);

protected:
    // See base class for documentation.
    void string(const nstring &text);

    // See base class for documentation.
    nstring get_name() const;

private:
    /**
      * The where instance variable is ued to remember where to put the
      * value, once parsed.
      */
    string_ty *&where;

    /**
      * The default constructor.  Do not use.
      */
    introspector_string();

    /**
      * The copy constructor.  Do not use.
      */
    introspector_string(const introspector_string &);

    /**
      * The assignment operator.  Do not use.
      */
    introspector_string &operator=(const introspector_string &);
};

#endif // LIBAEGIS_INTROSPECTOR_STRING_H

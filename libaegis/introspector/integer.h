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

#ifndef LIBAEGIS_INTROSPECTOR_INTEGER_H
#define LIBAEGIS_INTROSPECTOR_INTEGER_H

#include <libaegis/introspector.h>
#include <libaegis/output.h>

/**
  * The introspector_integer class is used to represent the manipulations
  * required to manage integers by the meta-data parser.
  */
class introspector_integer:
    public introspector
{
public:
    /**
      * The destructor.
      */
    virtual ~introspector_integer();

private:
    /**
      * The constructor.
      * It is private on purpose, use the #create class method instead.
      *
      * @param where
      *     The location of the integer value to be manipulated.
      */
    introspector_integer(long &where);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param where
      *     The location of the integer value to be manipulated.
      */
    static pointer create(long &where);

    /**
      * The write class method is used to write the named field to the
      * given output.
      *
      * @param op
      *     The output to write the value on
      * @param name
      *     The name of the field being written
      * @param value
      *     The value of the field being written
      * @param show_if_default
      *     Whether or not to display zero values, or hide them.
      */
    static void write(const output::pointer &op, const nstring &name,
        long value, bool show_if_default = false);

    /**
      * The write_xml class method is used to write the named field to the
      * given output as an XML element.
      *
      * @param op
      *     The output to write the value on
      * @param name
      *     The name of the field being written
      * @param value
      *     The value of the field being written
      * @param show_if_default
      *     Whether or not to display zero values, or hide them.
      */
    static void write_xml(const output::pointer &op, const nstring &name,
        long value, bool show_if_default = false);

protected:
    // See base class for documentation.
    void integer(long n);

    // See base class for documentation.
    void real(double n);

    // See base class for documentation.
    nstring get_name() const;

private:
    /**
      * The where instance variable is ued to remember where to put the
      * value, once parsed.
      */
    long &where;

    /**
      * The default constructor.  Do not use.
      */
    introspector_integer();

    /**
      * The copy constructor.  Do not use.
      */
    introspector_integer(const introspector_integer &);

    /**
      * The assignment operator.  Do not use.
      */
    introspector_integer &operator=(const introspector_integer &);
};

#endif // LIBAEGIS_INTROSPECTOR_INTEGER_H

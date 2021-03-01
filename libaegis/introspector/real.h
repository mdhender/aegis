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

#ifndef LIBAEGIS_INTROSPECTOR_REAL_H
#define LIBAEGIS_INTROSPECTOR_REAL_H

#include <libaegis/introspector.h>
#include <libaegis/output.h>

/**
  * The introspector_real class is used to represent the manipulations
  * required to manage floating point values by the meta-data parser.
  */
class introspector_real:
    public introspector
{
public:
    /**
      * The destructor.
      */
    virtual ~introspector_real();

private:
    /**
      * The constructor.
      * It is private on purpose, use the #create class method instead.
      *
      * @param where
      *     The location of the floating point value to be manipulated.
      */
    introspector_real(double &where);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param where
      *     The location of the floating point value to be manipulated.
      */
    static pointer create(double &where);

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
        double value, bool show_if_default = false);

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
        double value, bool show_if_default = false);

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
    double &where;

    /**
      * The default constructor.  Do not use.
      */
    introspector_real();

    /**
      * The copy constructor.  Do not use.
      */
    introspector_real(const introspector_real &);

    /**
      * The assignment operator.  Do not use.
      */
    introspector_real &operator=(const introspector_real &);
};

#endif // LIBAEGIS_INTROSPECTOR_REAL_H

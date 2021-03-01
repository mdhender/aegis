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

#ifndef LIBAEGIS_INTROSPECTOR_BOOLEAN_H
#define LIBAEGIS_INTROSPECTOR_BOOLEAN_H

#include <libaegis/introspector.h>
#include <libaegis/output.h>

/**
  * The introspector_boolean class is used to represent the
  * manipulations required to manage bool instance variables by the
  * meta-data parser.  It is an abstract base class, further derivation
  * is required before a useable introspector can be constructed.
  */
class introspector_boolean:
    public introspector
{
public:
    /**
      * The destructor.
      */
    virtual ~introspector_boolean();

protected:
    /**
      * The constructor.
      */
    introspector_boolean();

public:
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
        bool value, bool show_if_default = false);

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
        bool value, bool show_if_default = false);

protected:
    // See base class for documentation.
    nstring get_name() const;

private:
    /**
      * The copy constructor.  Do not use.
      */
    introspector_boolean(const introspector_boolean &);

    /**
      * The assignment operator.  Do not use.
      */
    introspector_boolean &operator=(const introspector_boolean &);
};

#endif // LIBAEGIS_INTROSPECTOR_BOOLEAN_H

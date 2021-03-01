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

#ifndef LIBAEGIS_INTROSPECTOR_NSTRING_H
#define LIBAEGIS_INTROSPECTOR_NSTRING_H

#include <libaegis/introspector.h>
#include <libaegis/output.h>

/**
  * The introspector_nstring class is used to represent the manipulations
  * required to manage nstring values by the meta-data parser.
  */
class introspector_nstring:
    public introspector
{
public:
    /**
      * The destructor.
      */
    virtual ~introspector_nstring();

private:
    /**
      * The constructor.
      * It is private on purpose, use the #create class method instead.
      *
      * @param where
      *     The location of the string_ty* value to be manipulated.
      */
    introspector_nstring(nstring &where);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param where
      *     The location of the string_ty* value to be manipulated.
      */
    static pointer create(nstring &where);

    /**
      * The write class method is used to write out a string value in
      * the format the meta-data parser can read back in.
      *
      * @param op
      *     The output sink to send the text to.
      * @param name
      *     The name of the field to be written.
      * @param value
      *     The value of the field to be written.
      * @param show_if_default
      *     whether or not to show the variable even if it is the empty string.
      */
    static void write(const output::pointer &op, const char *name,
        const nstring &value, bool show_if_default = false);

    /**
      * The write_xml class method is used to write out a string value
      * in XML format.
      *
      * @param op
      *     The output sink to send the text to.
      * @param name
      *     The name of the field to be written.
      * @param value
      *     The value of the field to be written.
      * @param show_if_default
      *     whether or not to show the variable even if it is the empty string.
      */
    static void write_xml(const output::pointer &op, const char *name,
        const nstring &value, bool show_if_default = false);

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
    nstring &where;

    /**
      * The default constructor.  Do not use.
      */
    introspector_nstring();

    /**
      * The copy constructor.  Do not use.
      */
    introspector_nstring(const introspector_nstring &);

    /**
      * The assignment operator.  Do not use.
      */
    introspector_nstring &operator=(const introspector_nstring &);
};

#endif // LIBAEGIS_INTROSPECTOR_NSTRING_H

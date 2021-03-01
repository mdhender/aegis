//
//	aegis - project change supervisor
//	Copyright (C) 1994, 1996, 2002, 2005-2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#ifndef AEGIS_AER_VALUE_STRING_H
#define AEGIS_AER_VALUE_STRING_H

#include <common/nstring.h>
#include <libaegis/aer/value.h>

/**
  * The rpt_value_string class is used to represent a value which is an
  * string
  */
class rpt_value_string:
    public rpt_value
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_value_string();

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * class method nstead.
      *
      * @param value
      *     The value of the string.
      */
    rpt_value_string(const nstring &value);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param value
      *     The value of the string.
      */
    static rpt_value::pointer create(const nstring &value);

    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param value
      *     The value of the string.
      */
    static rpt_value::pointer create(const char *value);

    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param value
      *     The value of the string.
      */
    static rpt_value::pointer create(string_ty *value) DEPRECATED;

    /**
      * The query method may be used to obtain the value of this class.
      */
    nstring query() const;

protected:
    // See base class for documentation.
    rpt_value::pointer integerize_or_null() const;

    // See base class for documentation.
    rpt_value::pointer realize_or_null() const;

    // See base class for documentation.
    rpt_value::pointer arithmetic_or_null() const;

    // See base class for documentation.
    rpt_value::pointer booleanize_or_null() const;

    // See base class for documentation.
    const char *name() const;

private:
    /**
      * The value instance variable is used to remember the value of
      * this object.
      */
    nstring value;

    /**
      * The default constructor.  Do not use.
      */
    rpt_value_string();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_value_string(const rpt_value_string &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_value_string &operator=(const rpt_value_string &);
};

#endif // AEGIS_AER_VALUE_STRING_H

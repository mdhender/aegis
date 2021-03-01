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

#ifndef AEGIS_AER_VALUE_REF_H
#define AEGIS_AER_VALUE_REF_H

#include <libaegis/aer/value.h>

/**
  * The rpt_value_reference class is used to represent a proxy for
  * another value.  This is how variables are implemented in the report
  * generator.
  */
class rpt_value_reference:
    public rpt_value
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_value_reference();

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * class method instead.
      */
    rpt_value_reference(const rpt_value::pointer &value);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instance of this class.
      */
    static rpt_value::pointer create(const rpt_value::pointer &value);

    /**
      * The get method may be used to get the value being referenced.
      */
    rpt_value::pointer get() const;

    /**
      * The set method may be used to set the value being referenced.
      * Only use this if you are implementing a report generator
      * variable.
      *
      * @param value
      *     The new value for the variable.
      */
    void set(const rpt_value::pointer &value);

protected:
    // See base class for documentation.
    rpt_value::pointer integerize_or_null() const;

    // See base class for documentation.
    rpt_value::pointer realize_or_null() const;

    // See base class for documentation.
    rpt_value::pointer arithmetic_or_null() const;

    // See base class for documentation.
    rpt_value::pointer stringize_or_null() const;

    // See base class for documentation.
    rpt_value::pointer booleanize_or_null() const;

    // See base class for documentation.
    rpt_value::pointer lookup(const rpt_value::pointer &rhs, bool lvalue) const;

    // See base class for documentation.
    rpt_value::pointer keys() const;

    // See base class for documentation.
    rpt_value::pointer count() const;

    // See base class for documentation.
    const char *type_of() const;

    // See base class for documentation.
    const char *name() const;

private:
    /**
      * The value instance variable is used to remember the location of
      * the value being referenced.
      */
    rpt_value::pointer value;

    /**
      * The default constructor.  Do not use.
      */
    rpt_value_reference();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_value_reference(const rpt_value_reference &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_value_reference &operator=(const rpt_value_reference &);
};

#endif // AEGIS_AER_VALUE_REF_H

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

#ifndef AEGIS_AER_VALUE_FUNC_H
#define AEGIS_AER_VALUE_FUNC_H

#include <libaegis/aer/value.h>
#include <libaegis/aer/func.h>

/**
  * The rpt_value_func class is used to represent a function which may
  * be called from within the report generator.
  */
class rpt_value_func:
    public rpt_value
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_value_func();

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * class method instead.
      */
    rpt_value_func(const rpt_func::pointer &value);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      */
    static rpt_value::pointer create(const rpt_func::pointer &value);

    /**
      * The query method may be used to obtain the value.
      */
    rpt_func::pointer query() const;

protected:
    // See base class for documentation.
    const char *name() const;

    // See base class for documentation.
    rpt_value::pointer stringize_or_null() const;

private:
    /**
      * The value instance variable is used to remember the value of
      * this object.
      */
    rpt_func::pointer value;

    /**
      * The default constructor.  Do not use.
      */
    rpt_value_func();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_value_func(const rpt_value_func &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_value_func &operator=(const rpt_value_func &);
};

#endif // AEGIS_AER_VALUE_FUNC_H

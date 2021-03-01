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

#ifndef AEGIS_AER_VALUE_VOID_H
#define AEGIS_AER_VALUE_VOID_H

#include <libaegis/aer/value.h>


/**
  * The rpt_value_void class is used to represent a value which is a
  * void.  Unlike rpt_value_null, it cannot be coerced into a string or
  * an integer.
  */
class rpt_value_void:
    public rpt_value
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_value_void();

private:
    /**
      * The default constructor.  It is private on purpose, use the
      * "create" class method nstead.
      */
    rpt_value_void();

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      */
    static rpt_value::pointer create();

protected:
    // See base class for documentation.
    const char *name() const;

private:
    /**
      * The copy constructor.  Do not use.
      */
    rpt_value_void(const rpt_value_void &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_value_void &operator=(const rpt_value_void &);
};

#endif // AEGIS_AER_VALUE_VOID_H

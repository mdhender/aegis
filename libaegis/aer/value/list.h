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

#ifndef AEGIS_AER_VALUE_LIST_H
#define AEGIS_AER_VALUE_LIST_H

#include <libaegis/aer/value.h>

/**
  * The rpt_value_list class is used to represent a value which consists
  * of an ordered list of values.
  */
class rpt_value_list:
    public rpt_value
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_value_list();

    /**
      * The constructor.
      */
    rpt_value_list();

    /**
      * The create class method may be used to create new dynamically
      * allocated instances of this class.
      */
    static rpt_value::pointer create();

    /**
      * The append mthod may be used to append another value to the end
      * of the list of values held.
      */
    void append(const rpt_value::pointer &vp);

    /**
      * The size method may be used to determine how many value are in
      * the list.
      */
    size_t size() const { return length; }

    /**
      * The nth method may be used to obtain the n-th value in the list.
      */
    rpt_value::pointer nth(size_t n) const;

protected:
    // See base class for documentation
    const char *name() const;

    // See base class for documentation
    rpt_value::pointer undefer() const;

    // See base class for documentation
    rpt_value::pointer lookup(const rpt_value::pointer &rhs, bool lvalue) const;

    // See base class for documentation
    rpt_value::pointer keys() const;

    // See base class for documentation
    rpt_value::pointer count() const;

private:
    /**
      * The length instance variable is used to remember the amount of
      * "item" used so far.
      */
    size_t length;

    /**
      * The max instance variable is used to remember how large the
      * "item" array was allocated.
      */
    size_t max;

    /**
      * The item instance variable is used to remember the base address
      * of a dynamically allocated array of ppinters to values.
      */
    rpt_value::pointer *item;

    /**
      * The copy constructor.  Do not use.
      */
    rpt_value_list(const rpt_value_list &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_value_list &operator=(const rpt_value_list &);
};

#endif // AEGIS_AER_VALUE_LIST_H

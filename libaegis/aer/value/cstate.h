//
//      aegis - project change supervisor
//      Copyright (C) 1994, 1996, 2002, 2005-2008, 2011, 2012 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#ifndef AEGIS_AER_VALUE_CSTATE_H
#define AEGIS_AER_VALUE_CSTATE_H

#include <libaegis/aer/value.h>

struct project; // forward

/**
  * The rpt_value_cstate class is used to represent the state of a change.
  */
class rpt_value_cstate:
    public rpt_value
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_value_cstate();

private:
    /**
      * The constructor.
      *
      * @param pp
      *     The project involved
      * @param length
      *     The length of the next argument
      * @param list
      *     The address of an array of change numbers (zero encoded)
      */
    rpt_value_cstate(project *pp, size_t length, const long *list);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param pp
      *     The project involved
      * @param length
      *     The length of the next argument
      * @param list
      *     The address of an array of change numbers (zero encoded)
      */
    static rpt_value::pointer create(project *pp, size_t length,
        const long *list);

protected:
    // See base class for documentation.
    const char *name() const;

    // See base class for documentation.
    bool is_a_struct() const;

    // See base class for documentation.
    rpt_value::pointer lookup(const rpt_value::pointer &rhs, bool lvalue) const;

    // See base class for documentation.
    rpt_value::pointer keys() const;

    // See base class for documentation.
    rpt_value::pointer count() const;

    // See base class for documentation.
    const char *type_of() const;

private:
    /**
      * The pp instance varbale is used to remember the project the
      * changes belong to.
      */
    project *pp;

    /**
      * The length instance variable is used to remember the number of
      * change numbers in the "list" array.
      */
    size_t length;

    /**
      * The list instance variable is used to remember the base of an
      * array of change numbers.
      */
    long *list;

    /**
      * The default constructor.  Do not use.
      */
    rpt_value_cstate();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_value_cstate(const rpt_value_cstate &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_value_cstate &operator=(const rpt_value_cstate &);
};

#endif // AEGIS_AER_VALUE_CSTATE_H
// vim: set ts=8 sw=4 et :

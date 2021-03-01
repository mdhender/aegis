//
//      aegis - project change supervisor
//      Copyright (C) 1995, 1996, 2002, 2005-2008, 2012 Peter Miller
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

#ifndef AEGIS_AER_VALUE_FSTATE_H
#define AEGIS_AER_VALUE_FSTATE_H

#include <libaegis/aer/value.h>
#include <libaegis/change.h>

/**
  * The rpt_value_fstate class is used tp represent deferred change
  * file state data.
  */
class rpt_value_fstate:
    public rpt_value
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_value_fstate();

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * class method instead.
      *
      * @param cp
      *     The change of interest.
      */
    rpt_value_fstate(const change::pointer &cp);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param cp
      *     The change of interest.
      */
    static rpt_value::pointer create(const change::pointer &cp);

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
    rpt_value::pointer undefer_or_null() const;

    // See base class for documentation.
    const char *type_of() const;

private:
    /**
      * The the cp instance variable is used to remember the change set
      * of interest.
      */
    change::pointer cp;

    /**
      * The converted instance variable is used to remember the change
      * file state, once it is converted into a rpt_value_* tree.  It is
      * only ever converted on demand.
      */
    mutable rpt_value::pointer converted;

    /**
      * The convert method is used to convert the fstate data into a
      * rpt_value_* tree value.
      */
    void convert() const;

    /**
      * The default constructor.  Do not use.
      */
    rpt_value_fstate();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_value_fstate(const rpt_value_fstate &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_value_fstate &operator=(const rpt_value_fstate &);
};

#endif // AEGIS_AER_VALUE_FSTATE_H
// vim: set ts=8 sw=4 et :

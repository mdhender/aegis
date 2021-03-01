//
//      aegis - project change supervisor
//      Copyright (C) 2001, 2002, 2005-2008, 2012 Peter Miller
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

#ifndef LIBAEGIS_AER_VALUE_PCONF_H
#define LIBAEGIS_AER_VALUE_PCONF_H

#include <libaegis/aer/value.h>
#include <libaegis/change.h>

/**
  * The rpt_value_pconf class is used to represent a deferred valie of a
  * project's configuration data (aegis.conf file).
  */
class rpt_value_pconf:
    public rpt_value
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_value_pconf();

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * class method instead.
      *
      * @param cp
      *     The change to use to access the project config data.
      */
    rpt_value_pconf(const change::pointer &cp);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param cp
      *     The change to use to access the project config data.
      */
    static rpt_value::pointer create(const change::pointer &cp);

protected:
    // See base class for documentation.
    const char *name() const;

    // See base class for documentation.
    const char *type_of() const;

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

private:
    /**
      * The cp instance variable is used to remember the change through
      * which to acess the project configuration data (particular;y as a
      * change set is the mechanism to change it).
      */
    change::pointer cp;

    /**
      * The value instance variable is used to remember the read-in and
      * converted meta-data.
      *
      * It is mutable because performing the deferred read does not
      * change the semantic value of the object.
      */
    mutable rpt_value::pointer value;

    /**
      * The grab method is used to read in and convert the meta data.
      */
    void grab() const;

    /**
      * The default constructor.  Do not use.
      */
    rpt_value_pconf();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_value_pconf(const rpt_value_pconf &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_value_pconf &operator=(const rpt_value_pconf &);
};


#endif // LIBAEGIS_AER_VALUE_PCONF_H
// vim: set ts=8 sw=4 et :

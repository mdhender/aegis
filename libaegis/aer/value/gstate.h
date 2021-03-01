//
//	aegis - project change supervisor
//	Copyright (C) 1994, 1996, 2002, 2005-2007 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
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

#ifndef AEGIS_AER_VALUE_GSTATE_H
#define AEGIS_AER_VALUE_GSTATE_H

#include <libaegis/aer/value.h>

/**
  * The gstate class is used to represet the list of available projects.
  * It may be indexed by project name to access the state of the
  * project.
  */
class rpt_value_gstate:
    public rpt_value
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_value_gstate();

private:
    /**
      * The default constructor.  It is private on purpose, use the
      * "create" class method instead.
      */
    rpt_value_gstate();

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      */
    static rpt_value::pointer create();

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

private:
    /**
      * The vkeys instance variable i sused to remember this list of keys
      * of the virtual array of projects.
      */
    mutable rpt_value::pointer vkeys;

    /**
      * The grab method is used to obtain and convert the meta-data.
      */
    void grab() const;

    /**
      * The copy constructor.  Do not use.
      */
    rpt_value_gstate(const rpt_value_gstate &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_value_gstate &operator=(const rpt_value_gstate &);
};

#endif // AEGIS_AER_VALUE_GSTATE_H

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

#ifndef AEGIS_AER_VALUE_GROUP_H
#define AEGIS_AER_VALUE_GROUP_H

#include <libaegis/aer/value.h>

/**
  * The rpt_value_group class is used to represent the /etc/group file,
  * presenting it as an array indexable by either group ID or group name.
  */
class rpt_value_group:
    public rpt_value
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_value_group();

private:
    /**
      * The default constructor.  It is private on purpose, use the
      * "create" class method instead.
      */
    rpt_value_group();

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
    bool is_a_struct() const;

    // See base class for documentation.
    rpt_value::pointer lookup(const rpt_value::pointer &rhs, bool) const;

    // See base class for documentation.
    rpt_value::pointer keys() const;

    // See base class for documentation.
    rpt_value::pointer count() const;

    // See base class for documentation.
    const char *type_of() const;

private:
    /**
      * The copy constructor.  Do not use.
      */
    rpt_value_group(const rpt_value_group &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_value_group &operator=(const rpt_value_group &);
};

#endif // AEGIS_AER_VALUE_GROUP_H

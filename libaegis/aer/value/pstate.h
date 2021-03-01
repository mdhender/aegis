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

#ifndef AEGIS_AER_VALUE_PSTATE_H
#define AEGIS_AER_VALUE_PSTATE_H

#include <common/nstring.h>
#include <libaegis/aer/value.h>

/**
  * The rpt_value_pstate class is used to represet the internal state
  * of a project.  The meta-data load is deferred, to avoid reading in
  * meta-data which is not actually used.
  */
class rpt_value_pstate:
    public rpt_value
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_value_pstate();

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * class method instead.
      *
      * @param pname
      *     The name of the project
      */
    rpt_value_pstate(const nstring &pname);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param pname
      *     The name of the project
      */
    static rpt_value::pointer create(const nstring &name);

    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param pname
      *     The name of the project
      */
    static rpt_value::pointer create(string_ty *name);

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
      * The pname instance variable is used to remember the name of the
      * project of interest.  This is used when un-defering the value.
      */
    nstring pname;

    /**
      * The value instance variable is used to remember the rpt_value_*
      * tree form of the project meta-data.  It is mutable because the
      * deferred load does not alter the semantic value of the object.
      */
    mutable rpt_value::pointer value;

    /**
      * The grab method is used to read in the meta-data and convert it.
      */
    void grab() const;

    /**
      * The default constructor.  Do not use.
      */
    rpt_value_pstate();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_value_pstate(const rpt_value_pstate &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_value_pstate &operator=(const rpt_value_pstate &);
};

#endif // AEGIS_AER_VALUE_PSTATE_H

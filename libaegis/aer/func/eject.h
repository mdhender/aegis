//
//	aegis - project change supervisor
//	Copyright (C) 1994, 2005-2007 Peter Miller.
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

#ifndef AEGIS_AER_FUNC_EJECT_H
#define AEGIS_AER_FUNC_EJECT_H

#include <libaegis/aer/func.h>


/**
  * The rpt_func_eject class is used to represent the eject
  * function, callable from within the report generator.
  */
class rpt_func_eject:
    public rpt_func
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_func_eject();

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * class method instead.
      */
    rpt_func_eject();

public:
    /**
      * The create class method is used to create a new dynamically
      * allocated instance of this class.
      */
    static rpt_func::pointer create();

protected:
    // See base class for documentation.
    const char *name() const;

    // See base class for documentation.
    bool optimizable() const;

    // See base class for documentation.
    bool verify(const rpt_expr::pointer &ep) const;

    // See base class for documentation.
    rpt_value::pointer run(const rpt_expr::pointer &ep, size_t argc,
        rpt_value::pointer *argv) const;

private:
    /**
      * The copy constructor.  Do not use.
      */
    rpt_func_eject(const rpt_func_eject &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_func_eject &operator=(const rpt_func_eject &);
};

#endif // AEGIS_AER_FUNC_EJECT_H

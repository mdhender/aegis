//
//	aegis - project change supervisor
//	Copyright (C) 1994, 2002, 2005-2008 Peter Miller.
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

#ifndef AEGIS_AER_EXPR_LIST_H
#define AEGIS_AER_EXPR_LIST_H

#include <libaegis/aer/expr.h>

/**
  * The rpt_expr_list class is used to represent a list expression node.
  */
class rpt_expr_list:
    public rpt_expr
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_expr_list();

private:
    /**
      * The default constructor.  It's private on purpose; use the
      * "create" class method instead.
      */
    rpt_expr_list();

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of list expression nodes.
      */
    static rpt_expr::pointer create();

protected:
    // See base class for documentation.
    const char *name() const;

    // See base class for documentation.
    rpt_value::pointer evaluate() const;

private:
    /**
      * The copy constructor.  Do not use.
      */
    rpt_expr_list(rpt_expr_list &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_expr_list &operator=(rpt_expr_list &);
};

#endif // AEGIS_AER_EXPR_LIST_H

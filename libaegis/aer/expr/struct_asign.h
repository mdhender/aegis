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

#ifndef AEGIS_AER_EXPR_STRUCT_ASIGN_H
#define AEGIS_AER_EXPR_STRUCT_ASIGN_H

#include <libaegis/aer/expr.h>

/**
  * The rpt_expr_struct_assign class is used to represent the state of
  * evaluating a struct expression.
  */
class rpt_expr_struct_assign:
    public rpt_expr
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_expr_struct_assign();

private:
    /**
      * The constructor.
      * It's private on purpose, use the "create" class method instead.
      *
      * @param name
      *     The name of the struct member
      * @param value
      *     The value of the struct member
      */
    rpt_expr_struct_assign(const nstring &name, const rpt_expr::pointer &value);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated isnatnces of this class.
      *
      * @param name
      *     The name of the struct member
      * @param value
      *     The value of the struct member
      */
    static rpt_expr::pointer create(const nstring &name,
        const rpt_expr::pointer &value);

protected:
    // See base class for documentation.
    const char *name() const;

    // See base class for documentation.
    rpt_value::pointer evaluate() const;

private:
    /**
      * The default constructor.  Do not use.
      */
    rpt_expr_struct_assign();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_expr_struct_assign(const rpt_expr_struct_assign &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_expr_struct_assign &operator=(const rpt_expr_struct_assign &);
};

#endif // AEGIS_AER_EXPR_STRUCT_ASIGN_H

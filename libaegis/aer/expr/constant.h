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

#ifndef AEGIS_AER_EXPR_CONSTANT_H
#define AEGIS_AER_EXPR_CONSTANT_H

#include <libaegis/aer/expr.h>

/**
  * The rpt_expr_constant class is used to represent expression nodes
  * which have a constant value.
  */
class rpt_expr_constant:
    public rpt_expr
{
public:
    /**
      * The destructor.
      */
    ~rpt_expr_constant();

private:
    /**
      * The constructor.  It is private because all users of the API are
      * required to create new instances via the create class method.
      *
      * @param value
      *     The value of this constant expression.
      */
    rpt_expr_constant(const rpt_value::pointer &value);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances.
      *
      * @param value
      *     The value of this constant expression.
      */
    static pointer create(const rpt_value::pointer &value);

protected:
    // See base class for documentation.
    bool lvalue() const;

    // See base class for documentation.
    const char *name() const;

    // See base class for documentation.
    rpt_value::pointer evaluate() const;

private:
    /**
      * The value instance variable
      * is used to remember
      */
    rpt_value::pointer value;

    /**
      * The default constructor.  Do not use.
      */
    rpt_expr_constant();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_expr_constant(const rpt_expr_constant &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_expr_constant &operator=(const rpt_expr_constant &);
};

#endif // AEGIS_AER_EXPR_CONSTANT_H

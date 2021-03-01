//
//	aegis - project change supervisor
//	Copyright (C) 1994, 2002, 2005-2007 Peter Miller.
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

#ifndef AEGIS_AER_EXPR_LOOKUP_H
#define AEGIS_AER_EXPR_LOOKUP_H

#include <common/nstring.h>
#include <libaegis/aer/expr.h>

/**
  * The rpt_expr_lookup class is used to represent array lookup syntax
  * tree expression nodes.
  */
class rpt_expr_lookup:
    public rpt_expr
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_expr_lookup();

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * class method instead.
      *
      * @param lhs
      *     The left hand expression, the array to index
      * @param rhs
      *     The right hand expression, the array index
      */
    rpt_expr_lookup(const rpt_expr::pointer &lhs, const rpt_expr::pointer &rhs);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param lhs
      *     The left hand expression, the array to index
      * @param rhs
      *     The right hand expression, the array index
      */
    static rpt_expr::pointer create(const rpt_expr::pointer &lhs,
        const rpt_expr::pointer &rhs);

    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param lhs
      *     The left hand expression, the structure to index
      * @param rhs
      *     The right hand expression, the member name
      */
    static rpt_expr::pointer create(const rpt_expr::pointer &lhs,
        const nstring &rhs);

protected:
    // See base class for documentation.
    bool lvalue() const;

    // See base class for documentation.
    rpt_value::pointer evaluate() const;

private:
    /**
      * The default constructor.  Do not use.
      */
    rpt_expr_lookup();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_expr_lookup(const rpt_expr_lookup &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_expr_lookup &operator=(const rpt_expr_lookup &);
};

#endif // AEGIS_AER_EXPR_LOOKUP_H

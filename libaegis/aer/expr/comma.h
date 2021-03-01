//
//	aegis - project change supervisor
//	Copyright (C) 2007, 2008 Peter Miller
//
//      This program is free software; you can redistribute it and/or
//      modify it under the terms of the GNU General Public License as
//      published by the Free Software Foundation; either version 3 of
//      the License, or (at your option) any later version.
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

#ifndef LIBAEGIS_AER_EXPR_COMMA_H
#define LIBAEGIS_AER_EXPR_COMMA_H

#include <libaegis/aer/expr.h>

/**
  * The rpt_expr_comma class is used to represent a comma expression
  * (left hand side discarded).
  */
class rpt_expr_comma:
    public rpt_expr
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_expr_comma();

private:
    /**
      * The constructor.
      * The public interface is via the "create" class method.
      *
      * @param lhs
      *     The left hand expression.
      * @param rhs
      *     The right hand expression.
      */
    rpt_expr_comma(const rpt_expr::pointer &lhs, const rpt_expr::pointer &rhs);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated comma expression nodes.
      *
      * @param lhs
      *     The left hand expression.
      * @param rhs
      *     The right hand expression.
      */
    static rpt_expr::pointer create(const rpt_expr::pointer &lhs,
        const rpt_expr::pointer &rhs);

protected:
    // See base class for documentation.
    const char *name() const;

    // See base class for documentation.
    rpt_value::pointer evaluate() const;

private:
    /**
      * The default constructor.  Do not use.
      */
    rpt_expr_comma();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_expr_comma(const rpt_expr_comma &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_expr_comma &operator=(const rpt_expr_comma &);
};

#endif // LIBAEGIS_AER_EXPR_COMMA_H

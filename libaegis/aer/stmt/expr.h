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

#ifndef AEGIS_AER_STMT_EXPR_H
#define AEGIS_AER_STMT_EXPR_H

#include <libaegis/aer/expr.h>
#include <libaegis/aer/stmt.h>


/**
  * The rpt_stmt_expr is used to represent a statement which consists of
  * a single expression (not much use if it doesn't have a side-effect, tho).
  */
class rpt_stmt_expr:
    public rpt_stmt
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_stmt_expr();

private:
    /**
      * The constructor.  It is private for a reason, use the "create"
      * class method instead.
      *
      * @param ep
      *     The expression in this node.
      */
    rpt_stmt_expr(const rpt_expr::pointer &ep);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param ep
      *     The expression in this node.
      */
    static pointer create(const rpt_expr::pointer &ep);

protected:
    // See base class for documentation.
    void run(rpt_stmt_result_ty *) const;

private:
    /**
      * The ep instance variable is used to remember the expression
      * syntax tree attached to this node of the statement syntax tree.
      */
    rpt_expr::pointer ep;

    /**
      * The default constructor.  Do not use.
      */
    rpt_stmt_expr();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_stmt_expr(const rpt_stmt_expr &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_stmt_expr &operator=(const rpt_stmt_expr &);
};

#endif // AEGIS_AER_STMT_EXPR_H

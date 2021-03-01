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

#ifndef AEGIS_AER_STMT_IF_H
#define AEGIS_AER_STMT_IF_H

#include <libaegis/aer/expr.h>
#include <libaegis/aer/stmt.h>

/**
  * The rpt_stmt_if class is used to represent "if" statement syntax
  * tree nodes.
  */
class rpt_stmt_if:
    public rpt_stmt
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_stmt_if();

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * class method.
      *
      * @param cond
      *     The condition to decide which clause
      * @param then_clause
      *     What to do if the condition is true
      * @param else_clause
      *     What to do if the condition is false
      */
    rpt_stmt_if(const rpt_expr::pointer &cond,
        const rpt_stmt::pointer &then_clause,
        const rpt_stmt::pointer &else_clause);

public:
    /**
      * The create class method is used to create a new synamically
      * allocated instance of this class.
      *
      * @param cond
      *     The condition to decide which clause
      * @param then_clause
      *     What to do if the condition is true
      * @param else_clause
      *     What to do if the condition is false
      */
    static rpt_stmt::pointer create(const rpt_expr::pointer &cond,
        const rpt_stmt::pointer &then_clause,
        const rpt_stmt::pointer &else_clause);

    /**
      * The create class method is used to create a new synamically
      * allocated instance of this class.
      *
      * @param cond
      *     The condition to decide which clause
      * @param then_clause
      *     What to do if the condition is true
      */
    static rpt_stmt::pointer create(const rpt_expr::pointer &cond,
        const rpt_stmt::pointer &then_clause);

protected:
    // See base class for documentation.
    void run(rpt_stmt_result_ty *) const;

private:
    /**
      * The condition instance variable is used to remember the
      * expression which controls which branch to execute.
      */
    rpt_expr::pointer condition;

    /**
      * The default constructor.  Do not use.
      */
    rpt_stmt_if();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_stmt_if(const rpt_stmt_if &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_stmt_if &operator=(const rpt_stmt_if &);
};

#endif // AEGIS_AER_STMT_IF_H

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

#ifndef AEGIS_AER_STMT_WHILE_H
#define AEGIS_AER_STMT_WHILE_H

#include <libaegis/aer/expr.h>
#include <libaegis/aer/stmt.h>

/**
  * The rpt_stmt_while class is used to repesent "while" statement
  * syntax tree nodes.
  */
class rpt_stmt_while:
    public rpt_stmt
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_stmt_while();

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * class method instead.
      *
      * @param ep
      *     The loop constrol expression
      * @param body
      *     The loop body
      */
    rpt_stmt_while(const rpt_expr::pointer &ep, const rpt_stmt::pointer &body);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param ep
      *     The loop constrol expression
      * @param body
      *     The loop body
      */
    static pointer create(const rpt_expr::pointer &ep,
        const rpt_stmt::pointer &body);

protected:
    // See base class for documentation.
    void run(rpt_stmt_result_ty *) const;

private:
    /**
      * The condition instance variable is used to remember the loop
      * control expression.
      */
    rpt_expr::pointer condition;

    /**
      * The default constructor.  Do not use.
      */
    rpt_stmt_while();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_stmt_while(const rpt_stmt_while &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_stmt_while &operator=(const rpt_stmt_while &);
};

/**
  * The rpt_stmt_do class is used to repesent "do" statement
  * syntax tree nodes.
  */
class rpt_stmt_do:
    public rpt_stmt
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_stmt_do();

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * class method instead.
      *
      * @param ep
      *     The loop constrol expression
      * @param body
      *     The loop body
      */
    rpt_stmt_do(const rpt_stmt::pointer &body, const rpt_expr::pointer &ep);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param ep
      *     The loop constrol expression
      * @param body
      *     The loop body
      */
    static pointer create(const rpt_stmt::pointer &body,
        const rpt_expr::pointer &ep);

protected:
    // See base class for documentation.
    void run(rpt_stmt_result_ty *) const;

private:
    /**
      * The condition instance variable is used to remember the loop
      * control expression.
      */
    rpt_expr::pointer condition;

    /**
      * The default constructor.  Do not use.
      */
    rpt_stmt_do();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_stmt_do(const rpt_stmt_do &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_stmt_do &operator=(const rpt_stmt_do &);
};

#endif // AEGIS_AER_STMT_WHILE_H

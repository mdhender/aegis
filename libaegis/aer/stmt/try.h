//
//	aegis - project change supervisor
//	Copyright (C) 1996, 2002, 2005-2008 Peter Miller
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

#ifndef AEGIS_AER_STMT_TRY_H
#define AEGIS_AER_STMT_TRY_H

#include <libaegis/aer/expr.h>
#include <libaegis/aer/stmt.h>

/**
  * The rpt_stmt_try class is used to represent "try" statament syntax
  * tree nodes.
  */
class rpt_stmt_try:
    public rpt_stmt
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_stmt_try();

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * class method instead.
      *
      * @param try_clause
      *     The statement to try and execute
      * @param variable
      *     The variable to receive the thrown exception
      * @param catch_clause
      *     The statement to execute if an exception is caught.
      */
    rpt_stmt_try(const rpt_stmt::pointer &try_clause,
        const rpt_expr::pointer &variable,
        const rpt_stmt::pointer &catch_clause);

public:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * class method instead.
      *
      * @param try_clause
      *     The statement to try and execute
      * @param variable
      *     The variable to receive the thrown exception
      * @param catch_clause
      *     The statement to execute if an exception is caught.
      */
    static pointer create(const rpt_stmt::pointer &try_clause,
        const rpt_expr::pointer &variable,
        const rpt_stmt::pointer &catch_clause);

protected:
    // see base class for documentation.
    void run(rpt_stmt_result_ty *) const;

private:
    /**
      * The "variabe" instance variable is sued to remember the variable
      * which will be assigned the exception value when it is thrown.
      */
    rpt_expr::pointer variable;

    /**
      * The default constructor.  Do not use.
      */
    rpt_stmt_try();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_stmt_try(const rpt_stmt_try &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_stmt_try &operator=(const rpt_stmt_try &);
};

#endif // AEGIS_AER_STMT_TRY_H

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

#ifndef AEGIS_AER_STMT_RETURN_H
#define AEGIS_AER_STMT_RETURN_H

#include <libaegis/aer/expr.h>
#include <libaegis/aer/stmt.h>

/**
  * The rpt_stmt_return class is used to represent an return statement
  * node of a syntax tree.
  */
class rpt_stmt_return:
    public rpt_stmt
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_stmt_return();

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * class method instead.
      *
      * @param ep
      *     The expression tree for calculating hte return value.
      */
    rpt_stmt_return(const rpt_expr::pointer &ep);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param ep
      *     The expression tree for calculating hte return value.
      */
    static pointer create(const rpt_expr::pointer &ep);

    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      */
    static pointer create();

protected:
    // See base class for documentation.
    void run(rpt_stmt_result_ty *) const;

private:
    /**
      * The ep instance variable is used to remember the expression node
      * syntax tree for calculating the return value.
      */
    rpt_expr::pointer ep;

    /**
      * The default constructor.  Do not use.
      */
    rpt_stmt_return();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_stmt_return(const rpt_stmt_return &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_stmt_return &operator=(const rpt_stmt_return &);
};

#endif // AEGIS_AER_STMT_RETURN_H

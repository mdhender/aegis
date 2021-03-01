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

#ifndef AEGIS_AER_STMT_THROW_H
#define AEGIS_AER_STMT_THROW_H

#include <libaegis/aer/expr.h>
#include <libaegis/aer/stmt.h>

/**
  * The rpt_stmt_throw class is used to represent a "throw" statement
  * syntax tree node.
  */
class rpt_stmt_throw:
    public rpt_stmt
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_stmt_throw();

private:
    /**
      * The constructor.  It's private on purpose, use the "create"
      * class method.
      *
      * @param ep
      *      The value of the exception to be thrown.
      */
    rpt_stmt_throw(const rpt_expr::pointer &ep);

public:
    /**
      * The create class method is used to creatre new aynamically
      * allocated instances of this class.
      *
      * @param ep
      *      The value of the exception to be thrown.
      */
    static rpt_stmt::pointer create(const rpt_expr::pointer &ep);

protected:
    // See base class for documentation.
    void run(rpt_stmt_result_ty *) const;

private:
    /**
      * The ep instance variable is used to remember the value of the
      * exception to be thrown.
      */
    rpt_expr::pointer ep;

    /**
      * The default constructor.  Do not use.
      */
    rpt_stmt_throw();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_stmt_throw(const rpt_stmt_throw &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_stmt_throw &operator=(const rpt_stmt_throw &);
};

#endif // AEGIS_AER_STMT_THROW_H

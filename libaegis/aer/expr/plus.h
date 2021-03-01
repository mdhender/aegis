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

#ifndef AEGIS_AER_EXPR_PLUS_H
#define AEGIS_AER_EXPR_PLUS_H

#include <libaegis/aer/expr.h>


/**
  * The rpt_expr_plus class is used to represent a syntax
  * tree addition expression node.
  */
class rpt_expr_plus:
    public rpt_expr
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_expr_plus();

private:
    /**
      * The constructor.  It's private on purpose, use the "create"
      * class method instead.
      */
    rpt_expr_plus(const rpt_expr::pointer &lhs,
        const rpt_expr::pointer &rhs);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instance of this class.
      */
    static rpt_expr::pointer create(const rpt_expr::pointer &lhs,
        const rpt_expr::pointer &rhs);

protected:
    // See base class for documentation.
    rpt_value::pointer evaluate() const;

private:
    /**
      * The default constructor.  Do not use.
      */
    rpt_expr_plus();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_expr_plus(const rpt_expr_plus &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_expr_plus &operator=(const rpt_expr_plus &);
};


/**
  * The rpt_expr_minus class is used to represent a syntax
  * tree subtraction expression node.
  */
class rpt_expr_minus:
    public rpt_expr
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_expr_minus();

private:
    /**
      * The constructor.  It's private on purpose, use the "create"
      * class method instead.
      */
    rpt_expr_minus(const rpt_expr::pointer &lhs,
        const rpt_expr::pointer &rhs);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instance of this class.
      */
    static rpt_expr::pointer create(const rpt_expr::pointer &lhs,
        const rpt_expr::pointer &rhs);

protected:
    // See base class for documentation.
    rpt_value::pointer evaluate() const;

private:
    /**
      * The default constructor.  Do not use.
      */
    rpt_expr_minus();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_expr_minus(const rpt_expr_minus &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_expr_minus &operator=(const rpt_expr_minus &);
};


/**
  * The rpt_expr_join class is used to represent a syntax
  * tree string join expression node.
  */
class rpt_expr_join:
    public rpt_expr
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_expr_join();

private:
    /**
      * The constructor.  It's private on purpose, use the "create"
      * class method instead.
      */
    rpt_expr_join(const rpt_expr::pointer &lhs,
        const rpt_expr::pointer &rhs);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instance of this class.
      */
    static rpt_expr::pointer create(const rpt_expr::pointer &lhs,
        const rpt_expr::pointer &rhs);

protected:
    // See base class for documentation.
    rpt_value::pointer evaluate() const;

private:
    /**
      * The default constructor.  Do not use.
      */
    rpt_expr_join();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_expr_join(const rpt_expr_join &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_expr_join &operator=(const rpt_expr_join &);
};

#endif // AEGIS_AER_EXPR_PLUS_H

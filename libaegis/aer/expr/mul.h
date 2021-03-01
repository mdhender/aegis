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

#ifndef AEGIS_AER_EXPR_MUL_H
#define AEGIS_AER_EXPR_MUL_H

#include <libaegis/aer/expr.h>


/**
  * The rpt_expr_mul class is used to represent a syntax
  * tree multiply expression node.
  */
class rpt_expr_mul:
    public rpt_expr
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_expr_mul();

private:
    /**
      * The constructor.  It's private on purpose, use the "create"
      * class method instead.
      */
    rpt_expr_mul(const rpt_expr::pointer &lhs,
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
    rpt_expr_mul();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_expr_mul(const rpt_expr_mul &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_expr_mul &operator=(const rpt_expr_mul &);
};


/**
  * The rpt_expr_div class is used to represent a syntax
  * tree division expression node.
  */
class rpt_expr_div:
    public rpt_expr
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_expr_div();

private:
    /**
      * The constructor.  It's private on purpose, use the "create"
      * class method instead.
      */
    rpt_expr_div(const rpt_expr::pointer &lhs,
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
    rpt_expr_div();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_expr_div(const rpt_expr_div &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_expr_div &operator=(const rpt_expr_div &);
};


/**
  * The rpt_expr_mod class is used to represent a syntax
  * tree modulo expression node.
  */
class rpt_expr_mod:
    public rpt_expr
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_expr_mod();

private:
    /**
      * The constructor.  It's private on purpose, use the "create"
      * class method instead.
      */
    rpt_expr_mod(const rpt_expr::pointer &lhs,
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
    rpt_expr_mod();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_expr_mod(const rpt_expr_mod &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_expr_mod &operator=(const rpt_expr_mod &);
};

#endif // AEGIS_AER_EXPR_MUL_H

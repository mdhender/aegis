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

#ifndef AEGIS_AER_EXPR_LOGICAL_H
#define AEGIS_AER_EXPR_LOGICAL_H

#include <libaegis/aer/expr.h>


/**
  * The rpt_expr_and_logical class is used to represent a syntax
  * tree logical AND expression node.
  */
class rpt_expr_and_logical:
    public rpt_expr
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_expr_and_logical();

private:
    /**
      * The constructor.  It's private on purpose, use the "create"
      * class method instead.
      */
    rpt_expr_and_logical(const rpt_expr::pointer &lhs,
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
    rpt_expr_and_logical();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_expr_and_logical(const rpt_expr_and_logical &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_expr_and_logical &operator=(const rpt_expr_and_logical &);
};


/**
  * The rpt_expr_or_logical class is used to represent a syntax
  * tree logical OR expression node.
  */
class rpt_expr_or_logical:
    public rpt_expr
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_expr_or_logical();

private:
    /**
      * The constructor.  It's private on purpose, use the "create"
      * class method instead.
      */
    rpt_expr_or_logical(const rpt_expr::pointer &lhs,
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
    rpt_expr_or_logical();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_expr_or_logical(const rpt_expr_or_logical &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_expr_or_logical &operator=(const rpt_expr_or_logical &);
};


/**
  * The rpt_expr_not_logical class is used to represent a syntax
  * tree logical NOT expression node.
  */
class rpt_expr_not_logical:
    public rpt_expr
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_expr_not_logical();

private:
    /**
      * The constructor.  It's private on purpose, use the "create"
      * class method instead.
      */
    rpt_expr_not_logical(const rpt_expr::pointer &arg);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instance of this class.
      */
    static rpt_expr::pointer create(const rpt_expr::pointer &arg);

protected:
    // See base class for documentation.
    rpt_value::pointer evaluate() const;

private:
    /**
      * The default constructor.  Do not use.
      */
    rpt_expr_not_logical();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_expr_not_logical(const rpt_expr_not_logical &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_expr_not_logical &operator=(const rpt_expr_not_logical &);
};


/**
  * The rpt_expr_if class is used to represent a syntax tree
  * ternary expression node.
  */
class rpt_expr_if:
    public rpt_expr
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_expr_if();

private:
    /**
      * The constructor.  It's private on purpose, use the "create"
      * class method instead.
      */
    rpt_expr_if(const rpt_expr::pointer &e1, const rpt_expr::pointer &e2,
        const rpt_expr::pointer &e3);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instance of this class.
      */
    static rpt_expr::pointer create(const rpt_expr::pointer &e1,
        const rpt_expr::pointer &e2, const rpt_expr::pointer &e3);

protected:
    // See base class for documentation.
    rpt_value::pointer evaluate() const;

private:
    /**
      * The default constructor.  Do not use.
      */
    rpt_expr_if();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_expr_if(const rpt_expr_if &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_expr_if &operator=(const rpt_expr_if &);
};

#endif // AEGIS_AER_EXPR_LOGICAL_H

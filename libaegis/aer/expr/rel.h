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

#ifndef AEGIS_AER_EXPR_REL_H
#define AEGIS_AER_EXPR_REL_H

#include <libaegis/aer/expr.h>


/**
  * The rpt_expr_lt class is used to represent a syntax
  * tree lt expression node.
  */
class rpt_expr_lt:
    public rpt_expr
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_expr_lt();

private:
    /**
      * The constructor.  It's private on purpose, use the "create"
      * class method instead.
      */
    rpt_expr_lt(const rpt_expr::pointer &lhs,
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
    rpt_expr_lt();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_expr_lt(const rpt_expr_lt &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_expr_lt &operator=(const rpt_expr_lt &);
};


/**
  * The rpt_expr_gt class is used to represent a syntax
  * tree gt expression node.
  */
class rpt_expr_gt:
    public rpt_expr
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_expr_gt();

private:
    /**
      * The constructor.  It's private on purpose, use the "create"
      * class method instead.
      */
    rpt_expr_gt(const rpt_expr::pointer &lhs,
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
    rpt_expr_gt();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_expr_gt(const rpt_expr_gt &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_expr_gt &operator=(const rpt_expr_gt &);
};


/**
  * The rpt_expr_le class is used to represent a syntax
  * tree le expression node.
  */
class rpt_expr_le:
    public rpt_expr
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_expr_le();

private:
    /**
      * The constructor.  It's private on purpose, use the "create"
      * class method instead.
      */
    rpt_expr_le(const rpt_expr::pointer &lhs,
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
    rpt_expr_le();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_expr_le(const rpt_expr_le &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_expr_le &operator=(const rpt_expr_le &);
};


/**
  * The rpt_expr_ge class is used to represent a syntax
  * tree ge expression node.
  */
class rpt_expr_ge:
    public rpt_expr
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_expr_ge();

private:
    /**
      * The constructor.  It's private on purpose, use the "create"
      * class method instead.
      */
    rpt_expr_ge(const rpt_expr::pointer &lhs,
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
    rpt_expr_ge();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_expr_ge(const rpt_expr_ge &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_expr_ge &operator=(const rpt_expr_ge &);
};


/**
  * The rpt_expr_eq class is used to represent a syntax
  * tree eq expression node.
  */
class rpt_expr_eq:
    public rpt_expr
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_expr_eq();

private:
    /**
      * The constructor.  It's private on purpose, use the "create"
      * class method instead.
      */
    rpt_expr_eq(const rpt_expr::pointer &lhs,
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
    rpt_expr_eq();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_expr_eq(const rpt_expr_eq &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_expr_eq &operator=(const rpt_expr_eq &);
};


/**
  * The rpt_expr_ne class is used to represent a syntax
  * tree ne expression node.
  */
class rpt_expr_ne:
    public rpt_expr
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_expr_ne();

private:
    /**
      * The constructor.  It's private on purpose, use the "create"
      * class method instead.
      */
    rpt_expr_ne(const rpt_expr::pointer &lhs,
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
    rpt_expr_ne();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_expr_ne(const rpt_expr_ne &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_expr_ne &operator=(const rpt_expr_ne &);
};


/**
  * The rpt_expr_match class is used to represent a syntax
  * tree match expression node.
  */
class rpt_expr_match:
    public rpt_expr
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_expr_match();

private:
    /**
      * The constructor.  It's private on purpose, use the "create"
      * class method instead.
      */
    rpt_expr_match(const rpt_expr::pointer &lhs,
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
    rpt_expr_match();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_expr_match(const rpt_expr_match &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_expr_match &operator=(const rpt_expr_match &);
};


/**
  * The rpt_expr_nmatch class is used to represent a syntax
  * tree nmatch expression node.
  */
class rpt_expr_nmatch:
    public rpt_expr
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_expr_nmatch();

private:
    /**
      * The constructor.  It's private on purpose, use the "create"
      * class method instead.
      */
    rpt_expr_nmatch(const rpt_expr::pointer &lhs,
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
    rpt_expr_nmatch();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_expr_nmatch(const rpt_expr_nmatch &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_expr_nmatch &operator=(const rpt_expr_nmatch &);
};

#endif // AEGIS_AER_EXPR_REL_H

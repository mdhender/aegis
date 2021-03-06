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

#ifndef AEGIS_AER_EXPR_INCR_H
#define AEGIS_AER_EXPR_INCR_H

#include <libaegis/aer/expr.h>


/**
  * The rpt_expr_inc_pre class is used to represent a syntax
  * tree pre-incremement expression node.
  */
class rpt_expr_inc_pre:
    public rpt_expr
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_expr_inc_pre();

private:
    /**
      * The constructor.  It's private on purpose, use the "create"
      * class method instead.
      */
    rpt_expr_inc_pre(const rpt_expr::pointer &arg);

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
    rpt_expr_inc_pre();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_expr_inc_pre(const rpt_expr_inc_pre &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_expr_inc_pre &operator=(const rpt_expr_inc_pre &);
};


/**
  * The rpt_expr_inc_post class is used to represent a syntax
  * tree post-increment expression node.
  */
class rpt_expr_inc_post:
    public rpt_expr
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_expr_inc_post();

private:
    /**
      * The constructor.  It's private on purpose, use the "create"
      * class method instead.
      */
    rpt_expr_inc_post(const rpt_expr::pointer &arg);

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
    rpt_expr_inc_post();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_expr_inc_post(const rpt_expr_inc_post &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_expr_inc_post &operator=(const rpt_expr_inc_post &);
};


/**
  * The rpt_expr_dec_pre class is used to represent a syntax
  * tree pre-decrement expression node.
  */
class rpt_expr_dec_pre:
    public rpt_expr
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_expr_dec_pre();

private:
    /**
      * The constructor.  It's private on purpose, use the "create"
      * class method instead.
      */
    rpt_expr_dec_pre(const rpt_expr::pointer &arg);

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
    rpt_expr_dec_pre();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_expr_dec_pre(const rpt_expr_dec_pre &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_expr_dec_pre &operator=(const rpt_expr_dec_pre &);
};


/**
  * The rpt_expr_dec_post class is used to represent a syntax
  * tree post-decrement expression node.
  */
class rpt_expr_dec_post:
    public rpt_expr
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_expr_dec_post();

private:
    /**
      * The constructor.  It's private on purpose, use the "create"
      * class method instead.
      */
    rpt_expr_dec_post(const rpt_expr::pointer &arg);

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
    rpt_expr_dec_post();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_expr_dec_post(const rpt_expr_dec_post &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_expr_dec_post &operator=(const rpt_expr_dec_post &);
};

#endif // AEGIS_AER_EXPR_INCR_H

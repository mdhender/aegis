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

#ifndef AEGIS_AER_EXPR_ASSIGN_H
#define AEGIS_AER_EXPR_ASSIGN_H

#include <libaegis/aer/expr.h>


/**
  * The rpt_expr_assign class is used to represent a syntax tree
  * assignment node.
  */
class rpt_expr_assign:
    public rpt_expr
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_expr_assign();

private:
    /**
      * The constructor.  It's private on purpose, use the "create"
      * class method instead.
      */
    rpt_expr_assign(const rpt_expr::pointer &lhs, const rpt_expr::pointer &rhs);

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
    rpt_expr_assign();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_expr_assign(const rpt_expr_assign &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_expr_assign &operator=(const rpt_expr_assign &);
};


/**
  * The rpt_expr_assign_power class is used to represent a syntax tree
  * power-and-assign node.
  */
class rpt_expr_assign_power:
    public rpt_expr
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_expr_assign_power();

private:
    /**
      * The constructor.  It's private on purpose, use the "create"
      * class method instead.
      */
    rpt_expr_assign_power(const rpt_expr::pointer &lhs,
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
    rpt_expr_assign_power();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_expr_assign_power(const rpt_expr_assign_power &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_expr_assign_power &operator=(const rpt_expr_assign_power &);
};


/**
  * The rpt_expr_assign_mul class is used to represent a syntax tree
  * multiply-and-assign node.
  */
class rpt_expr_assign_mul:
    public rpt_expr
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_expr_assign_mul();

private:
    /**
      * The constructor.  It's private on purpose, use the "create"
      * class method instead.
      */
    rpt_expr_assign_mul(const rpt_expr::pointer &lhs,
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
    rpt_expr_assign_mul();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_expr_assign_mul(const rpt_expr_assign_mul &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_expr_assign_mul &operator=(const rpt_expr_assign_mul &);
};


/**
  * The rpt_expr_assign_div class is used to represent a syntax tree
  * divide-and-assign node.
  */
class rpt_expr_assign_div:
    public rpt_expr
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_expr_assign_div();

private:
    /**
      * The constructor.  It's private on purpose, use the "create"
      * class method instead.
      */
    rpt_expr_assign_div(const rpt_expr::pointer &lhs,
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
    rpt_expr_assign_div();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_expr_assign_div(const rpt_expr_assign_div &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_expr_assign_div &operator=(const rpt_expr_assign_div &);
};


/**
  * The rpt_expr_assign_mod class is used to represent a syntax tree
  * modulo-and-assign node.
  */
class rpt_expr_assign_mod:
    public rpt_expr
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_expr_assign_mod();

private:
    /**
      * The constructor.  It's private on purpose, use the "create"
      * class method instead.
      */
    rpt_expr_assign_mod(const rpt_expr::pointer &lhs,
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
    rpt_expr_assign_mod();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_expr_assign_mod(const rpt_expr_assign_mod &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_expr_assign_mod &operator=(const rpt_expr_assign_mod &);
};


/**
  * The rpt_expr_assign_plus class is used to represent a syntax tree
  * add-and-assign node.
  */
class rpt_expr_assign_plus:
    public rpt_expr
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_expr_assign_plus();

private:
    /**
      * The constructor.  It's private on purpose, use the "create"
      * class method instead.
      */
    rpt_expr_assign_plus(const rpt_expr::pointer &lhs,
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
    rpt_expr_assign_plus();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_expr_assign_plus(const rpt_expr_assign_plus &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_expr_assign_plus &operator=(const rpt_expr_assign_plus &);
};


/**
  * The rpt_expr_assign_minus class is used to represent a syntax tree
  * subtract-and-assign node.
  */
class rpt_expr_assign_minus:
    public rpt_expr
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_expr_assign_minus();

private:
    /**
      * The constructor.  It's private on purpose, use the "create"
      * class method instead.
      */
    rpt_expr_assign_minus(const rpt_expr::pointer &lhs,
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
    rpt_expr_assign_minus();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_expr_assign_minus(const rpt_expr_assign_minus &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_expr_assign_minus &operator=(const rpt_expr_assign_minus &);
};


/**
  * The rpt_expr_assign_join class is used to represent a syntax tree
  * join-and-assign node.
  */
class rpt_expr_assign_join:
    public rpt_expr
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_expr_assign_join();

private:
    /**
      * The constructor.  It's private on purpose, use the "create"
      * class method instead.
      */
    rpt_expr_assign_join(const rpt_expr::pointer &lhs,
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
    rpt_expr_assign_join();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_expr_assign_join(const rpt_expr_assign_join &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_expr_assign_join &operator=(const rpt_expr_assign_join &);
};


/**
  * The rpt_expr_assign_and_bit class is used to represent a syntax tree
  * bitwise-and-then-assign node.
  */
class rpt_expr_assign_and_bit:
    public rpt_expr
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_expr_assign_and_bit();

private:
    /**
      * The constructor.  It's private on purpose, use the "create"
      * class method instead.
      */
    rpt_expr_assign_and_bit(const rpt_expr::pointer &lhs,
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
    rpt_expr_assign_and_bit();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_expr_assign_and_bit(const rpt_expr_assign_and_bit &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_expr_assign_and_bit &operator=(const rpt_expr_assign_and_bit &);
};


/**
  * The rpt_expr_assign_xor_bit class is used to represent a syntax tree
  * bitwise-xor-then-assign node.
  */
class rpt_expr_assign_xor_bit:
    public rpt_expr
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_expr_assign_xor_bit();

private:
    /**
      * The constructor.  It's private on purpose, use the "create"
      * class method instead.
      */
    rpt_expr_assign_xor_bit(const rpt_expr::pointer &lhs,
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
    rpt_expr_assign_xor_bit();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_expr_assign_xor_bit(const rpt_expr_assign_xor_bit &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_expr_assign_xor_bit &operator=(const rpt_expr_assign_xor_bit &);
};


/**
  * The rpt_expr_assign_or_bit class is used to represent a syntax tree
  * bitwise-or-then-assign node.
  */
class rpt_expr_assign_or_bit:
    public rpt_expr
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_expr_assign_or_bit();

private:
    /**
      * The constructor.  It's private on purpose, use the "create"
      * class method instead.
      */
    rpt_expr_assign_or_bit(const rpt_expr::pointer &lhs,
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
    rpt_expr_assign_or_bit();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_expr_assign_or_bit(const rpt_expr_assign_or_bit &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_expr_assign_or_bit &operator=(const rpt_expr_assign_or_bit &);
};


/**
  * The rpt_expr_assign_shift_left class is used to represent a syntax tree
  * shift-left-then-assign node.
  */
class rpt_expr_assign_shift_left:
    public rpt_expr
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_expr_assign_shift_left();

private:
    /**
      * The constructor.  It's private on purpose, use the "create"
      * class method instead.
      */
    rpt_expr_assign_shift_left(const rpt_expr::pointer &lhs,
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
    rpt_expr_assign_shift_left();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_expr_assign_shift_left(const rpt_expr_assign_shift_left &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_expr_assign_shift_left &operator=(const rpt_expr_assign_shift_left &);
};


/**
  * The rpt_expr_assign_shift_right class is used to represent a syntax tree
  * shift-right-then-assign node.
  */
class rpt_expr_assign_shift_right:
    public rpt_expr
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_expr_assign_shift_right();

private:
    /**
      * The constructor.  It's private on purpose, use the "create"
      * class method instead.
      */
    rpt_expr_assign_shift_right(const rpt_expr::pointer &lhs,
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
    rpt_expr_assign_shift_right();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_expr_assign_shift_right(const rpt_expr_assign_shift_right &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_expr_assign_shift_right &operator=(const rpt_expr_assign_shift_right &);
};

#endif // AEGIS_AER_EXPR_ASSIGN_H

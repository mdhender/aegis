//
//      aegis - project change supervisor
//      Copyright (C) 1997, 2002, 2005-2008, 2012 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#ifndef AEFIND_TREE_ARITHMETIC_H
#define AEFIND_TREE_ARITHMETIC_H

#include <aefind/tree/diadic.h>
#include <aefind/tree/monadic.h>

class tree_list; // forward

/**
  * The tree_divide class is used to represent an expression tree which
  * performs a division.
  */
class tree_divide:
    public tree_diadic
{
public:
    /**
      * The destructor.
      */
    virtual ~tree_divide();

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * clas smethod instead.
      *
      * @param left
      *     The left hand argument to this function.
      * @param right
      *     The right hand argument to this function.
      */
    tree_divide(const pointer &left, const pointer &right);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instance of this class.
      *
      * @param left
      *     The left hand argument to this function.
      * @param right
      *     The right hand argument to this function.
      */
    static pointer create(const pointer &left, const pointer &right);

    /**
      * The create_l class method is used to create new dynamically
      * allocated instance of this class.
      *
      * @param args
      *     The arguments to this function.
      */
    static pointer create_l(const tree_list &args);

protected:
    // See base class for documentation.
    const char *name() const;

    // See base class for documentation.
    rpt_value::pointer evaluate(string_ty *, string_ty *, string_ty *,
        struct stat *) const;

    // See base class for documentation.
    tree::pointer optimize() const;

private:
    /**
      * The default constructor.  Do not use.
      */
    tree_divide();

    /**
      * The copy constructor.  Do not use.
      */
    tree_divide(const tree_divide &);

    /**
      * The assignment operator.  Do not use.
      */
    tree_divide &operator=(const tree_divide &);
};


/**
  * The tree_join class is used to represent an expression tree which
  * performs a string join.
  */
class tree_join:
    public tree_diadic
{
public:
    /**
      * The destructor.
      */
    virtual ~tree_join();

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * clas smethod instead.
      *
      * @param left
      *     The left hand argument to this function.
      * @param right
      *     The right hand argument to this function.
      */
    tree_join(const pointer &left, const pointer &right);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instance of this class.
      *
      * @param left
      *     The left hand argument to this function.
      * @param right
      *     The right hand argument to this function.
      */
    static pointer create(const pointer &left, const pointer &right);

    /**
      * The create_l class method is used to create new dynamically
      * allocated instance of this class.
      *
      * @param args
      *     The arguments to this function.
      */
    static pointer create_l(const tree_list &args);

protected:
    // See base class for documentation.
    const char *name() const;

    // See base class for documentation.
    rpt_value::pointer evaluate(string_ty *, string_ty *, string_ty *,
        struct stat *) const;

    // See base class for documentation.
    tree::pointer optimize() const;

private:
    /**
      * The default constructor.  Do not use.
      */
    tree_join();

    /**
      * The copy constructor.  Do not use.
      */
    tree_join(const tree_join &);

    /**
      * The assignment operator.  Do not use.
      */
    tree_join &operator=(const tree_join &);
};


/**
  * The tree_subtract class is used to represent an expression tree which
  * performs a subtraction.
  */
class tree_subtract:
    public tree_diadic
{
public:
    /**
      * The destructor.
      */
    virtual ~tree_subtract();

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * clas smethod instead.
      *
      * @param left
      *     The left hand argument to this function.
      * @param right
      *     The right hand argument to this function.
      */
    tree_subtract(const pointer &left, const pointer &right);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instance of this class.
      *
      * @param left
      *     The left hand argument to this function.
      * @param right
      *     The right hand argument to this function.
      */
    static pointer create(const pointer &left, const pointer &right);

    /**
      * The create_l class method is used to create new dynamically
      * allocated instance of this class.
      *
      * @param args
      *     The arguments to this function.
      */
    static pointer create_l(const tree_list &args);

protected:
    // See base class for documentation.
    const char *name() const;

    // See base class for documentation.
    rpt_value::pointer evaluate(string_ty *, string_ty *, string_ty *,
        struct stat *) const;

    // See base class for documentation.
    tree::pointer optimize() const;

private:
    /**
      * The default constructor.  Do not use.
      */
    tree_subtract();

    /**
      * The copy constructor.  Do not use.
      */
    tree_subtract(const tree_subtract &);

    /**
      * The assignment operator.  Do not use.
      */
    tree_subtract &operator=(const tree_subtract &);
};


/**
  * The tree_mod class is used to represent an expression tree which
  * performs a modulo (remainder) operation.
  */
class tree_mod:
    public tree_diadic
{
public:
    /**
      * The destructor.
      */
    virtual ~tree_mod();

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * clas smethod instead.
      *
      * @param left
      *     The left hand argument to this function.
      * @param right
      *     The right hand argument to this function.
      */
    tree_mod(const pointer &left, const pointer &right);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instance of this class.
      *
      * @param left
      *     The left hand argument to this function.
      * @param right
      *     The right hand argument to this function.
      */
    static pointer create(const pointer &left, const pointer &right);

    /**
      * The create_l class method is used to create new dynamically
      * allocated instance of this class.
      *
      * @param args
      *     The arguments to this function.
      */
    static pointer create_l(const tree_list &args);

protected:
    // See base class for documentation.
    const char *name() const;

    // See base class for documentation.
    rpt_value::pointer evaluate(string_ty *, string_ty *, string_ty *,
        struct stat *) const;

    // See base class for documentation.
    tree::pointer optimize() const;

private:
    /**
      * The default constructor.  Do not use.
      */
    tree_mod();

    /**
      * The copy constructor.  Do not use.
      */
    tree_mod(const tree_mod &);

    /**
      * The assignment operator.  Do not use.
      */
    tree_mod &operator=(const tree_mod &);
};


/**
  * The tree_mul class is used to represent an expression tree which
  * performs a multiplication.
  */
class tree_mul:
    public tree_diadic
{
public:
    /**
      * The destructor.
      */
    virtual ~tree_mul();

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * clas smethod instead.
      *
      * @param left
      *     The left hand argument to this function.
      * @param right
      *     The right hand argument to this function.
      */
    tree_mul(const pointer &left, const pointer &right);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instance of this class.
      *
      * @param left
      *     The left hand argument to this function.
      * @param right
      *     The right hand argument to this function.
      */
    static pointer create(const pointer &left, const pointer &right);

    /**
      * The create_l class method is used to create new dynamically
      * allocated instance of this class.
      *
      * @param args
      *     The arguments to this function.
      */
    static pointer create_l(const tree_list &args);

protected:
    // See base class for documentation.
    const char *name() const;

    // See base class for documentation.
    rpt_value::pointer evaluate(string_ty *, string_ty *, string_ty *,
        struct stat *) const;

    // See base class for documentation.
    tree::pointer optimize() const;

private:
    /**
      * The default constructor.  Do not use.
      */
    tree_mul();

    /**
      * The copy constructor.  Do not use.
      */
    tree_mul(const tree_mul &);

    /**
      * The assignment operator.  Do not use.
      */
    tree_mul &operator=(const tree_mul &);
};


/**
  * The tree_neg class is used to represent an expression tree which
  * evatuates to teh negative of its argument.
  */
class tree_neg:
    public tree_monadic
{
public:
    /**
      * The destructor.
      */
    virtual ~tree_neg();

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * clas smethod instead.
      */
    tree_neg(const pointer &arg);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instance of this class.
      *
      * @param arg
      *     The singel argument to this function.
      */
    static pointer create(const pointer &arg);

    /**
      * The create_l class method is used to create new dynamically
      * allocated instance of this class.
      *
      * @param args
      *     The arguments to this function.
      */
    static pointer create_l(const tree_list &args);

protected:
    // See base class for documentation.
    const char *name() const;

    // See base class for documentation.
    rpt_value::pointer evaluate(string_ty *, string_ty *, string_ty *,
        struct stat *) const;

    // See base class for documentation.
    tree::pointer optimize() const;

private:
    /**
      * The default constructor.  Do not use.
      */
    tree_neg();

    /**
      * The copy constructor.  Do not use.
      */
    tree_neg(const tree_neg &);

    /**
      * The assignment operator.  Do not use.
      */
    tree_neg &operator=(const tree_neg &);
};


/**
  * The tree_plus class is used to represent an expression tree which
  * performs a FUBAR
  */
class tree_plus:
    public tree_diadic
{
public:
    /**
      * The destructor.
      */
    virtual ~tree_plus();

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * clas smethod instead.
      *
      * @param left
      *     The left hand argument to this function.
      * @param right
      *     The right hand argument to this function.
      */
    tree_plus(const pointer &left, const pointer &right);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instance of this class.
      *
      * @param left
      *     The left hand argument to this function.
      * @param right
      *     The right hand argument to this function.
      */
    static pointer create(const pointer &left, const pointer &right);

    /**
      * The create_l class method is used to create new dynamically
      * allocated instance of this class.
      *
      * @param args
      *     The arguments to this function.
      */
    static pointer create_l(const tree_list &args);

protected:
    // See base class for documentation.
    const char *name() const;

    // See base class for documentation.
    rpt_value::pointer evaluate(string_ty *, string_ty *, string_ty *,
        struct stat *) const;

    // See base class for documentation.
    tree::pointer optimize() const;

private:
    /**
      * The default constructor.  Do not use.
      */
    tree_plus();

    /**
      * The copy constructor.  Do not use.
      */
    tree_plus(const tree_plus &);

    /**
      * The assignment operator.  Do not use.
      */
    tree_plus &operator=(const tree_plus &);
};

/**
  * The tree_pos class is used to represent an expression tree which
  * coreces its arguemnt to be an arithmetic type.
  */
class tree_pos:
    public tree_monadic
{
public:
    /**
      * The destructor.
      */
    virtual ~tree_pos();

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * clas smethod instead.
      */
    tree_pos(const pointer &arg);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instance of this class.
      *
      * @param arg
      *     The singel argument to this function.
      */
    static pointer create(const pointer &arg);

    /**
      * The create_l class method is used to create new dynamically
      * allocated instance of this class.
      *
      * @param args
      *     The arguments to this function.
      */
    static pointer create_l(const tree_list &args);

protected:
    // See base class for documentation.
    const char *name() const;

    // See base class for documentation.
    rpt_value::pointer evaluate(string_ty *, string_ty *, string_ty *,
        struct stat *) const;

    // See base class for documentation.
    tree::pointer optimize() const;

private:
    /**
      * The default constructor.  Do not use.
      */
    tree_pos();

    /**
      * The copy constructor.  Do not use.
      */
    tree_pos(const tree_pos &);

    /**
      * The assignment operator.  Do not use.
      */
    tree_pos &operator=(const tree_pos &);
};

#endif // AEFIND_TREE_ARITHMETIC_H
// vim: set ts=8 sw=4 et :

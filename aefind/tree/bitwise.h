//
//	aegis - project change supervisor
//	Copyright (C) 1997, 2002, 2005-2008 Peter Miller
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

#ifndef AEFIND_TREE_BITWISE_H
#define AEFIND_TREE_BITWISE_H

#include <aefind/tree/diadic.h>
#include <aefind/tree/monadic.h>

class tree_list; // forward

/**
  * The tree_bitwise_and class is used to represent an expression tree which
  * performs a FUBAR
  */
class tree_bitwise_and:
    public tree_diadic
{
public:
    /**
      * The destructor.
      */
    virtual ~tree_bitwise_and();

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
    tree_bitwise_and(const pointer &left, const pointer &right);

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
    static pointer create_l(const tree_list &arg);

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
    tree_bitwise_and();

    /**
      * The copy constructor.  Do not use.
      */
    tree_bitwise_and(const tree_bitwise_and &);

    /**
      * The assignment operator.  Do not use.
      */
    tree_bitwise_and &operator=(const tree_bitwise_and &);
};


/**
  * The tree_bitwise_or class is used to represent an expression tree which
  * performs a FUBAR
  */
class tree_bitwise_or:
    public tree_diadic
{
public:
    /**
      * The destructor.
      */
    virtual ~tree_bitwise_or();

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
    tree_bitwise_or(const pointer &left, const pointer &right);

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
    static pointer create_l(const tree_list &arg);

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
    tree_bitwise_or();

    /**
      * The copy constructor.  Do not use.
      */
    tree_bitwise_or(const tree_bitwise_or &);

    /**
      * The assignment operator.  Do not use.
      */
    tree_bitwise_or &operator=(const tree_bitwise_or &);
};


/**
  * The tree_bitwise_xor class is used to represent an expression tree which
  * performs a FUBAR
  */
class tree_bitwise_xor:
    public tree_diadic
{
public:
    /**
      * The destructor.
      */
    virtual ~tree_bitwise_xor();

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
    tree_bitwise_xor(const pointer &left, const pointer &right);

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
    static pointer create_l(const tree_list &arg);

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
    tree_bitwise_xor();

    /**
      * The copy constructor.  Do not use.
      */
    tree_bitwise_xor(const tree_bitwise_xor &);

    /**
      * The assignment operator.  Do not use.
      */
    tree_bitwise_xor &operator=(const tree_bitwise_xor &);
};


/**
  * The tree_bitwise_not class is used to represent an expression tree which
  * evatuates to FUBAR
  */
class tree_bitwise_not:
    public tree_monadic
{
public:
    /**
      * The destructor.
      */
    virtual ~tree_bitwise_not();

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * clas smethod instead.
      */
    tree_bitwise_not(const pointer &arg);

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
    static pointer create_l(const tree_list &arg);

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
    tree_bitwise_not();

    /**
      * The copy constructor.  Do not use.
      */
    tree_bitwise_not(const tree_bitwise_not &);

    /**
      * The assignment operator.  Do not use.
      */
    tree_bitwise_not &operator=(const tree_bitwise_not &);
};


/**
  * The tree_shift_left class is used to represent an expression tree which
  * performs a FUBAR
  */
class tree_shift_left:
    public tree_diadic
{
public:
    /**
      * The destructor.
      */
    virtual ~tree_shift_left();

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
    tree_shift_left(const pointer &left, const pointer &right);

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
    static pointer create_l(const tree_list &arg);

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
    tree_shift_left();

    /**
      * The copy constructor.  Do not use.
      */
    tree_shift_left(const tree_shift_left &);

    /**
      * The assignment operator.  Do not use.
      */
    tree_shift_left &operator=(const tree_shift_left &);
};


/**
  * The tree_shift_right class is used to represent an expression tree which
  * performs a FUBAR
  */
class tree_shift_right:
    public tree_diadic
{
public:
    /**
      * The destructor.
      */
    virtual ~tree_shift_right();

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
    tree_shift_right(const pointer &left, const pointer &right);

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
    static pointer create_l(const tree_list &arg);

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
    tree_shift_right();

    /**
      * The copy constructor.  Do not use.
      */
    tree_shift_right(const tree_shift_right &);

    /**
      * The assignment operator.  Do not use.
      */
    tree_shift_right &operator=(const tree_shift_right &);
};

#endif // AEFIND_TREE_BITWISE_H

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

#ifndef AEFIND_TREE_LOGICAL_H
#define AEFIND_TREE_LOGICAL_H

#include <aefind/tree/diadic.h>
#include <aefind/tree/monadic.h>

class tree_list; // forward

/**
  * The tree_and class is used to represent an expression tree which
  * performs a logical AND.
  */
class tree_and:
    public tree_diadic
{
public:
    /**
      * The destructor.
      */
    virtual ~tree_and();

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
    tree_and(const pointer &left, const pointer &right);

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
    tree_and();

    /**
      * The copy constructor.  Do not use.
      */
    tree_and(const tree_and &);

    /**
      * The assignment operator.  Do not use.
      */
    tree_and &operator=(const tree_and &);
};


/**
  * The tree_or class is used to represent an expression tree which
  * performs a logical OR.
  */
class tree_or:
    public tree_diadic
{
public:
    /**
      * The destructor.
      */
    virtual ~tree_or();

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
    tree_or(const pointer &left, const pointer &right);

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
    tree_or();

    /**
      * The copy constructor.  Do not use.
      */
    tree_or(const tree_or &);

    /**
      * The assignment operator.  Do not use.
      */
    tree_or &operator=(const tree_or &);
};


/**
  * The tree_not class is used to represent an expression tree which
  * evatuates to FUBAR
  */
class tree_not:
    public tree_monadic
{
public:
    /**
      * The destructor.
      */
    virtual ~tree_not();

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * clas smethod instead.
      */
    tree_not(const pointer &arg);

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
    tree_not();

    /**
      * The copy constructor.  Do not use.
      */
    tree_not(const tree_not &);

    /**
      * The assignment operator.  Do not use.
      */
    tree_not &operator=(const tree_not &);
};


/**
  * The tree_comma class is used to represent an expression tree which
  * performs a "comma" operator (discards the left value).
  */
class tree_comma:
    public tree_diadic
{
public:
    /**
      * The destructor.
      */
    virtual ~tree_comma();

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
    tree_comma(const pointer &left, const pointer &right);

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
    tree_comma();

    /**
      * The copy constructor.  Do not use.
      */
    tree_comma(const tree_comma &);

    /**
      * The assignment operator.  Do not use.
      */
    tree_comma &operator=(const tree_comma &);
};


/**
  * The tree_triadic class is used to represent an expression tree which
  * performs a (a ? b : c) operation.
  */
class tree_triadic:
    public tree
{
public:
    /**
      * The destructor.
      */
    virtual ~tree_triadic();

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * clas smethod instead.
      *
      * @param a1
      *     The first argument to this function.
      * @param a2
      *     The second argument to this function.
      * @param a3
      *     The third argument to this function.
      */
    tree_triadic(const pointer &a1, const pointer &a2, const pointer &a3);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instance of this class.
      *
      * @param a1
      *     The first argument to this function.
      * @param a2
      *     The second argument to this function.
      * @param a3
      *     The third argument to this function.
      */
    static pointer create(const pointer &a1, const pointer &a2,
        const pointer &a3);

protected:
    // See base class for documentation.
    const char *name() const;

    // See base class for documentation.
    rpt_value::pointer evaluate(string_ty *, string_ty *, string_ty *,
        struct stat *) const;

    // See base class for documentation.
    tree::pointer optimize() const;

    // See base class for documentation.
    void print() const;

    // See base class for documentation.
    bool useful() const;

    // See base class for documentation.
    bool constant() const;

private:
    tree::pointer a1;
    tree::pointer a2;
    tree::pointer a3;

    /**
      * The default constructor.  Do not use.
      */
    tree_triadic();

    /**
      * The copy constructor.  Do not use.
      */
    tree_triadic(const tree_triadic &);

    /**
      * The assignment operator.  Do not use.
      */
    tree_triadic &operator=(const tree_triadic &);
};

#endif // AEFIND_TREE_LOGICAL_H
// vim: set ts=8 sw=4 et :

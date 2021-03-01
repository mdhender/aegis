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

#ifndef AEFIND_TREE_RELATIVE_H
#define AEFIND_TREE_RELATIVE_H

#include <aefind/tree/diadic.h>

class tree_list; // forward

/**
  * The tree_ge class is used to represent an expression tree which
  * performs a greater than or equal comparison.
  */
class tree_ge:
    public tree_diadic
{
public:
    /**
      * The destructor.
      */
    virtual ~tree_ge();

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
    tree_ge(const pointer &left, const pointer &right);

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
    tree_ge();

    /**
      * The copy constructor.  Do not use.
      */
    tree_ge(const tree_ge &);

    /**
      * The assignment operator.  Do not use.
      */
    tree_ge &operator=(const tree_ge &);
};


/**
  * The tree_gt class is used to represent an expression tree which
  * performs a greater than comparison.
  */
class tree_gt:
    public tree_diadic
{
public:
    /**
      * The destructor.
      */
    virtual ~tree_gt();

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
    tree_gt(const pointer &left, const pointer &right);

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
    tree_gt();

    /**
      * The copy constructor.  Do not use.
      */
    tree_gt(const tree_gt &);

    /**
      * The assignment operator.  Do not use.
      */
    tree_gt &operator=(const tree_gt &);
};


/**
  * The tree_le class is used to represent an expression tree which
  * performs a less than or equal comparison.
  */
class tree_le:
    public tree_diadic
{
public:
    /**
      * The destructor.
      */
    virtual ~tree_le();

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
    tree_le(const pointer &left, const pointer &right);

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
    tree_le();

    /**
      * The copy constructor.  Do not use.
      */
    tree_le(const tree_le &);

    /**
      * The assignment operator.  Do not use.
      */
    tree_le &operator=(const tree_le &);
};


/**
  * The tree_lt class is used to represent an expression tree which
  * performs a less than comparison.
  */
class tree_lt:
    public tree_diadic
{
public:
    /**
      * The destructor.
      */
    virtual ~tree_lt();

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
    tree_lt(const pointer &left, const pointer &right);

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
    tree_lt();

    /**
      * The copy constructor.  Do not use.
      */
    tree_lt(const tree_lt &);

    /**
      * The assignment operator.  Do not use.
      */
    tree_lt &operator=(const tree_lt &);
};


/**
  * The tree_eq class is used to represent an expression tree which
  * performs an equal to comnparison.
  */
class tree_eq:
    public tree_diadic
{
public:
    /**
      * The destructor.
      */
    virtual ~tree_eq();

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
    tree_eq(const pointer &left, const pointer &right);

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
    tree_eq();

    /**
      * The copy constructor.  Do not use.
      */
    tree_eq(const tree_eq &);

    /**
      * The assignment operator.  Do not use.
      */
    tree_eq &operator=(const tree_eq &);
};


/**
  * The tree_ne class is used to represent an expression tree which
  * performs a not equal to comparison.
  */
class tree_ne:
    public tree_diadic
{
public:
    /**
      * The destructor.
      */
    virtual ~tree_ne();

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
    tree_ne(const pointer &left, const pointer &right);

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
    tree_ne();

    /**
      * The copy constructor.  Do not use.
      */
    tree_ne(const tree_ne &);

    /**
      * The assignment operator.  Do not use.
      */
    tree_ne &operator=(const tree_ne &);
};

#endif // AEFIND_TREE_RELATIVE_H

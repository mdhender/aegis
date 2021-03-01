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

#ifndef AEGIS_AER_STMT_FOR_H
#define AEGIS_AER_STMT_FOR_H

#include <libaegis/aer/expr.h>
#include <libaegis/aer/stmt.h>

/**
  * The rpt_stmt_for class is used to represent a "for" loop statement
  * syntax tree node.
  */
class rpt_stmt_for:
    public rpt_stmt
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_stmt_for();

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * class method instead.
      *
      * @param a
      *     The loop initialization expression
      * @param b
      *     The loop text expression
      * @param c
      *     The loop advance expression
      * @param body
      *     The loop body ststement tree.
      */
    rpt_stmt_for(const rpt_expr::pointer &a, const rpt_expr::pointer &b,
        const rpt_expr::pointer &c, const rpt_stmt::pointer &body);

public:
    /**
      * The create class method is used to create a new dynamically
      * allocate instance of this class.
      *
      * @param a
      *     The loop initialization expression
      * @param b
      *     The loop text expression
      * @param c
      *     The loop advance expression
      * @param body
      *     The loop body ststement tree.
      */
    static rpt_stmt::pointer create(const rpt_expr::pointer &a,
        const rpt_expr::pointer &b, const rpt_expr::pointer &c,
        const rpt_stmt::pointer &body);

protected:
    // see base class for documentation.
    void run(rpt_stmt_result_ty *) const;

private:
    /**
      * The e instance variable is used to remember the three
      * expressions used to control the loop.
      */
    rpt_expr::pointer e[3];

    /**
      * The default constructor.  Do not use.
      */
    rpt_stmt_for();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_stmt_for(const rpt_stmt_for &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_stmt_for &operator=(const rpt_stmt_for &);
};


/**
  * The rpt_stmt_foreach class is sued to represent a "for a in b" loop
  * statement syntax tree node.
  */
class rpt_stmt_foreach:
    public rpt_stmt
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_stmt_foreach();

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * class method instead.
      *
      * @param a
      *     The loop variable expression
      * @param b
      *     The loop list expression
      * @param body
      *     The loop body ststement tree.
      */
    rpt_stmt_foreach(const rpt_expr::pointer &a, const rpt_expr::pointer &b,
        const rpt_stmt::pointer &body);

public:
    /**
      * The create class method is used to create a new dynamically
      * allocate instance of this class.
      *
      * @param a
      *     The loop variable expression
      * @param b
      *     The loop list expression
      * @param body
      *     The loop body ststement tree.
      */
    static rpt_stmt::pointer create(const rpt_expr::pointer &a,
        const rpt_expr::pointer &b, const rpt_stmt::pointer &body);

protected:
    // see base class for documentation.
    void run(rpt_stmt_result_ty *) const;

private:
    /**
      * The e environment variable is used to remember the
      * expressions used to control the loop.
      */
    rpt_expr::pointer e[2];

    /**
      * The default constructor.  Do not use.
      */
    rpt_stmt_foreach();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_stmt_foreach(const rpt_stmt_foreach &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_stmt_foreach &operator=(const rpt_stmt_foreach &);
};


/**
  * The rpt_stmt_break class is sued to represent a "break" statement
  * syntax tree node.
  */
class rpt_stmt_break:
    public rpt_stmt
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_stmt_break();

private:
    /**
      * The default constructor.  It is private on purpose, use the
      * "create" class method instead.
      */
    rpt_stmt_break();

public:
    /**
      * The create class method is used to create a new dynamically
      * allocate instance of this class.
      */
    static rpt_stmt::pointer create();

protected:
    // see base class for documentation.
    void run(rpt_stmt_result_ty *) const;

private:
    /**
      * The copy constructor.  Do not use.
      */
    rpt_stmt_break(const rpt_stmt_break &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_stmt_break &operator=(const rpt_stmt_break &);
};


/**
  * The rpt_stmt_continue class is sued to represent a "continue"
  * statement syntax tree node.
  */
class rpt_stmt_continue:
    public rpt_stmt
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_stmt_continue();

private:
    /**
      * The default constructor.  It is private on purpose, use the
      * "create" class method instead.
      */
    rpt_stmt_continue();

public:
    /**
      * The create class method is used to create a new dynamically
      * allocate instance of this class.
      */
    static rpt_stmt::pointer create();

protected:
    // see base class for documentation.
    void run(rpt_stmt_result_ty *) const;

private:
    /**
      * The copy constructor.  Do not use.
      */
    rpt_stmt_continue(const rpt_stmt_continue &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_stmt_continue &operator=(const rpt_stmt_continue &);
};

#endif // AEGIS_AER_STMT_FOR_H

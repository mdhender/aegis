//
//	aegis - project change supervisor
//	Copyright (C) 1994, 1996, 2002-2008 Peter Miller
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

#ifndef AEGIS_AER_STMT_H
#define AEGIS_AER_STMT_H

#include <common/ac/shared_ptr.h>
#include <common/ac/stddef.h>

#include <libaegis/aer/value.h>

enum rpt_stmt_status_ty
{
    rpt_stmt_status_normal,
    rpt_stmt_status_break,
    rpt_stmt_status_continue,
    rpt_stmt_status_return,
    rpt_stmt_status_error
};

struct rpt_stmt_result_ty
{
    rpt_stmt_status_ty status;
    rpt_value::pointer thrown;
};


/**
  * The rpt_stmt abstract base class is used to represent a generic
  * statement in the statement tree used by the report generator.
  */
class rpt_stmt
{
public:
    typedef aegis_shared_ptr<rpt_stmt> pointer;

    /**
      * The destructor.
      */
    virtual ~rpt_stmt();

protected:
    /**
      * The default constructor.
      */
    rpt_stmt();

public:
    /**
      * The run method is used to execute the statament.
      */
    virtual void run(rpt_stmt_result_ty *) const = 0;

    /**
      * The append method is used to append another statement node to
      * the children of this statement node.
      */
    void append(const pointer &child);

    /**
      * The prepend method is used to prepend another statement node to
      * the children of this statement node.
      */
    void prepend(const pointer &child);

    /**
      * The get_nchildren method may be used to obtain the number of
      * child nodes this node has.
      */
    size_t get_nchildren() const { return nchild; }

protected:
    /**
      * The nth_child method may be used to obtain the given child
      * statement.
      *
      * @param n
      *     The shild to obtain, zero based.
      * @returns
      *     pointer to statement node, or NULL if too high
      */
    pointer nth_child(size_t n) const;

private:
    /**
      * The child instance variable is used to remember the base of a
      * dynamically allocated array of pointers to statement nodes,
      * each a child of this node.
      */
    pointer *child;

    /**
      * The nchild instance variable is used to remember how many
      * elements of the "child" array have been used to date.
      */
    size_t nchild;

    /**
      * The nchild_max instance variable is used to remember how many
      * elements of the "child" array were allocated.  This is used to
      * know when to allocate more.
      */
    size_t nchild_max;

    /**
      * The copy constructor.  Do not use.
      */
    rpt_stmt(const rpt_stmt &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_stmt &operator=(const rpt_stmt &);
};

#endif // AEGIS_AER_STMT_H

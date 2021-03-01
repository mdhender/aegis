//
//      aegis - project change supervisor
//      Copyright (C) 1994-1996, 2002-2008 Peter Miller
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

#ifndef AEGIS_AER_EXPR_H
#define AEGIS_AER_EXPR_H

#include <common/ac/shared_ptr.h>
#include <common/ac/stddef.h>

#include <libaegis/aer/pos.h>
#include <libaegis/aer/value.h>


/**
  * The rpt_expr abstract base class is used to represent a generic
  * expressson node in the report generator.
  */
class rpt_expr
{
public:
    typedef aegis_shared_ptr<rpt_expr> pointer;

    /**
      * The destructor.
      */
    virtual ~rpt_expr();

protected:
    /**
      * The default constructor.
      */
    rpt_expr();

public:
    /**
      * The parse_error method is used to report parse errors, reported
      * against the location of this expression node.
      */
    void parse_error(const char *fmt) const;

    /**
      * The append method is used to append a sub-mode to the end of the
      * list of child nodes of this expression node.
      */
    void append(const rpt_expr::pointer &child);

    /**
      * The prepend method is used to prepend a sub-mode to the front of
      * the list of child nodes of this expression node.
      */
    void prepend(const rpt_expr::pointer &child);

    /**
      * The evaluate method may be used to calculate the value of the
      * expression syntax tree.
      *
      * @param undefer
      *     True if deferred values should be evaluated.
      * @param dereference
      *     True if reference values should be replaced with the
      *     referenced value.
      */
    rpt_value::pointer evaluate(bool undefer, bool dereference) const;

    /**
      * The get_nchildren method is used to obtain the number of child
      * nodes associated with this node.
      */
    size_t get_nchildren() const { return nchild; }

    /**
      * The get_pos method is used to obtain the source file location
      * associated with this node.
      */
    rpt_position::pointer get_pos() const { return pos; }

    /**
      * The pos_from_lex method is used to set the exproession node's
      * source code position from the lexer's current position.
      */
    void pos_from_lex();

    /**
      * The pos_from method is used to set the exproession node's
      * source code position from the source code location of another
      * expression node.
      */
    void pos_from(const rpt_expr::pointer &other);

protected:
    /**
      * The evaluate method may be used to calculate the value of the
      * expression syntax tree.
      */
    virtual rpt_value::pointer evaluate() const = 0;

public:
    /**
      * The lvalue method may be used to determine whether or not this
      * expression node is an L-value (something which may appear on the
      * left hand side of an assigment, a variable).
      *
      * The default implementation, which is true for the vast majority
      * of expression nodes, returns false.
      */
    virtual bool lvalue() const;

    /**
      * The nth_child method may be used to obtain the specified child
      * node of this node.
      *
      * @param n
      *     The index of the child desired, zero based.
      * @returns
      *     pointer to child, or NULL if index out of range
      */
    rpt_expr::pointer nth_child(size_t n) const;

private:
    /**
      * The pos instance variable is used to remember the location of
      * this expression in the source code.
      */
    rpt_position::pointer pos;

    /**
      * The child instance variable is used to remember the base address
      * of a dynamically allocated array of pointer to child expression
      * nodes.
      */
    rpt_expr::pointer *child;

    /**
      * The nchild instance variable is used to remember the number of
      * elements used to date in the "child" array.
      */
    size_t nchild;

    /**
      * The nchild_max instance variable is used to remember the
      * allocated size of the "child" array.
      */
    size_t nchild_max;

    /**
      * The copy constructor.  Do not use.
      */
    rpt_expr(const rpt_expr &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_expr &operator=(const rpt_expr &);
};

#endif // AEGIS_AER_EXPR_H

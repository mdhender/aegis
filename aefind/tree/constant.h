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

#ifndef AEFIND_TREE_CONSTANT_H
#define AEFIND_TREE_CONSTANT_H

#include <aefind/tree.h>

struct rpt_value_ty; // existence

/**
  * The tree_constant class us ised to represent an expression tree node
  * with a constant value.
  */
class tree_constant:
    public tree
{
public:
    /**
      * The destructor.
      */
    virtual ~tree_constant();

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * class method indtead.
      *
      * @param value
      *     The value of this node.
      */
    tree_constant(rpt_value::pointer value);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instance of this class.
      *
      * @param value
      *     The value of this node.
      */
    static tree::pointer create(rpt_value::pointer value);

protected:
    // See base class for documentation.
    const char *name() const;

    // See base class for documentation.
    void print() const;

    // See base class for documentation.
    rpt_value::pointer evaluate(string_ty *, string_ty *, string_ty *,
        struct stat *) const;

    // See base class for documentation.
    bool useful() const;

    // See base class for documentation.
    bool constant() const;

    // See base class for documentation.
    tree::pointer optimize() const;

private:
    /**
      * The value instance variable is used to remember the value of
      * this expression tree node.
      */
    rpt_value::pointer value;

    /**
      * The default constructor.  Do not use.
      */
    tree_constant();

    /**
      * The copy constructor.  Do not use.
      */
    tree_constant(const tree_constant &);

    /**
      * The assignment operator.  Do not use.
      */
    tree_constant &operator=(const tree_constant &);
};

#endif // AEFIND_TREE_CONSTANT_H

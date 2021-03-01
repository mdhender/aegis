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

#ifndef AEFIND_TREE_THIS_H
#define AEFIND_TREE_THIS_H

#include <aefind/tree.h>

/**
  * The tree_this class is used to represent the {} family of expression
  * tree nodes.
  */
class tree_this:
    public tree
{
public:
    /**
      * The destructor.
      */
    virtual ~tree_this();

private:
    /**
      * The constructor.
      *
      * @param arg
      *     Set to -1 for {}, 0 for {-}, or 1 for {+}
      */
    tree_this(int arg);

public:
    /**
      * The create class method is used to create a new {} expression tree
      * node, allocated in dynamic memory.
      *
      * @param arg
      *     Set to -1 for {}, 0 for {-}, or 1 for {+}
      */
    static tree::pointer create(int arg = -1);

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
      * The resolved instance variable is used to remember was kind of
      * {} this is.  Set to -1 for {}, 0 for {-}, or 1 for {+}
      */
    int resolved;

    /**
      * The default constructor.  Do not use.
      */
    tree_this();

    /**
      * The copy constructor.  Do not use.
      */
    tree_this(const tree_this &);

    /**
      * The assignment operator.  Do not use.
      */
    tree_this &operator=(const tree_this &);
};

#endif // AEFIND_TREE_THIS_H

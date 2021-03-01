//
//	aegis - project change supervisor
//	Copyright (C) 1997, 2002, 2004-2008 Peter Miller
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

#ifndef AEFIND_TREE_MONADIC_H
#define AEFIND_TREE_MONADIC_H

#include <aefind/tree.h>

/**
  * The tree_monadic class is used to represent an abstract tree one with
  * a single argument.
  */
class tree_monadic:
    public tree
{
public:
    /**
      * The destructor.
      */
    virtual ~tree_monadic();

protected:
    /**
      * The constructor.
      *
      * @param arg
      *     The single argument to the function.
      */
    tree_monadic(const pointer &arg);

    // See base class for documentation.
    void print() const;

    // See base class for documentation.
    bool useful() const;

    // See base class for documentation.
    bool constant() const;

    tree::pointer get_arg() const { return arg; }

private:
    /**
      * The arg instance variable is used to remember the argument to
      * this single-argument function.
      */
    tree::pointer arg;

    /**
      * The default constructor.  Do not use.
      */
    tree_monadic();

    /**
      * The copy constructor.  Do not use.
      */
    tree_monadic(const tree_monadic &);

    /**
      * The assignment operator.  Do not use.
      */
    tree_monadic &operator=(const tree_monadic &);
};

#endif // AEFIND_TREE_MONADIC_H

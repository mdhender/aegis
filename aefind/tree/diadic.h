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

#ifndef AEFIND_TREE_DIADIC_H
#define AEFIND_TREE_DIADIC_H

#include <aefind/tree.h>

/**
  * The tree_diadic abstract class is used to represent a generic
  * expression tree node which takes two arguments.
  */
class tree_diadic:
    public tree
{
public:
    virtual ~tree_diadic();

protected:
    /**
      * The constructor.
      */
    tree_diadic(const tree::pointer &left, const tree::pointer &right);

    // See base class for documentation.
    void print() const;

    // See base class for documentation.
    bool useful() const;

    // See base class for documentation.
    bool constant() const;

    tree::pointer get_left() const { return left; }
    tree::pointer get_right() const { return right; }

private:
    tree::pointer left;
    tree::pointer right;

    /**
      * The default constructor.  Do not use.
      */
    tree_diadic();

    /**
      * The copy constructor.  Do not use.
      */
    tree_diadic(const tree_diadic &);

    /**
      * The assignment operator.  Do not use.
      */
    tree_diadic &operator=(const tree_diadic &);
};

#endif // AEFIND_TREE_DIADIC_H

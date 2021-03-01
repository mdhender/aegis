//
// aegis - project change supervisor
// Copyright (C) 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program. If not, see <http://www.gnu.org/licenses/>.
//

#ifndef AEFIND_FUNCTION_DELETE_H
#define AEFIND_FUNCTION_DELETE_H

#include <aefind/tree/monadic.h>

class tree_list; // forward

/**
  * The tree_delete class is used to represent an expression tree which
  * evaluates to true, and deletes (removes, unlinks) its argument.
  */
class tree_delete:
    public tree_monadic
{
public:
    /**
      * The destructor.
      */
    virtual ~tree_delete();

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * class method instead.
      */
    tree_delete(const tree::pointer &arg);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instance of this class.
      */
    static pointer create(const tree::pointer &arg);

    /**
      * The create class method is used to create new dynamically
      * allocated instance of this class.
      */
    static pointer create_l(const tree_list &arg);

protected:
    // See base class for documentation.
    const char *name(void) const;

    // See base class for documentation.
    rpt_value::pointer evaluate(string_ty *, string_ty *, string_ty *,
        struct stat *) const;

    // See base class for documentation.
    bool useful(void) const;

    // See base class for documentation.
    bool constant(void) const;

    // See base class for documentation.
    tree::pointer optimize(void) const;

private:
    /**
      * The default constructor.  Do not use.
      */
    tree_delete();

    /**
      * The copy constructor.  Do not use.
      */
    tree_delete(const tree_delete &);

    /**
      * The assignment operator.  Do not use.
      */
    tree_delete &operator=(const tree_delete &);
};

// vim: set ts=8 sw=4 et :
#endif // AEFIND_FUNCTION_DELETE_H

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

#ifndef AEFIND_FUNCTION_PRINT_H
#define AEFIND_FUNCTION_PRINT_H

#include <aefind/tree/monadic.h>

class tree_list; // forward

/**
  * The tree_print class is used to represent an expression tree which
  * evaluates to true, and prints its argument on the standard output as
  * a side effect.
  */
class tree_print:
    public tree_monadic
{
public:
    /**
      * The destructor.
      */
    virtual ~tree_print();

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * class method instead.
      */
    tree_print(const tree::pointer &arg);

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
    const char *name() const;

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
      * The default constructor.  Do not use.
      */
    tree_print();

    /**
      * The copy constructor.  Do not use.
      */
    tree_print(const tree_print &);

    /**
      * The assignment operator.  Do not use.
      */
    tree_print &operator=(const tree_print &);
};

#endif // AEFIND_FUNCTION_PRINT_H

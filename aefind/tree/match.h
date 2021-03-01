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

#ifndef AEFIND_TREE_MATCH_H
#define AEFIND_TREE_MATCH_H

#include <aefind/tree/diadic.h>

/**
  * The tree_match class is used to represent an expression tree node
  * for compareing a file globbing pattern with a string (usually a file
  * name).
  */
class tree_match:
    public tree_diadic
{
public:
    /**
      * The destructor.
      */
    virtual ~tree_match();

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * class method instead.
      */
    tree_match(const tree::pointer &lhs, const tree::pointer &rhs);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      */
    static pointer create(const tree::pointer &lhs, const tree::pointer &rhs);

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
    tree_match();

    /**
      * The copy constructor.  Do not use.
      */
    tree_match(const tree_match &);

    /**
      * The assignment operator.  Do not use.
      */
    tree_match &operator=(const tree_match &);
};


#endif // AEFIND_TREE_MATCH_H

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

#ifndef AEFIND_TREE_H
#define AEFIND_TREE_H

#include <common/ac/shared_ptr.h>
#include <libaegis/aer/value.h>

struct string_ty; // existence
struct stat; // existence

/**
  * The tree abstract base class is used to represent a generic node in
  * an expression syntax tree.
  */
class tree
{
public:
    typedef aegis_shared_ptr<tree> pointer;

    virtual ~tree();

protected:
    /**
      * The default constructor.
      */
    tree();

public:
    virtual const char *name() const = 0;
    virtual void print() const = 0;
    virtual rpt_value::pointer evaluate(string_ty *, string_ty *, string_ty *,
        struct stat *) const = 0;
    virtual bool useful() const = 0;
    virtual bool constant() const = 0;
    virtual tree::pointer optimize() const = 0;

    rpt_value::pointer evaluate_constant() const;
    tree::pointer optimize_constant() const;

private:
    /**
      * The copy constructor.  Do not use.
      */
    tree(const tree &);

    /**
      * The copy constructor.  Do not use.
      */
    tree &operator=(const tree &);
};


#endif // AEFIND_TREE_H

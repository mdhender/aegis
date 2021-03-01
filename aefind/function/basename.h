//
//      aegis - project change supervisor
//      Copyright (C) 1997, 2002, 2005-2008, 2012 Peter Miller
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

#ifndef AEFIND_FUNCTION_BASENAME_H
#define AEFIND_FUNCTION_BASENAME_H

#include <aefind/tree/monadic.h>

class tree_list; // forward

/**
  * The tree_basename class is used to rpresent an expression tree which
  * returns the basename of its argument.
  */
class tree_basename:
    public tree_monadic
{
public:
    /**
      * The destructor.
      */
    virtual ~tree_basename();

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * clas smethod instead.
      */
    tree_basename(const pointer &arg);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instance of this class.
      *
      * @param arg
      *     The singel argument to this function.
      */
    static pointer create(const pointer &arg);

    /**
      * The create_l class method is used to create new dynamically
      * allocated instance of this class.
      *
      * @param args
      *     The arguments to this function.
      */
    static pointer create_l(const tree_list &args);

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
    tree_basename();

    /**
      * The copy constructor.  Do not use.
      */
    tree_basename(const tree_basename &);

    /**
      * The assignment operator.  Do not use.
      */
    tree_basename &operator=(const tree_basename &);
};

#endif // AEFIND_FUNCTION_BASENAME_H
// vim: set ts=8 sw=4 et :

//
//      aegis - project change supervisor
//      Copyright (C) 2007, 2008, 2012 Peter Miller
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

#ifndef LIBAEGIS_SUB_FUNCTOR_GLUE_H
#define LIBAEGIS_SUB_FUNCTOR_GLUE_H

#include <libaegis/sub/functor.h>

/**
  * The sub_functor_glue class is used to represent calling a function
  * to do the work of the functor.
  *
  * DO NOT write new substitutions using this class.  New substitutions
  * should always be implemented by derving from the sub_functor
  * abstract base class.
  */
class sub_functor_glue:
    public sub_functor
{
public:
    /**
      * The destructor.
      */
    virtual ~sub_functor_glue();

    typedef wstring (*func_p)(sub_context_ty *scp, const wstring_list &arg);

private:
    /**
      * The constructor.  May not be called directly, you must always go
      * via the create class method.
      *
      * @param name
      *     The name of this substitution
      * @param what
      *     The function to perform the substitution
      * @param resub
      *     the output of the substitution is to be re-interpreted again
      *     for substitutions
      */
    sub_functor_glue(const nstring &name, func_p what, bool resub);

public:
    /**
      * The create class method is used to create a new instance of this
      * class.  This ensures that smart pointers are always used.
      *
      * @param name
      *     The name of this substitution
      * @param what
      *     The function to perform the substitution
      */
    static pointer create(const nstring &name, func_p what);

    /**
      * The create class method is used to create a new instance of this
      * class.  This ensures that smart pointers are always used.
      *
      * @param name
      *     The name of this substitution
      * @param what
      *     The function to perform the substitution
      * @param resub
      *     the output of the substitution is to be re-interpreted again
      *     for substitutions
      */
    static pointer create(const nstring &name, func_p what, bool resub);

protected:
    // See base class for documentation.
    wstring evaluate(sub_context_ty *cp, const wstring_list &arg);

    // See base class for documentation.
    bool resubstitute() const;

    // See base class for documentation.
    bool append_if_unused() const;

    // See base class for documentation.
    bool override() const;

    // See base class for documentation.
    bool must_be_used() const;

    // See base class for documentation.
    void resubstitute_set();

    // See base class for documentation.
    void optional_set();

    // See base class for documentation.
    void append_if_unused_set();

    // See base class for documentation.
    void override_set();

private:
    /**
      * The what instance variable is used to remember the function to
      * call to perform the substitution.
      */
    func_p what;

    /**
      * The resub instance variable is used to remeber whether or
      * not the output of this substritution is to be reexamined fro
      * substitutions.
      */
    bool resub;

    /**
      * The default constructor.  Do not use.
      */
    sub_functor_glue();

    /**
      * The copy constructor.  Do not use.
      */
    sub_functor_glue(const sub_functor_glue &);

    /**
      * The assignment operator.  Do not use.
      */
    sub_functor_glue &operator=(const sub_functor_glue &);
};

#endif // LIBAEGIS_SUB_FUNCTOR_GLUE_H
// vim: set ts=8 sw=4 et :

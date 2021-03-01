//
//	aegis - project change supervisor
//	Copyright (C) 2007, 2008 Peter Miller
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

#ifndef LIBAEGIS_SUB_FUNCTOR_H
#define LIBAEGIS_SUB_FUNCTOR_H

#include <common/ac/shared_ptr.h>

#include <common/nstring.h>
#include <common/wstring.h>

class sub_context_ty; // forward
class wstring_list; // forward

/**
  * The sub_functor class is used to represent an abstract substitution
  * to be applied by a substitution context.
  */
class sub_functor
{
public:
    /**
      * The pointer typedef makes it easier to write pointers to
      * instances of this class.  It also permits the pointer
      * implementation to change without having to edit every client of
      * this API.
      */
    typedef aegis_shared_ptr<sub_functor> pointer;

    /**
      * The destructor.
      */
    virtual ~sub_functor();

    /**
      * The name_get method is used to obtain the name of this functor.
      */
    nstring name_get() const { return name; }

    /**
      * The evaluate method is used to evaluate this functor in the
      * given context, with the given arguments.
      *
      * @param scp
      *     Substitution context
      * @param arg
      *     The list of arguments
      * @returns
      *     a wide string containng the text to be substituted
      */
    virtual wstring evaluate(sub_context_ty *cp, const wstring_list &arg) = 0;

    /**
      * This is why it's a functor: it can be called like a function.
      * The functioncall operator is used to evaluate this functor in
      * the given context, with the given arguments.
      *
      * @param scp
      *     Substitution context
      * @param arg
      *     The list of arguments
      * @returns
      *     a wide string containng the text to be substituted
      */
    wstring
    operator()(sub_context_ty *cp, const wstring_list &arg)
    {
        return evaluate(cp, arg);
    }

    /**
      * The append_if_unused method is used to determine hether or not
      * the value of this fucntor should be appended to the current
      * substitution if it is not used at all in the substitution.
      * Almost always false.
      */
    virtual bool append_if_unused() const = 0;

    /**
      * The override method is used to determine whether or not this
      * functor overrides an arlier functor of the same name.  Almost
      * always false.
      */
    virtual bool override() const = 0;

    /**
      * The override method is used to determine whether or not the
      * output of this functor should be rescanned for substitutions.
      * Almost always false (making it an error to be ambiguous).
      */
    virtual bool resubstitute() const = 0;

    /**
      * The must_be_used method is used to determine whether or not
      * it is an error if this functor is not used in the current
      * substitution.  Almost always true for non-builtins.
      */
    virtual bool must_be_used() const = 0;

    /**
      * The resubstitute_set method is used to request that the output
      * be resubstituted.  This is sticky until the context is reset.
      */
    virtual void resubstitute_set() = 0;

    /**
      * The override_set method is used to request that this functor be
      * considereed to override the value of an earlier functor of the
      * same name.  This is sticky until the context is reset.
      */
    virtual void override_set() = 0;

    /**
      * The optional_set method is used to request that the non-use of
      * this fucntor be a non-error.  This is sticky until the context
      * is reset.
      */
    virtual void optional_set() = 0;

    /**
      * The append_if_unused_set method is used to request that the
      * value of this fucntor be appended to the substitution if this
      * functor has not been used anywhere in the current substitution.
      * This is sticky until the context is reset.
      */
    virtual void append_if_unused_set() = 0;

protected:
    /**
      * The constructor.
      */
    sub_functor(const nstring &name);

private:
    /**
      * The name instance variable is used to remember the name of
      * this substitution.  The name also doubles as an arglex_compare
      * pattern.
      */
    nstring name;

    /**
      * The default constructor.  Do not use.
      */
    sub_functor();

    /**
      * The copy constructor.  Do not use.
      */
    sub_functor(const sub_functor &);

    /**
      * The assignment operator.  Do not use.
      */
    sub_functor &operator=(const sub_functor &);
};

#endif // LIBAEGIS_SUB_FUNCTOR_H

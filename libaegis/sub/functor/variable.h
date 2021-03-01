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

#ifndef LIBAEGIS_SUB_FUNCTOR_VARIABLE_H
#define LIBAEGIS_SUB_FUNCTOR_VARIABLE_H

#include <libaegis/sub/functor.h>

/**
  * The sub_functor_variable class is used to represent a substitution
  * which inserts a string.
  */
class sub_functor_variable:
    public sub_functor
{
public:
    /**
      * The destructor.
      */
    virtual ~sub_functor_variable();

private:
    /**
      * The constructor.
      * It is private on purpose: always use the create class method.
      *
      * @param name
      *     The name of the variable
      * @param value
      *     The value to assign to the variable.
      */
    sub_functor_variable(const nstring &name, const nstring &value);

public:
    /**
      * The create class method is used to allocate new instances of
      * this class.  It exists to ensure that only smart pointers are
      * used.
      *
      * @param name
      *     The name of the variable
      * @param value
      *     The value to assign to the variable.
      * @returns
      *     pointer to new instance
      */
    static pointer create(const nstring &name, const nstring &value);

protected:
    // See base class for documentation
    wstring evaluate(sub_context_ty *, const wstring_list &);

    // See base class for documentation
    bool append_if_unused() const;

    // See base class for documentation
    bool override() const;

    // See base class for documentation
    bool resubstitute() const;

    // See base class for documentation
    bool must_be_used() const;

    // See base class for documentation
    void resubstitute_set();

    // See base class for documentation
    void override_set();

    // See base class for documentation
    void optional_set();

    // See base class for documentation
    void append_if_unused_set();

private:
    /**
      * The value instance variable is used to remember the value of
      * this variable.
      */
    wstring value;

    /**
      * The used instance variable is used to remember whether or not
      * this variable has been used.
      */
    bool used;

    /**
      * The append_if_unused_flag instance variable is used to remember
      * whether or not the value of this variable should be appended to
      * the substitution if it has not yet been used.
      */
    bool append_if_unused_flag;

    /**
      * The override_flag instance variable is used to remember whether
      * or not this variable is deliberately overriding a builtin
      * substitution.
      */
    bool override_flag;

    /**
      * The resubstitute_flag instance variable is used to remember
      * whether or not the outrput of this functor shall be rescanned
      * for more substitutions.  Almost always false.
      */
    bool resubstitute_flag;

    /**
      * The default constructor.
      */
    sub_functor_variable();

    /**
      * The copy constructor.
      */
    sub_functor_variable(const sub_functor_variable &);

    /**
      * The assignment operator.
      */
    sub_functor_variable &operator=(const sub_functor_variable &);
};

#endif // LIBAEGIS_SUB_FUNCTOR_VARIABLE_H

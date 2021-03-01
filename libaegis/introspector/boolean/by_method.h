//
// aegis - project change supervisor
// Copyright (C) 2008 Peter Miller
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

#ifndef LIBAEGIS_INTROSPECTOR_BOOLEAN_BY_METHOD_H
#define LIBAEGIS_INTROSPECTOR_BOOLEAN_BY_METHOD_H

#include <libaegis/introspector/boolean.h>

/**
  * The introspector_boolean_by_method template class is used to
  * represent the manipulations required to manage bool instance
  * variables by the meta-data parser, using a template to allow
  * parametric spacialization using pointers to methods to manipulate
  * the bool within it's object.
  */
template <class T>
class introspector_boolean_by_method:
    public introspector_boolean
{
public:
    /**
      * The destructor.
      */
    virtual ~introspector_boolean_by_method(){}

private:
    /**
      * The constructor.
      * It is private on purpose, use the #create class method instead.
      *
      * @param a_object
      *     The object the bool is a member of.
      * @param a_set
      *     method used to set the bool member
      * @param a_is_set
      *     method used to determine whether or not the bool member has been set
      */
    introspector_boolean_by_method(T &a_object, void (T::*a_set)(bool),
            bool (T::*a_is_set)() const) :
        object(a_object),
        set(a_set),
        is_set(a_is_set)
    {
    }

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param a_object
      *     The object the bool is a member of.
      * @param a_set
      *     method used to set the bool member
      * @param a_is_set
      *     method used to determine whether or not the bool member has been set
      */
    static pointer
    create(T &a_object, void (T::*a_set)(bool), bool (T::*a_is_set)()const)
    {
        return
            pointer
            (
                new introspector_boolean_by_method(a_object, a_set, a_is_set)
            );
    }

protected:
    // See base class for documentation.
    void
    enumeration(const nstring &name)
    {
        if (name == "true")
            (object.*set)(true);
        else if (name == "false")
            (object.*set)(false);
        else
            value_of_type_required();
    }

private:
    /**
      * The object instance variable is used to remember the object the
      * field is a member of.
      */
    T &object;

    /**
      * The set instance variable i sused to remember the method used
      * to set the boolean_by_method member
      */
    void (T::*set)(bool);

    /**
      * The is_set instance variable is sued to remember the method used
      * to determine whether or not the member has been set.
      */
    bool (T::*is_set)() const;

    /**
      * The default constructor.  Do not use.
      */
    introspector_boolean_by_method();

    /**
      * The copy constructor.  Do not use.
      */
    introspector_boolean_by_method(const introspector_boolean_by_method &);

    /**
      * The assignment operator.  Do not use.
      */
    introspector_boolean_by_method &operator=(
        const introspector_boolean_by_method &);
};

#endif // LIBAEGIS_INTROSPECTOR_BOOLEAN_BY_METHOD_H

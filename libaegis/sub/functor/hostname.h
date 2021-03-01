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

#ifndef LIBAEGIS_SUB_FUNCTOR_HOSTNAME_H
#define LIBAEGIS_SUB_FUNCTOR_HOSTNAME_H

#include <libaegis/sub/functor.h>

/**
  * The sub_functor_hostname class is used to represent a substitution
  * that is replaced by the hostname of the executing host.
  */
class sub_functor_hostname:
    public sub_functor
{
public:
    /**
      * The destructor.
      */
    virtual ~sub_functor_hostname();

private:
    /**
      * The constructor.
      * It is private on purpose, use the #create class method instead.
      *
      * @param aname
      *     The name of the substitution.
      */
    sub_functor_hostname(const nstring &aname);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param aname
      *     The name of the substitution.
      */
    static pointer create(const nstring &aname);

protected:
    // See base class for documentation.
    wstring evaluate(sub_context_ty *cp, const wstring_list &arg);

private:
    /**
      * The default constructor.  Do not use.
      */
    sub_functor_hostname();

    /**
      * The copy constructor.  Do not use.
      */
    sub_functor_hostname(const sub_functor_hostname &);

    /**
      * The assignment operator.  Do not use.
      */
    sub_functor_hostname &operator=(const sub_functor_hostname &);
};

#endif // LIBAEGIS_SUB_FUNCTOR_HOSTNAME_H

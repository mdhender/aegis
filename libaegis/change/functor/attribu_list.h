//
//	aegis - project change supervisor
//	Copyright (C) 2004-2008 Peter Miller
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

#ifndef LIBAEGIS_CHANGE_FUNCTOR_ATTRIBU_LIST_H
#define LIBAEGIS_CHANGE_FUNCTOR_ATTRIBU_LIST_H

#include <libaegis/change/functor.h>

/**
  * The change_functor_attribute_list class is used to represent
  * a functor used to walk branch trees accumulating UUIDs as
  * original-UUID values.
  */
class change_functor_attribute_list:
    public change_functor
{
public:
    /**
      * The destructor.
      */
    virtual ~change_functor_attribute_list();

    /**
      * The constructor.
      *
      * \param incbr
      *     Include branches in the result.
      * \param arg
      *     The attribute list to hold the UUIDs.
      */
    change_functor_attribute_list(bool incbr, attributes_list_ty *arg);

    // See base class for documentation.
    void operator()(change::pointer cp);

private:
    /**
      * The remember method is used to add another UUID to the attribute
      * list.
      *
      * \param uuid
      *     The UUID to add to the list.
      */
    void remember(string_ty *uuid);

    /**
      * The ap instance variable is used to remember where the attribute
      * list is located, so that we can append to it.
      */
    attributes_list_ty *alp;

    /**
      * The default constructor.  Do not use.
      */
    change_functor_attribute_list();

    /**
      * The copy constructor.  Do not use.
      */
    change_functor_attribute_list(const change_functor_attribute_list &);

    /**
      * The assignment operator.  Do not use.
      */
    change_functor_attribute_list &operator=(
	const change_functor_attribute_list &);
};

#endif // LIBAEGIS_CHANGE_FUNCTOR_ATTRIBU_LIST_H

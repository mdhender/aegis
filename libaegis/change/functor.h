//
//	aegis - project change supervisor
//	Copyright (C) 2004 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: interface of the change_functor class
//

#ifndef LIBAEGIS_CHANGE_FUNCTOR_H
#define LIBAEGIS_CHANGE_FUNCTOR_H

#pragma interface "change_functor"

#include <change.h>

/**
  * The change_functor class is used to represent an abstract bas class
  * used to present object which <i>look like</i> callable functions
  * which take a singe change_ty * argument.
  */
class change_functor
{
public:
    /**
      * The destructor.
      */
    virtual ~change_functor();

    /**
      * The operator() method is used to call the functor.
      *
      * \param cp
      *     The change to be operated upon.
      */
    virtual void operator()(change_ty *cp) = 0;

protected:
    /**
      * The default constructor.
      * May ony be called by a derived class.
      */
    change_functor();

    /**
      * The copy constructor.
      * May ony be called by a derived class.
      */
    change_functor(const change_functor &);

    /**
      * The assignment operator.
      * May ony be called by a derived class.
      */
    change_functor &operator=(const change_functor &);
};

#endif // LIBAEGIS_CHANGE_FUNCTOR_H

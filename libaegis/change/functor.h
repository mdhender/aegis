//
//	aegis - project change supervisor
//	Copyright (C) 2004, 2005 Peter Miller;
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

    /**
      * The include_branches method is sued to determine whether or not
      * the functor should also be invoked for branches, when performing
      * a recursive descent.
      */
    bool include_branches() const { return include_branches_flag; }

    /**
      * The all_changes method is used to determine whether or not the
      * functor should be invoked for incomplete changes as well as
      * completed changes and branches.
      */
    bool all_changes() const { return all_changes_flag; }

protected:
    /**
      * The constructor.
      * May ony be called by a derived class.
      */
    change_functor(bool include_branches, bool all_changes = false);

    /**
      * The constructor.
      * May ony be called by a derived class.
      */
    change_functor(const change_functor &);

    /**
      * The assignment operator.
      * May ony be called by a derived class.
      */
    change_functor &operator=(const change_functor &);

private:
    bool include_branches_flag;
    bool all_changes_flag;

    /**
      * The default constructor.  Do not use.
      */
    change_functor();
};

#endif // LIBAEGIS_CHANGE_FUNCTOR_H

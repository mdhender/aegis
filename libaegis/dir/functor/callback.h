//
//	aegis - project change supervisor
//	Copyright (C) 2005 Peter Miller;
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
// MANIFEST: interface of the dir_functor_callback class
//

#ifndef LIBAEGIS_DIR_FUNCTOR_CALLBACK_H
#define LIBAEGIS_DIR_FUNCTOR_CALLBACK_H

#pragma interface "dir_functor_callback"

#include <dir.h>
#include <dir/functor.h>

/**
  * The dir_functor_callback class is used to represent a backwards
  * compatibility interface for how dir_walk used to work.
  */
class dir_functor_callback:
    public dir_functor
{
public:
    /**
      * The destructor.
      */
    virtual ~dir_functor_callback();

    /**
      * The constructor.
      *
      * @param callback
      *     the function to call then an event happens.
      * @param auxilliary
      *     the extra argument to the callback function.
      */
    dir_functor_callback(dir_walk_callback_ty callback, void *auxilliary);

    // See base class for documentation.
    void operator()(msg_t msg, const nstring &path, const struct stat &st);

private:
    /**
      * The callback instance variable is used to remember the function
      * to call then an event happens.
      */
    dir_walk_callback_ty callback;

    /**
      * The callback instance variable is used to remember the extra
      * argument to the callback function.
      */
    void *auxilliary;

    /**
      * The default constructor.  Do not use.
      */
    dir_functor_callback();

    /**
      * The copy constructor.  Do not use.
      */
    dir_functor_callback(const dir_functor_callback &);

    /**
      * The assignment operator.  Do not use.
      */
    dir_functor_callback &operator=(const dir_functor_callback &);
};

#endif // LIBAEGIS_DIR_FUNCTOR_CALLBACK_H

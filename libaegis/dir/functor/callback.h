//
//	aegis - project change supervisor
//	Copyright (C) 2005, 2006, 2008 Peter Miller
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

#ifndef LIBAEGIS_DIR_FUNCTOR_CALLBACK_H
#define LIBAEGIS_DIR_FUNCTOR_CALLBACK_H

#include <libaegis/dir.h>
#include <libaegis/dir/functor.h>

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

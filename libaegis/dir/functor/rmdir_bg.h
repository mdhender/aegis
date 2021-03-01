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
// MANIFEST: interface of the dir_functor_rmdir_bg class
//

#ifndef LIBAEGIS_DIR_FUNCTOR_RMDIR_BG_H
#define LIBAEGIS_DIR_FUNCTOR_RMDIR_BG_H

#include <common/ac/unistd.h>

#include <libaegis/dir/functor.h>

/**
  * The dir_functor_rmdir_bg class is used to represent a background
  * recursive directory remover.
  */
class dir_functor_rmdir_bg:
    public dir_functor
{
public:
    /**
      * The destructor.
      */
    virtual ~dir_functor_rmdir_bg();

    /**
      * The default constructor.
      */
    dir_functor_rmdir_bg();

    // See base class for documentation.
    void operator()(msg_t msg, const nstring &path, const struct stat &st);

private:
    /**
      * The copy constructor.  Do not use.
      */
    dir_functor_rmdir_bg(const dir_functor_rmdir_bg &);

    /**
      * The assignment operator.  Do not use.
      */
    dir_functor_rmdir_bg &operator=(const dir_functor_rmdir_bg &);
};

#endif // LIBAEGIS_DIR_FUNCTOR_RMDIR_BG_H

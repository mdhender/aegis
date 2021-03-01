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
// MANIFEST: interface of the dir_functor class
//

#ifndef LIBAEGIS_DIR_FUNCTOR_H
#define LIBAEGIS_DIR_FUNCTOR_H

#pragma interface "dir_functor"

class nstring; // forward

/**
  * The dir_functor class is used to represent an abstract interface for
  * dir_walk to invoke.
  */
class dir_functor
{
public:
    /**
      * The destructor.
      */
    virtual ~dir_functor();

    /**
      * The default constructor.
      */
    dir_functor();

    /**
      * The copy constructor.
      */
    dir_functor(const dir_functor &);

    /**
      * The assignment operator.
      */
    dir_functor &operator=(const dir_functor &);

    enum msg_t
    {
	msg_dir_before,
	msg_dir_after,
	msg_file,
	msg_special,
	msg_symlink
    };

    /**
      * The () operator is used to implement the function of the
      * functor.
      *
      * @param msg
      *     The msg argument is used to say what s to be done.
      * @param path
      *     The path to the file is question.
      * @param st
      *     The file information about the \a path.
      */
    virtual void operator()(msg_t msg, const nstring &path,
	const struct stat &st) = 0;
};

#endif // LIBAEGIS_DIR_FUNCTOR_H

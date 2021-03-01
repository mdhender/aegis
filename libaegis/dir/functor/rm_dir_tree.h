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

#ifndef LIBAEGIS_DIR_FUNCTOR_RM_DIR_TREE_H
#define LIBAEGIS_DIR_FUNCTOR_RM_DIR_TREE_H

#include <libaegis/dir/functor.h>

/**
  * The dir_functor_rm_dir_tree class is used to represent an object
  * which is able to remove directoryt trees when called by the dir_walk
  * function.
  *
  * This class's () operator is the function that actually deletes
  * things out of the directory tree as it is walked.  Note that the
  * directory should be deleted last, after all contents have been
  * nuked.
  *
  * Some sleight-of-hand is involved here, as this function pushes extra
  * stuff onto the lists of things to be deleted, rather than really
  * doing it itself.
  */
class dir_functor_rm_dir_tree:
    public dir_functor
{
public:
    /**
      * The destructor.
      */
    virtual ~dir_functor_rm_dir_tree();

    /**
      * The default constructor.
      */
    dir_functor_rm_dir_tree();

    // See base class for documentation.
    void operator()(msg_t msg, const nstring &path, const struct stat &st);

private:
    /**
      * The copy constructor.  Do not use.
      */
    dir_functor_rm_dir_tree(const dir_functor_rm_dir_tree &);

    /**
      * The assignment operator.  Do not use.
      */
    dir_functor_rm_dir_tree &operator=(const dir_functor_rm_dir_tree &);
};

#endif // LIBAEGIS_DIR_FUNCTOR_RM_DIR_TREE_H

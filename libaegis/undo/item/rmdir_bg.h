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
// MANIFEST: interface of the undo_item_rmdir_bg class
//

#ifndef LIBAEGIS_UNDO_ITEM_RMDIR_BG_H
#define LIBAEGIS_UNDO_ITEM_RMDIR_BG_H

#pragma interface "undo_item_rmdir_bg"

#include <nstring.h>
#include <undo/item.h>

/**
  * The undo_item_rmdir_bg class is used to represent an (rm -r &)
  * action to be performed in the event of an error.  Errors are
  * ignored.
  */
class undo_item_rmdir_bg:
    public undo_item
{
public:
    /**
      * The destructor.
      */
    virtual ~undo_item_rmdir_bg();

    /**
      * The constructor.
      */
    undo_item_rmdir_bg(const nstring &path);

    // See base class for documentation.
    void action();

    // See base class for documentation.
    void unfinished();

private:
    nstring path;

    /**
      * The default constructor.  Do not use.
      */
    undo_item_rmdir_bg();

    /**
      * The copy constructor.  Do not use.
      */
    undo_item_rmdir_bg(const undo_item_rmdir_bg &);

    /**
      * The assignment operator.  Do not use.
      */
    undo_item_rmdir_bg &operator=(const undo_item_rmdir_bg &);
};

#endif // LIBAEGIS_UNDO_ITEM_RMDIR_BG_H

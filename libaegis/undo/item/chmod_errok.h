//
//	aegis - project change supervisor
//	Copyright (C) 2004-2006, 2008 Peter Miller
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

#ifndef LIBAEGIS_UNDO_ITEM_CHMOD_ERROK_H
#define LIBAEGIS_UNDO_ITEM_CHMOD_ERROK_H

#include <common/nstring.h>
#include <libaegis/undo/item.h>

/**
  * The undo_item_chmod_errok class is used to represent a chmod(2) to
  * be performed in the event of an error.  Errors are ignored.
  */
class undo_item_chmod_errok:
    public undo_item
{
public:
    /**
      * The destructor.
      */
    virtual ~undo_item_chmod_errok();

    /**
      * The constructor.
      */
    undo_item_chmod_errok(const nstring &path, int mode);

    // See base class for documentation.
    void action();

    // See base class for documentation.
    void unfinished();

private:
    nstring path;
    int mode;

    /**
      * The default constructor.  Do not use.
      */
    undo_item_chmod_errok();

    /**
      * The copy constructor.  Do not use.
      */
    undo_item_chmod_errok(const undo_item_chmod_errok &);

    /**
      * The assignment operator.  Do not use.
      */
    undo_item_chmod_errok &operator=(const undo_item_chmod_errok &);
};

#endif // LIBAEGIS_UNDO_ITEM_CHMOD_ERROK_H

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

#ifndef LIBAEGIS_UNDO_ITEM_MESSAGE_H
#define LIBAEGIS_UNDO_ITEM_MESSAGE_H

#include <common/nstring.h>
#include <libaegis/undo/item.h>

/**
  * The undo_item_message class is used to represent an error message to
  * be printed in the event of an error.
  */
class undo_item_message:
    public undo_item
{
public:
    /**
      * The destructor.
      */
    virtual ~undo_item_message();

    /**
      * The constructor.
      */
    undo_item_message(const nstring &msg);

    // See base class for documentation.
    void action();

    // See base class for documentation.
    void unfinished();

private:
    nstring message;

    /**
      * The default constructor.  Do not use.
      */
    undo_item_message();

    /**
      * The copy constructor.  Do not use.
      */
    undo_item_message(const undo_item_message &);

    /**
      * The assignment operator.  Do not use.
      */
    undo_item_message &operator=(const undo_item_message &);
};

#endif // LIBAEGIS_UNDO_ITEM_MESSAGE_H

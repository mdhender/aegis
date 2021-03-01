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

#ifndef LIBAEGIS_UNDO_ITEM_H
#define LIBAEGIS_UNDO_ITEM_H

/**
  * The undo_item class is used to represent an abstract undo action to
  * be perfomed in the event of command failure.
  */
class undo_item
{
public:
    /**
      * The destructor.
      */
    virtual ~undo_item();

    /**
      * The default constructor.
      */
    undo_item();

    /**
      * The act method is used to perform the undo action; it first sets
      * the appropriate user, and then call sthe action() method.
      */
    void act();

    /**
      * The unfinished method is used to print a warning about
      * unfinished undo action.  This happens when there is an undo
      * error.  The default action says nothing.
      */
    virtual void unfinished() = 0;

protected:
    /**
      * The action method is used to perform the undo action.
      */
    virtual void action() = 0;

private:
    /**
      * The uid instance variable is used to remember the user ID to be
      * used when performing the action.
      */
    int uid;

    /**
      * The gid instance variable is used to remember the group ID to be
      * used when performing the action.
      */
    int gid;

    /**
      * The umask instance variable is used to remember the user
      * permissions mask to be used when performing the action.
      */
    int umask;

    /**
      * The copy constructor.  Do not use.
      */
    undo_item(const undo_item &);

    /**
      * The assignment operator.  Do not use.
      */
    undo_item &operator=(const undo_item &);
};

#endif // LIBAEGIS_UNDO_ITEM_H

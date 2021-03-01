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
// MANIFEST: interface of the quit_action_undo class
//

#ifndef LIBAEGIS_QUIT_ACTION_UNDO_H
#define LIBAEGIS_QUIT_ACTION_UNDO_H

#include <common/quit/action.h>

/**
  * The quit_action_undo class is used to represent the set of actions
  * to be performed on a rollback on operation failure.
  */
class quit_action_undo:
    public quit_action
{
public:
    /**
      * The destructor.
      */
    virtual ~quit_action_undo();

    /**
      * The default constructor.
      */
    quit_action_undo();

    // See base class for documenattion.
    void operator()(int exit_status);

private:
    /**
      * The copy constructor.  Do not use.
      */
    quit_action_undo(const quit_action_undo &);

    /**
      * The assignment operator.  Do not use.
      */
    quit_action_undo &operator=(const quit_action_undo &);
};

#endif // LIBAEGIS_QUIT_ACTION_UNDO_H

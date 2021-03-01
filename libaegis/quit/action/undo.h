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

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

#ifndef LIBAEGIS_QUIT_ACTION_PAGER_H
#define LIBAEGIS_QUIT_ACTION_PAGER_H

#include <common/quit/action.h>

/**
  * The quit_action_pager class is used to represent actions to be
  * performed on program exit when an output pager is being used.
  */
class quit_action_pager:
    public quit_action
{
public:
    /**
      * The destructor.
      */
    virtual ~quit_action_pager();

    /**
      * The default constructor.
      */
    quit_action_pager();

    // See base class for documentation.
    void operator()(int exit_status);

private:
    /**
      * The copy constructor.  Do not use.
      */
    quit_action_pager(const quit_action_pager &);

    /**
      * The assignment operator.  Do not use.
      */
    quit_action_pager &operator=(const quit_action_pager &);
};

#endif // LIBAEGIS_QUIT_ACTION_PAGER_H

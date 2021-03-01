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
// MANIFEST: interface of the quit_action_pager class
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

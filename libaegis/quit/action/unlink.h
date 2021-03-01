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

#ifndef LIBAEGIS_QUIT_ACTION_UNLINK_H
#define LIBAEGIS_QUIT_ACTION_UNLINK_H

#include <common/nstring.h>
#include <common/quit/action.h>

/**
  * The quit_action_unlink class is used to represent an unlink action
  * to be formed in quit.
  */
class quit_action_unlink:
    public quit_action
{
public:
    /**
      * The destructor.
      */
    virtual ~quit_action_unlink();

    /**
      * The constructor.
      */
    quit_action_unlink(const nstring &arg);

    // See base class for documentation.
    void operator()(int exit_status);

private:
    /**
      * The path instance variable is used to remember the path of the
      * file to be unlinked on exit.
      */
    const nstring path;

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
      * The default constructor.  Do not use.
      */
    quit_action_unlink();

    /**
      * The copy constructor.  Do not use.
      */
    quit_action_unlink(const quit_action_unlink &);

    /**
      * The assignment operator.  Do not use.
      */
    quit_action_unlink &operator=(const quit_action_unlink &);
};

#endif // LIBAEGIS_QUIT_ACTION_UNLINK_H

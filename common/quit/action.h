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
// MANIFEST: interface of the quit_action class
//

#ifndef COMMON_QUIT_ACTION_H
#define COMMON_QUIT_ACTION_H

/**
  * The quit_action class is used to represent an abstract action to be
  * performed at quit() time.
  */
class quit_action
{
public:
    /**
      * The destructor.
      */
    virtual ~quit_action();

    /**
      * The default constructor.
      */
    quit_action();

    /**
      * The copy constructor.
      */
    quit_action(const quit_action &);

    /**
      * The assignment operator.
      */
    quit_action &operator=(const quit_action &);

    /**
      * The operator() method is called just before exit(exit_status)
      * is called.  It affords and opportunity to "clean up" before the
      * program exits.
      *
      * \param exit_status
      *     The imminent exit status of the program.
      */
    virtual void operator()(int exit_status) = 0;
};

#endif // COMMON_QUIT_ACTION_H

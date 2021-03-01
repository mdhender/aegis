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

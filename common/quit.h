//
// aegis - project change supervisor
// Copyright (C) 2004-2006, 2008, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#ifndef COMMON_QUIT_H
#define COMMON_QUIT_H

#include <common/gcc-attribute.h>
#include <common/quit/action.h>

/**
  * The quit_register function is used to register a quit_action to be
  * called when the quit() function is called.  Actions are called in
  * the reverse of the order they were registered.
  *
  * \param action
  *     The action to be performed when quit is called.
  */
void quit_register(quit_action &action);

/**
  * The quit_unregister function is used to cancel an previous quit
  * registration.  The action is <i>not</i> performed when quit()
  * is called.  It is a bug if quit_register has not been called
  * previously.
  */
void quit_unregister(quit_action &action);

/**
  * The quit function shall be used to exit the program, both for
  * success and for failure.  (Note: we don't use atexit because Aegis
  * was written long before atexit() was commonly available.)  (Note: we
  * don't use atexit because Aegis was written long before atexit() was
  * commonly available.)
  */
void quit(int) NORETURN;

#endif // COMMON_QUIT_H
// vim: set ts=8 sw=4 et :

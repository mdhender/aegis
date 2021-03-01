//
//	aegis - project change supervisor
//	Copyright (C) 2004 Peter Miller;
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
// MANIFEST: interface of the quit class
//

#ifndef COMMON_QUIT_H
#define COMMON_QUIT_H

#include <main.h>
#include <quit/action.h>

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

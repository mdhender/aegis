//
//	aegis - project change supervisor
//	Copyright (C) 2006 Peter Miller
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
// MANIFEST: interface of the stack class
//

#ifndef COMMON_STACK_H
#define COMMON_STACK_H

/**
  * The variable_is_on_stack function is used to determine whether a
  * variable is on the stack.  This is usually used for debugging.
  *
  * There are three broard categories of variables we have to worry
  * about: stack, static and heap.
  *
  * It is possible this function will return true for static variables,
  * depending on your compiler and linker.
  */
bool variable_is_on_stack(void *p);

#endif // COMMON_STACK_H

//
// aegis - project change supervisor
// Copyright (C) 1997, 2002, 2003, 2005, 2006, 2008, 2012 Peter Miller
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

#ifndef AEFIND_CMDLINE_H
#define AEFIND_CMDLINE_H

void cmdline_grammar(int, char **);

struct string_ty *stack_relative(struct string_ty *);
struct string_ty *stack_nth(int);

/**
  * The stack_eliminate function takes a stack-relative file name,
  * and returns 1 if that file name should be ignored, or 0 if it may
  * be processed.
  */
int stack_eliminate(struct string_ty *);

#endif // AEFIND_CMDLINE_H
// vim: set ts=8 sw=4 et :

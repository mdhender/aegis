//
//      aegis - project change supervisor
//      Copyright (C) 2003, 2005, 2006, 2008, 2011, 2012 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program.  If not, see
//      <http://www.gnu.org/licenses/>,
//

#ifndef LIBAEGIS_EMIT_PROJECT_H
#define LIBAEGIS_EMIT_PROJECT_H


struct project; // forward

/**
  * The emit_project function is used to emit the project name cross
  * linked to all of its parent branches.  This is used in web page
  * headings.
  */
void emit_project(struct project *);

/**
  * The emit_project function is used to emit the project name cross
  * linked to all of its parent branches, except the last element.
  * This is used in web page headings.
  */
void emit_project_but1(struct project *);

#endif // LIBAEGIS_EMIT_PROJECT_H
// vim: set ts=8 sw=4 et :

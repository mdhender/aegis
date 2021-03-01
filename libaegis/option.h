//
// aegis - project change supervisor
// Copyright (C) 1991-1993, 1995, 1997, 1999, 2002-2006, 2008, 2012 Peter Miller
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

#ifndef OPTION_H
#define OPTION_H


void option_progname_set(char *);
char *option_progname_get(void);

void option_verbose_set(void(*)(void));
bool option_verbose_get(void);
void option_unformatted_set(void(*)(void));
int option_unformatted_get(void);
void option_terse_set(void(*)(void));
int option_terse_get(void);

void option_page_width_set(int columns, void (*usage)(void));
void option_page_length_set(int rows, void (*usage)(void));

void option_tab_width_set(int, void(*)(void));
int option_tab_width_get(void);

void option_page_headers_set(int, void(*)(void));
int option_page_headers_get(void);

void option_signed_off_by_argument(void(*)(void));
bool option_signed_off_by_get(bool dflt);

#endif // OPTION_H
// vim: set ts=8 sw=4 et :

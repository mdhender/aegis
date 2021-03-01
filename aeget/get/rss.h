//
//      aegis - project change supervisor
//      Copyright (C) 2005 Matthew Lee;
//      All rights reserved.
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 2 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: interface of the get_rss class
//

#ifndef AEGET_GET_RSS_H
#define AEGET_GET_RSS_H

#include <main.h>

struct project_ty;
struct string_ty;
struct string_list_ty;

/**
 * Get an RSS feed file.
 */
void get_rss(struct project_ty *pp,
             struct string_ty *,
             struct string_list_ty *modifier);

#endif // AEGET_GET_RSS_H

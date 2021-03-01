//
// aegis - project change supervisor
// Copyright (C) 1997, 2002, 2005-2008, 2012 Peter Miller
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

#ifndef AEFIND_SHORTHAND_STAT_H
#define AEFIND_SHORTHAND_STAT_H

class nstring; // forward

typedef tree::pointer (*diadic_t)(const tree::pointer &, const tree::pointer &);

tree::pointer shorthand_atime(diadic_t, int, int);
tree::pointer shorthand_ctime(diadic_t, int, int);
tree::pointer shorthand_gid(diadic_t, int);
tree::pointer shorthand_ino(diadic_t, int);
tree::pointer shorthand_mode(int);
tree::pointer shorthand_mtime(diadic_t, int, int);
tree::pointer shorthand_newer(const nstring &filename);
tree::pointer shorthand_nlink(diadic_t, int);
tree::pointer shorthand_size(diadic_t, int);
tree::pointer shorthand_uid(diadic_t, int);
tree::pointer shorthand_type(const nstring &abbrev);

#endif // AEFIND_SHORTHAND_STAT_H
// vim: set ts=8 sw=4 et :

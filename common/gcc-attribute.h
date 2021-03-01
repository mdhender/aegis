//
// aegis - project change supervisor
// Copyright (C) 2012 Peter Miller
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
// You should have received a copy of the GNU General Public License along
// with this program. If not, see <http://www.gnu.org/licenses/>.
//

#ifndef COMMON_GCC_ATTRIBUTE_H
#define COMMON_GCC_ATTRIBUTE_H

//
// Function attributes.
//
#ifdef __GNUC__
#define ATTR_PRINTF(x, y) __attribute__((format(printf, x, y)))
#define ATTR_VPRINTF(x) __attribute__((format(printf, x, 0)))
#define NORETURN __attribute__((noreturn))
#else
#define ATTR_PRINTF(x, y)
#define ATTR_VPRINTF(x)
#define NORETURN
#endif

#if (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1))
#define DEPRECATED __attribute__((deprecated))
#else
#define DEPRECATED
#endif

// vim: set ts=8 sw=4 et :
#endif // COMMON_GCC_ATTRIBUTE_H

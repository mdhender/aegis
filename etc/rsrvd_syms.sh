#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2004, 2006-2008 Peter Miller
#
#	This program is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; either version 3 of the License, or
#	(at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program. If not, see
#	<http://www.gnu.org/licenses/>.
#
LANG=C
export LANG

LIBC=/usr/lib/libc.a
LIBM=/usr/lib/libm.a
 tmp=${TMP_DIR-/tmp}/$$
 obj="$*"
 nm $LIBC $LIBM 2>/dev/null |
    egrep ' [A-TXYZ] ' | awk '{print $3}' | sort -u > ${tmp}.a
 nm $obj 2>/dev/null |
    egrep ' [A-TXYZ] ' | awk '{print $3}' | sort -u > ${tmp}.b
 common=`comm -12 ${tmp}.a ${tmp}.b`
 rm ${tmp}.a ${tmp}.b
 if test -n "$common"
then
     echo "The following reserved symbols have been used:" $common
     for sym in $common
     do
      nm -A $obj 2>/dev/null | egrep " [A-TV-Z] $sym"
     done
     exit 1
 fi
exit 0

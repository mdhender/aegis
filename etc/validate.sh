#!/bin/sh
#
# aegis - project change supervisor
# Copyright (C) 2012 Peter Miller
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or (at
# your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program. If not, see <http://www.gnu.org/licenses/>.
#
tmp=/tmp/validate-$$.txt
validate --emacs $1 > $tmp 2>&1
status=$?
awk '/^.*:.*:.*:E: / {
    n = split($0, a, / *: */)
    oline = a[1]
    for (j = 2; j <= n; ++j)
        oline = oline ": " a[j]
    print oline
}' $tmp
rm $tmp
exit $status
# vim: set ts=8 sw=4 et :

#!/bin/sh
#
# aegis - project change supervisor
# Copyright (C) 1997, 2002, 2006-2008 Peter Miller
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
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#
#
# This script is placed in a user's per-user crontab and run weelky or
# monthly to remind them of changes which they are developing (or
# integrating) that they have forgotten about.
#
case $# in
0)
    ;;
*)
    echo "Usage: $0" 1>&2
    exit 1
    ;;
esac

aegis=aegis

addr=${USER-${LOGIN-`whoami`}}
changes=`$aegis -l uc -v`
if [ "$change" ]
then
    sendmail -t << fubar
To: $addr
Subject: Outstanding Changes

The following changes are assigned to you.  Please complete them at
your earliest possible convenience, so that changes may pass through
the system at the fastest possible rate.

$changes
fubar
fi
exit 0

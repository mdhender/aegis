#! /bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1993, 2006-2008 Peter Miller.
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
orig=$1
path=$2
name=$3
tmp=/tmp/$$
trap "rm $tmp; exit 1" 1 2 3 15
echo diff -c $orig $path 1>&2
diff -c $orig $path > $tmp
case $? in
0)
	;;
1)
	echo "Index: $name"
	sed '1,2d' < $tmp
	;;
*)
	rm $tmp
	exit 1
	;;
esac
rm $tmp
exit 0
